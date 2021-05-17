#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#define UDP_BUFFER_LEN 65507


void fill_random_nums(int *nums, int n, int min, int max)//заполняем массив ранд. числами
{
  srand((unsigned)(time(0)));
  for (int i = 0; i < n; i++)
  {
    nums[i] = min + rand() % (max - min + 1);
  }
}

void print_nums(int *nums, int c)// выводим числа 
{
  for (int i = 0; i < c; i++)
  {
    printf("%d ", nums[i]);
  }
  printf("\n");
}

long timedifference(struct timeval t0, struct timeval t1) //находим разницу во времени
{
  return (t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec);
}

int main(int argc, char *argv[])
{
  size_t max_n = UDP_BUFFER_LEN / sizeof(int);
  int n = 0, min, max;
  while (n <= 0 || n > max_n)
  {
    printf("Enter array length(<=%ld): ", max_n); //записываем размер массива 
    scanf("%d", &n);
  }
  printf("Enter minimum: ");
  scanf("%d", &min);// вводим границы массива
  printf("Enter maximum: ");
  scanf("%d", &max);

  int sockfd; // создаем переменную для соккета 
  char sendline[n * sizeof(int) / sizeof(char)], recvline[n * sizeof(int) / sizeof(char)];// буфер
  struct sockaddr_in servaddr, cliaddr;

  sockfd = socket(PF_INET, SOCK_DGRAM, 0);// создаем соккет

  bzero(&cliaddr, sizeof(cliaddr));//обнуляем переменные 
  cliaddr.sin_family = AF_INET;
  cliaddr.sin_port = htons(0);//задаем настройки 
  cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));//присваиваем имя соккету

  char *sep = strchr(argv[1], ':');// получаем порт 
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(sep + 1));//настройки для сервера
  sep[0] = 0;
  inet_aton(argv[1], &servaddr.sin_addr);// приводим ip в нужный формат 
  fill_random_nums((int *)sendline, n, min, max);// вызываем функцию рандома
  print_nums((int *)sendline, n);//выводим

  struct timeval start, end;// вводим переменные для подсчета времени
  int s = sendto(sockfd, sendline, sizeof(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));// отправляем массив
  gettimeofday(&start, 0);
  recvfrom(sockfd, recvline, sizeof(recvline), 0, (struct sockaddr *)NULL, NULL);// получаем массив 
  gettimeofday(&end, 0);// получаем время 
  print_nums((int *)recvline, n);//вывод сортированного массива
  printf("Response time: %ldns\n", timedifference(start, end));// вывод разницы во времени 
  close(sockfd);
}