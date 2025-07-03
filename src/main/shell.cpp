#include "shell.hpp"
#include "../error/error.hpp"
#include <cstring>

using namespace std;

void shell(FileSystemManager *fsm)
{
  std::string input, operation, argument;

  while (1)
  {
    try
    {
      std::cout << std::endl<< "[ " << fsm->pwd() << " ]>  ";
      std::getline(std::cin, input); //Lê uma linha completa digitada pelo usuário e armazena em input.
      std::cout << std::endl;

      // Extrai a primeira palavra da entrada (antes do primeiro espaço)
      operation = input.substr(0, input.find(" "));

      //Se houver espaço na entrada, assume que há um argumento
      bool has_argument = (input.find(" ") != std::string::npos);
      int pos = input.find(" ") + 1; // Calcula a posição de início do argumento.
      argument = input.substr(pos, input.length() - pos); // Extrai o argumento completo após o comando.

      if (!std::strcmp(operation.c_str(), "info")){
        fsm->info();
      }

      else if (!std::strcmp(operation.c_str(), "cd")){
        if(argument.size() == 0 || !has_argument)  throw new Error("invalid syntax.");
        fsm->cd(argument.c_str());
      }

      else if (!std::strcmp(operation.c_str(), "cat")){
        if(argument.size() == 0 || !has_argument)  throw new Error("invalid syntax.");
        fsm->cat(argument.c_str());
      }

      else if (!std::strcmp(operation.c_str(), "ls")) {
        fsm->ls();
      }

      else if (!std::strcmp(operation.c_str(), "pwd")){
        string pwd(fsm->pwd());

        if(pwd.size() > 1)  pwd.pop_back();
        cout << pwd << endl;
      }

      else if (!std::strcmp(operation.c_str(), "attr"))
      {
        if(argument.size() == 0 || !has_argument)  throw new Error("invalid syntax.");
        fsm->attr(argument.c_str());
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

        if (!std::strcmp(first_argument.c_str(), "inode"))
        {
          int inode = std::stoi(second_argument);
          fsm->inode_info(inode);
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