#include "stdio.h"
#include "sys/shm.h"
#include "stdlib.h"

void print_array_int(int* arr)	//функция для вывода массива
{
   for (int i = 0; i < 20; i++)
      printf("%i ", *(arr + i));	//*(arr +i) тоже самое что и arr[i]
   printf("\n");
}

int compare_int_value(const void* a, const void* b) //функция сравнения двух чисел
{													//Указатели на void, можно сравнивать резные типы
   return *((int*) a) - *((int*) b);	//конвертирование указателя в int* и взятие значения по нему.
}

int main(int argv, char* argc[])	//argv - количество аргументов, которые передаются в исп. файл.
{									//argc[] - массив на argv ячеек. В каждой ячейки строка (char*)
   if (argv <= 1)	//Если аргуменнтов не более 1, то сделать вот это:
   {
      printf("not enough params\n");
      return -1;
   }

   char* paramStr = argc[1];	//берём первый параметр
   int memId = atoi(paramStr); 	//ковертируем его в число

   if (memId == 0)	//если не конвертировалось, то программма выкидывает ошибку
   {
      printf("incorrect parameter string: %s\n", paramStr);
      return -2;
   }

   printf("receiving the memory data: shmid = %i\n", memId);

   int* mem = (int*) shmat(memId, 0, 0);	//получаем адресс разделяемой памяти по идентификатору memId
   if (NULL == mem)	//если указатель равен NULL, то программма выкидывает ошибку
   {
      printf("error with shmat()\n");
      return -3;
   }

   printf("receiving next array:\n");
   print_array_int(mem);	//выводим массив по указателю

   qsort(mem, 20, 4, compare_int_value);	//сортируем массив

   printf("sorted array:\n");
   print_array_int(mem);	//снова выводим

   return 0;
}