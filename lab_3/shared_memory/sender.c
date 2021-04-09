#include "stdio.h"
#include "sys/shm.h"
#include "stdlib.h"
#include "sys/ipc.h"
#include "time.h"

int main(void)
{
   const size_t memSize = 80;	//резмер сегмента разделяемой памяти
   
   int memId = shmget(IPC_PRIVATE, memSize, 0600 | IPC_CREAT | IPC_EXCL);	//Получение доступа к сегменту разделяемой памяти
																			//Размера memSize и с параметрами 0600 | IPC_CREAT | IPC_EXCL
   printf("shmid = %i\n", memId);
   if (memId <= 0)	//если индификатор memId <= 0, то вывести ошибку
   {
      printf("error with shmid()\n");
      return -1;
   }

   int* mem = (int* )shmat(memId, 0, 0);	//Получаем адрес разделяемой памяти.
   if (NULL == mem)	//Если равен NULL, то вывести ошибку
   {
      printf("error with shmat()\n");
      return -2;
   }

   srand(time(NULL));	//Заполнить массив случайными числами.
   for (int i = 0; i < 20; i++)
      *(mem + i) = rand() % 100;	//*(mem + i) тоже самое что и mem[i]
									
   char callbuf[1024];							//Создаем буфер
   sprintf(callbuf, "./receiver %i", memId);	//Записываем в него команду ./receiver [наш memId]
   system(callbuf);	//вызываем эту комаду

   return 0;
}