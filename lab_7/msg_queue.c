#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct thread_args	//структура, котороя нужна для передачи аргументов
{
  size_t msgId;	//в качестве параметров хранит переменную size_t, в которую будет записан индификатор очереди сообщений
};

struct my_msg	//данная структура нужна для помещения данных в очередь сообщений
{
  long mtype;
  char mtext[4];
};

void print_nums(char *nums, int c)	//функция, которая выводит строку с числами на экран { чесно, не понимаю зачем её делать отдельно}
{
  for (int i = 0; i < c; i++)
  {
    printf("%d ", nums[i]);
  }
  printf("\n");
}

void *mainTask(void *thread_args)
{
  struct thread_args *args = (struct thread_args *)thread_args;	//получаем структуру thread_args
  size_t msgId = args->msgId;	//получем из неё id очереди
  struct my_msg send;
  struct my_msg recive;

  char nums[4];
  srand((unsigned)(time(0)));	//генерируем случайные числа
  for (int i = 0; i < 4; i++)
  {
    nums[i] = rand() % 10;
  }
  print_nums(nums, 4);	//выводим на экран
  for (int i = 0; i < 4; i++)	//записываем их в структуру send
  {
    send.mtext[i] = nums[i];
  }
  send.mtype = 1;
  msgsnd(msgId, &send, sizeof(send), 0);	//помещаем её в очередь

  msgrcv(msgId, &recive, sizeof(recive), 2, 0);	//получаем данные из очереди
  int p_count = recive.mtext[0];	//сначала childTask отправляет p_count, количество получившихся перестановок
  printf("Permutations:\n");
  for (int i = 0; i < p_count; i++)	//через цикл достаёт числа из очереди, и выводит их
  {
    msgrcv(msgId, &recive, sizeof(recive), 2, 0);
    print_nums(recive.mtext, 4);
  }

  printf("Permutations count:%d\n", p_count);

  msgctl(msgId, IPC_RMID, NULL);	//удаляем очередь

  return NULL;
}

void *childTask(void *thread_args)
{
  struct thread_args *args = (struct thread_args *)thread_args;	//то же самое, что и в mainTask
  size_t msgId = args->msgId;
  struct my_msg send;
  struct my_msg recive;

  msgrcv(msgId, &recive, sizeof(recive), 1, 0); //получаем данные из очреди

  char p[24][4];
  int p_count = 0;
  for (int i = 0; i < 4; i++)	//в данном тройном цикле пперебераются все возможные варианты комбинаций цифр
  {								
    for (int j = 0; j < 4; j++)	//	
    {
      if (j == i)
        continue;
      for (int k = 0; k < 4; k++)
      {
        if (k == i || k == j)
          continue;
        int l = 6 - i - j - k;
        char curr_p[4];
        curr_p[0] = recive.mtext[i];
        curr_p[1] = recive.mtext[j];
        curr_p[2] = recive.mtext[k];
        curr_p[3] = recive.mtext[l];
        char unique = 1;
        for (int m = 0; m < p_count; m++)
        {
          if (memcmp(p[m], curr_p, 4) == 0)
          {
            unique = 0;
            break;
          }
        }
        if (unique)
        {
          memcpy(p[p_count], curr_p, 4);
          p_count++;
        }
      }
    }
  }
  send.mtext[0] = p_count;	//помещаем переменную p_count - количество перестановок
  send.mtype = 2;
  msgsnd(msgId, &send, sizeof(send), 0);	//и отправляем её
  for (int i = 0; i < p_count; i++)	//после этого через цикл отправляем все получившиеся перестановки
  {
    memcpy(send.mtext, p[i], 4);
    send.mtype = 2;
    msgsnd(msgId, &send, sizeof(send), 0);
  }

  return NULL;
}

int main()
{
  int msgId = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);	//создание очереди сообщений

  struct thread_args *args;	//создаём структуру thread_args
  args->msgId = msgId;	//записываем в её поле индификатор очереди

  pthread_t mainThread, childThread;
																//создаём потоки для mainTask и childTask
  pthread_create(&mainThread, NULL, mainTask, (void *)args);
  pthread_create(&childThread, NULL, childTask, (void *)args);

  pthread_join(mainThread, NULL);
}