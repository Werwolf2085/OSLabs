#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>

void sem(int semId, int n, int d)
{
    struct sembuf op;
    op.sem_op = d;
    op.sem_flg = 0;
    op.sem_num = n;
    semop(semId, &op, 1);
}

void unlockSem(int semId, char *check, int n)
{
    sem(semId, n, 1);
    check[n] = 0;
}

int lockSem(int semId, char *check, int n)
{
    int isBusy = check[n];
    sem(semId, n, -1);
    check[n] = 1;
    return isBusy;
}

void sort(int semId, int memId, char checkMemOffset, const size_t n)
{
    int *nums = (int *)shmat(memId, 0, 0);
    char *checks = shmat(memId, 0, 0) + checkMemOffset;

    for (int i = 0; i < n; i++)
    {
        int minInd = i;
        for (int j = i + 1; j < n; j++)
        {
            lockSem(semId, checks, i);
            lockSem(semId, checks, j);
            if (nums[j] < nums[minInd])
            {
                minInd = j;
            }
            unlockSem(semId, checks, i);
            unlockSem(semId, checks, j);
        }
        if (i != minInd)
        {
            lockSem(semId, checks, i);
            lockSem(semId, checks, minInd);
            int t = nums[i];
            nums[i] = nums[minInd];
            nums[minInd] = t;
            unlockSem(semId, checks, i);
            unlockSem(semId, checks, minInd);
        }
    }
}

void fill_random_nums(int *nums, int n, int min, int max)
{			//функция, заполняющая массив размера N случ. числами от min до max
    srand((unsigned)(time(0)));
    for (int i = 0; i < n; i++)
    {
        nums[i] = min + rand() % (max - min + 1);
    }
}

void print_nums(int *nums, int n)
{	//функция, которая выводит массив
    for (int i = 0; i < n; i++)
    {
        printf("%d ", nums[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    const int N = atoi(argv[1]);	//получем данные из аргументов при вызове исполняемого файла
    const int min = atoi(argv[2]);
    const int max = atoi(argv[3]);

    int memId = shmget(IPC_PRIVATE, sizeof(int) * N + N, 0600 | IPC_CREAT | IPC_EXCL);	//создаём сегмент разделяемой памяти
    int checkMemOffset = sizeof(int) * N;	//переменная, которая хранит размер 
    int semId = semget(IPC_PRIVATE, N, 0600 | IPC_CREAT);	//создаём семафор

    int *numbers = (int *)shmat(memId, 0, 0);	//получаем адресс разделяемой памяти
    char *checks = shmat(memId, 0, 0) + checkMemOffset;	//получем адресс памяти и прибавляем к нему sizeof(int) * N, для того, что бы получить адресс последних N байт.
    fill_random_nums(numbers, N, min, max);	//заполняем массив случ. числами
    print_nums(numbers, N);	//ывводим

    for (int i = 0; i < N; i++)		//НЕЗНАЮ
    {
        unlockSem(semId, checks, i);
    }

    int childId = fork();	//создаем процесс - ребёнок, которые копируем родителя по регист. и пользов. контексту
    if (childId == 0)	//код, который выполниться для ребёнка
    {
        sort(semId, memId, checkMemOffset, N);	//сортировка
    }
    else	//код, котрый выполниться для родителя
    {
        int i = 0;
        int status;
        do
        {
            printf("%d: ", i);
            for (int j = 0; j < N; j++)
            {
                if (lockSem(semId, checks, j))	//НЕЗНАЮ
                {
                    printf("[%d] ", numbers[j]);
                }
                else
                {
                    printf("%d ", numbers[j]);
                }

                fflush(stdout);
                unlockSem(semId, checks, j);
            }
            printf("\r\n");
            status = waitpid(childId, NULL, WNOHANG);	//родитель ждёт завершения ребнка
            i++;
        } while (!status);

        printf("Sort finished \r\n");	//выводим отсортированный массив
        print_nums(numbers, N);

        shmctl(memId, 0, IPC_RMID);	//удаляем сегменты разделяемой памяти
        semctl(semId, 0, IPC_RMID);
    }
}