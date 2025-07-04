#include "shell.hpp"
#include "../utils/utils.hpp"
#include "../error/error.hpp"
#include <limits>

int main()
{
  FILE *ext2_image = NULL;
  FileSystemManager *fsm;

  while(1){
    try{
      
        char *input = (char *)malloc(sizeof(char) * 100);
        cout << "enter the iso image:\t";
        cin >> input;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        ext2_image = get_file((const char *)input);
        if(ext2_image){
          fsm = new FileSystemManager(ext2_image);
          shell(fsm);
        }
      }
      catch (const char *str)
      {
        cout << str << endl;
      }
      catch (Error *error)
      {
        cout << endl
             << "[ error::" << error->message << " ]"<< endl;
      }
  }

  return 0;
}