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

#define MAX_FILENAME_SIZE 256 //макс размер файла 

int compare_ints(const void *a, const void *b) //компоратор
{
  return (*((int *)b) - *((int *)a));
}

long fsize(FILE *fp)// определяем размер файла 
{
  long prev = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  long sz = ftell(fp);
  fseek(fp, prev, SEEK_SET);
  return sz;
}

int main(int argc, char *argv[]) 
{
  int sockfd; // переменная для сокета
  struct sockaddr_in servaddr; // переменная для сервака

  bzero(&servaddr, sizeof(servaddr)); // обнуление 
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[1]));
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // настройки сервака

  sockfd = socket(PF_INET, SOCK_STREAM, 0); //создание соккета 
  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) // присваиваем имя
  {
    servaddr.sin_port = 0;
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
      perror(NULL);
      close(sockfd);
      exit(1);
    }
  }
  socklen_t servlen = sizeof(servaddr); //размер переменной сервака
  listen(sockfd, 5); // задаем колличества клиентов 
  getsockname(sockfd, (struct sockaddr *)&servaddr, &servlen); // получаем имя соккета 
  printf("Listening on port: %d\n", ntohs(servaddr.sin_port));

  if (fork() == 0) // создаем новый процесс(обрабатываем запросы клиента)
  {
    while (1)
    {
      struct sockaddr_in cliaddr;
      socklen_t clilen = sizeof(cliaddr);
      int newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen); //ожидаем нового клиента 
      if (fork() == 0) // создаем новый процесс
        continue;
      while (1) // обрабатываем запросы нового пользователя
      {
        char filename[MAX_FILENAME_SIZE]; // создаем массив имени файла 
        int n = read(newsockfd, filename, MAX_FILENAME_SIZE);// читаем из сокета имя файла 
        if (n == 0)
        {
          close(newsockfd);
          exit(0);
        }
        FILE *fin = fopen(filename, "r"); // открываем файл 
        if (fin == NULL) // если не получилось 
        {
          long statusmsg = -1;
          write(newsockfd, &statusmsg, sizeof(statusmsg));// отправляем -1
        }
        else
        {
          long filesize = fsize(fin); // определяем размер файла
          write(newsockfd, &filesize, sizeof(filesize)); // отсылаем клиенту размер файла 

          char msg[filesize];
          fread(msg, sizeof(char), filesize, fin);// читаем файл 
          fclose(fin);
          write(newsockfd, msg, sizeof(msg));// отправляем содержимое файла 
        }
      }
    }
  }
  else // обрабатываем команды сервера 
  {
    printf("Ready to recive commands\n");
    char command[MAX_FILENAME_SIZE];
    while (1) // цикл для обработки запроса сервака
    {
      scanf("%s", command);

      if (strcmp(command, "exit") == 0) 
      {
        exit(0);
      }
      else if (strcmp(command, "help") == 0)
      {
        printf("Avalible commands:\n");
        printf("exit - closes app\n");
        printf("help - shows avalible commands\n");
      }
      else
      {
        printf("Unknown command, please use help to get list of avalible commands\n");
      }
    }
  }
}