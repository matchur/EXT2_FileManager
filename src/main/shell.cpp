#include "shell.hpp"
#include "../error/error.hpp"
#include <cstring>

using namespace std;

void shell(FileSystemManager *fsm)
{
  std::string input, operation, argument;
  std::getline(std::cin, input);

  while (1)
  {
    try
    {
      
      std::getline(std::cin, input);
      std::cout << std::endl;

      operation = input.substr(0, input.find(" "));

      bool has_argument = (input.find(" ") != std::string::npos);
      int pos = input.find(" ") + 1;
      argument = input.substr(pos, input.length() - pos);

      if (!std::strcmp(operation.c_str(), "info")){
        fsm->info();
      }

      else if (!std::strcmp(operation.c_str(), "print"))
      {
        std::string first_argument = argument.substr(0, argument.find(" "));
        int prox = argument.find(" ") + 1;
        std::string second_argument = argument.substr(prox, argument.length() - prox);

        if (!std::strcmp(first_argument.c_str(), "superblock"))
        {
          fsm->superblock_info();
        }

        else if (!std::strcmp(first_argument.c_str(), "groups"))
        {
          for (int i = 0; i < 8; i++)
          {
            cout << "Block Group Descriptor " << i << ":" << endl;
            fsm->blocks_group_descriptor_info(i);
            cout << endl;
          }
        }

        else if (!std::strcmp(first_argument.c_str(), "group"))
        {
          int bgd_index = std::stoi(second_argument);
          cout << "Block Group Descriptor " << bgd_index << ":" << endl;
          fsm->blocks_group_descriptor_info(bgd_index);
        }

        else
          throw new Error("invalid syntax.");
      }

      
      else if (!std::strcmp(operation.c_str(), "exit"))
        exit(0);

      else
        throw new Error("command not found");
    }
    catch (Error *error)
    {
      std::cout << error->message << endl;
    }
  }
}