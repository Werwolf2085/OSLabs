#include <stdlib.h>

#include <stdio.h>

#include <unistd.h>

int main(int argc, char *argv[])

{

const char *fileName = argv[1]; // имя файла из аргументов командной

int byteC = atoi(argv[2]); // число байт из командной (atoi переводит

FILE *fin = fopen(fileName, "r"); // открввпем файл и получаем дескриптор

char data[byteC];

int res = fread(data, sizeof(char), byteC, fin); // читаем из файла нужное число байт, заносится в массив data

printf("%d\n", res);

for (int i = 0; i < res; i++)

{

printf("%c[%d]\n", data[i], data[i]); пишем из data

}

printf("\n");

fclose(fin);

}