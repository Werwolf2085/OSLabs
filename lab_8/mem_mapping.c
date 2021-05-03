#include <stdio.h>

#include <sys/mman.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <fcntl.h>

#include <unistd.h>

int main(int argc, char *argv[])

{

const char *input_file_name = argv[1]; // имя входного файла из аргументов командной строки

const char *output_file_name = argv[2]; // имя выходного из командной

int input_file = open(input_file_name, O_RDONLY, 0600); // открываем входной

int output_file = open(output_file_name, O_RDWR | O_CREAT, 0600); // открываем выходной

struct stat st; // структура стандатная для файла

stat(input_file_name, &st);

int file_size = st.st_size;

ftruncate(output_file, file_size);

char *output_file_data = (char *)mmap(NULL, file_size, PROT_WRITE | PROT_READ, MAP_SHARED, output_file, 0); // мапаем входной файл (заносим в оперптивку я так понял)

read(input_file, output_file_data, file_size); заносим данные из входного в выходной (по адресу, в оперативку)

munmap(output_file_data, file_size); // анмапаем выходной

close(input_file);

close(output_file);

}