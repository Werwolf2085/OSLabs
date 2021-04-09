#include <stdio.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


//Задаём количество элементов в "расшаренном" массиве константой
const int AMOUNT = 20;

//Функция для сравнения значения двух переменных типа int
//Применяется в вызове библиотечного метода сортировки
int compare_ints(const void *a, const void *b)
{
    return (*((int *)a) - *((int *)b));
}

//Функция сортировки и вывода массива
void sortAndPrint(int *mem)
{
    //Вызов библиотечной функции быстрой сортировки
    qsort(mem, AMOUNT, sizeof(int), compare_ints);
    //Вывод элементов массива
    for (size_t i = 0; i < AMOUNT; i++)
    {
        printf("%i ", mem[i]);
    }
    printf("\n");
}

int main()
{
    //Инициируем генератор случайных чисел
    srand((unsigned)(time(0)));
    //Создаём область "расшареной" памяти, указывавем её размер, права доступа и получаем её дескриптор
    int memId = shmget(IPC_PRIVATE, sizeof(int) * AMOUNT, 0600 | IPC_CREAT | IPC_EXCL);
    //Пристыковаваем выделенный участок памяти как массив
    int *numbers = (int *)shmat(memId, 0, 0);
    //Заполняем этот массив случайными числами
    for (size_t i = 0; i < AMOUNT; i++)
    {
        numbers[i] = rand() % 10000;
    }
    //Выводим этот массив
    for (size_t i = 0; i < AMOUNT; i++)
    {
        printf("%d ", numbers[i]);
    }

    printf("\n");
    //Создаём дочерний процесс
    int child_id = fork();

    if (child_id == 0)
    {	
	//Сортируем массив в дочернем массиве
        sortAndPrint(numbers);
    }
    else
    {
	//Ждём завершения работы дочернего процесса в процессе-родителе
        waitpid(child_id, NULL, 0);
    }
    //Освобождаем выделенный участок памяти
    shmdt(numbers);

    return 0;
}