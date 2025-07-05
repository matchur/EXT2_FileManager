#include "utils.hpp"
#include <ctime>
#include <iostream>

using std::cout;
using std::endl;

void print_time(unsigned int timestamp) { // imprime no formato DD/MM/AAAA HH:MM 
    time_t time = timestamp;
    tm* timeinfo = localtime(&time);
    char buffer[20];
    strftime(buffer, 20, "%d/%m/%Y %H:%M", timeinfo);
    cout << buffer << endl;
}

int get_block_offset(uint32_t block, int base_offset, int block_size){
    return base_offset + (block - 1) * block_size;
}

FILE *get_file(const char *ptr)
{
  FILE *file = fopen(ptr, "rb+");
  if (file == NULL) cout << "\nImagem " << ptr << " não é válida"  << endl;
  else  cout << "\nImagem " << ptr << " lida" << endl;
  return file;
}