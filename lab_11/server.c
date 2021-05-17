#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

#ifdef __linux__
#define UDP_BUFFER_LEN 65507
#endif

int compare_ints(const void *a, const void *b) //компоратор для функции qsort
{
  return (*((int *)b) - *((int *)a));
}

int main(int argc, char *argv[])
{
  size_t maxlen = UDP_BUFFER_LEN;
  int sockfd; // переменная для соккета 
  char line[maxlen]; // буфер для сообщений 
  struct sockaddr_in servaddr, cliaddr; // переменные для определения сервер и клиента 

  bzero(&servaddr, sizeof(servaddr)); // онулирование переменной 
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[1])); //задаем порт 
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // настройка для любого адреса 

  sockfd = socket(PF_INET, SOCK_DGRAM, 0); //создаем соккет
  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) //даем имя соккету
  {
    servaddr.sin_port = 0;
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
      perror(NULL);
      close(sockfd); 
      exit(1);
    }
  }
  socklen_t servlen = sizeof(servaddr); // определяем размер переменной servaddr
  getsockname(sockfd, (struct sockaddr *)&servaddr, &servlen);//получаем имя 
  printf("Listening on port: %d\n", ntohs(servaddr.sin_port));// выводим порт сервера

  while (1)
  {
    socklen_t clilen = sizeof(cliaddr);
    int n = recvfrom(sockfd, line, maxlen, 0, (struct sockaddr *)&cliaddr, &clilen);//получаем сообщение из соккета и записываем в буфер line
    qsort(line, n * sizeof(char) / sizeof(int), sizeof(int), compare_ints);//сортируем массив
    sendto(sockfd, line, n, 0, (struct sockaddr *)&cliaddr, clilen);// отправляем массив отсортированный по сокету 
  }
}