#include <iostream>
#include "file-operations.hpp"

using namespace std;

FILE *get_file(const char *ptr)
{
  FILE *file = fopen(ptr, "rb+");
  if (file == NULL) cout << "\nImage " << ptr << " is not valid"  << endl;
  else  cout << "\nImage " << ptr << " read" << endl;
  return file;
}