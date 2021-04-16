#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

const char *FIFO_NAME = "novsu.fifo";

int compare_ints(const void *a, const void *b)	//эта функция нужна для вызова функции Qsort 
{
    return (*((int *)b) - *((int *)a));
}

int *get_random_nums(int n)	//функция заполнения случ. числами
{
    srand((unsigned)(time(0)));	//установка зерна генератора случ. чисел
    int *nums = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++)	//гзаполнение случайными числами.
    {
        nums[i] = rand() % 100000;
    }
    return nums;
}

void print_nums(int *nums, int c)	//выводит (int)с чисел по указателю nums 
{
    for (int i = 0; i < c; i++)
    {
        printf("%d ", nums[i]);
    }
    printf("\n");
}

int main(int argv, char *argc[])
{
    int n = atoi(argc[1]);
    int *nums = get_random_nums(n);
    print_nums(nums, n);

    mknod(FIFO_NAME, S_IFIFO | 0666, 0);	//создаём файл с названием FIFO_NAME 
    int p[2];
    pipe(p);	//создаём канал межпроцессорного обмена

    int child_id = fork();

    if (child_id == 0)
    {	//код, который выполнится для процесса - ребёнка
        close(p[0]);	//закрываем чтение для родителя.
						//процесс - ребёнок считывает файл
        int fifo = open(FIFO_NAME, O_RDONLY);	//открытие файла
        int *received_nums = malloc(sizeof(int) * n);	//создаём массив
        read(fifo, received_nums, sizeof(int) * n);	//читаем в него данные из файла
        close(fifo);	//зактрываем файл
        qsort(received_nums, n, sizeof(int), compare_ints);	//сортируем
        write(p[1], received_nums, sizeof(int) * n);
        close(p[1]);	//записываем и закрываем чтение для ребёнка
    }
    else
    {	//код, который выполниться для процессса - родителя
        close(p[1]);
        int fifo = open(FIFO_NAME, O_WRONLY);	//открываем файл
        write(fifo, nums, sizeof(int) * n); //записываем в него массив nums
        close(fifo);	//закрываем
        int *sorted_nums = malloc(sizeof(int) * n);	//создаём новый массив
        read(p[0], sorted_nums, sizeof(int) * n);	//читаем данные из канала
        print_nums(sorted_nums, n); //выводим их
        close(p[0]);
        unlink(FIFO_NAME);	//удаляем файл с названием FIFO_NAME
		
		
    }
}