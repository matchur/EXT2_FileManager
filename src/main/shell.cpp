#include "shell.hpp"
#include "../error/error.hpp"
#include <cstring>

using std::cout;
using std::endl;

void shell(FileSystemManager *fsm)
{
  string input, operacao, argumento;

  while (1)
  {
    try
    {
      // Mostra o diretório atual no prompt
      string pwd = fsm->pwd();
      if (pwd.size() > 1 && pwd.back() == '/')
        pwd.pop_back();
      cout << "[ " << pwd << " ]> ";

      getline(cin, input);

      cout << endl;

      // Extrai a primeira palavra da entrada (antes do primeiro espaço)
      operacao = input.substr(0, input.find(" "));

      // Se houver espaço na entrada, assume que há um argumento
      bool tem_argumento = (input.find(" ") != string::npos);
      int pos = input.find(" ") + 1; // Calcula a posição de início do argumento.
      argumento = input.substr(pos, input.length() - pos); // Extrai o argumento completo após o comando.

      if (!strcmp(operacao.c_str(), "info")){
        fsm->info();
      }

      else if (!strcmp(operacao.c_str(), "cd")){
        if(argumento.size() == 0 || !tem_argumento)  throw new Error("sintaxe inválida.");
        fsm->cd(argumento.c_str());
      }

      else if (!strcmp(operacao.c_str(), "cat")){
        if(argumento.size() == 0 || !tem_argumento)  throw new Error("sintaxe inválida.");
        fsm->cat(argumento.c_str());
      }

      else if (!strcmp(operacao.c_str(), "ls")) {
        fsm->ls();
      }
      else if (!strcmp(operacao.c_str(), "pwd")) {
        cout << fsm->pwd() << endl;
      }

      else if (!strcmp(operacao.c_str(), "attr"))
      {
        if(argumento.size() == 0 || !tem_argumento)  throw new Error("sintaxe inválida.");
        fsm->attr(argumento.c_str());
      }

      else if (!strcmp(operacao.c_str(), "print"))
      {
        string primeiro_argumento = argumento.substr(0, argumento.find(" "));
        int prox = argumento.find(" ") + 1;
        string segundo_argumento = argumento.substr(prox, argumento.length() - prox);

        if (!strcmp(primeiro_argumento.c_str(), "superblock"))
        {
          fsm->superblock_info();
        }

        else if (!strcmp(primeiro_argumento.c_str(), "groups"))
        {
          for (int i = 0; i < 8; i++)
          {
            cout << "Descritor do Grupo de Blocos " << i << ":" << endl;
            fsm->blocks_group_descriptor_info(i);
            cout << endl;
          }
        }

        else if (!strcmp(primeiro_argumento.c_str(), "group"))
        {
          int bgd_index = stoi(segundo_argumento);
          cout << "Descritor do Grupo de Blocos " << bgd_index << ":" << endl;
          fsm->blocks_group_descriptor_info(bgd_index);
        }

        if (!strcmp(primeiro_argumento.c_str(), "inode"))
        {
          int inode = stoi(segundo_argumento);
          fsm->inode_info(inode);
        }

        else
          throw new Error("sintaxe inválida.");
      }

      else if (!strcmp(operacao.c_str(), "cp")) {
        string primeiro_argumento = argumento.substr(0, argumento.find(" "));
        int prox = argumento.find(" ") + 1;
        string segundo_argumento = argumento.substr(prox, argumento.length() - prox);
        if(primeiro_argumento.empty() || segundo_argumento.empty())
            throw new Error("sintaxe inválida.");
        fsm->cp(primeiro_argumento.c_str(), segundo_argumento.c_str());
      }
      
      else if (!strcmp(operacao.c_str(), "touch")) {
        if(argumento.size() == 0 || !tem_argumento) throw new Error("sintaxe inválida.");
        fsm->touch(argumento.c_str());
      }
      else if (!strcmp(operacao.c_str(), "rm")) {
        if(argumento.size() == 0 || !tem_argumento) throw new Error("sintaxe inválida.");
        fsm->rm(argumento.c_str());
      }
      else if (!strcmp(operacao.c_str(), "rename")) {
        string primeiro_argumento = argumento.substr(0, argumento.find(" "));
        int prox = argumento.find(" ") + 1;
        string segundo_argumento = argumento.substr(prox, argumento.length() - prox);
        if(primeiro_argumento.empty() || segundo_argumento.empty())
            throw new Error("sintaxe inválida.");
        fsm->rename(primeiro_argumento.c_str(), segundo_argumento.c_str());
      }
      else if (!strcmp(operacao.c_str(), "mkdir")) {
        if(argumento.size() == 0 || !tem_argumento) throw new Error("sintaxe inválida.");
        fsm->mkdir(argumento.c_str());
      }
      else if (!strcmp(operacao.c_str(), "rmdir")) {
        if(argumento.size() == 0 || !tem_argumento) throw new Error("sintaxe inválida.");
        fsm->rmdir(argumento.c_str());
      }
      else if (!strcmp(operacao.c_str(), "exit"))
        exit(0);

      else
        throw new Error("comando não encontrado");
    }
    catch (Error *erro)
    {
      cout << erro->message << endl;
      delete erro;
    }
  }
}