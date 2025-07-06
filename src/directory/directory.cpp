#include "directory.hpp"
#include <sys/stat.h>
#include <cstring>
#include "../utils/utils.hpp"
#define BLOCK_SIZE 1024
#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + ((block) - 1) * BLOCK_SIZE)

typedef char Block[BLOCK_SIZE];

vector<Directory> read_directories(FILE* image, Inode* inode) {
    vector<Directory> directories;

    if (S_ISDIR(inode->i_mode)) {
        for (int i = 0; i < 12; i++) {
            if (!inode->i_block[i])
                break;

            uint8_t block[BLOCK_SIZE];
            fseek(image, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
            fread(block, 1, BLOCK_SIZE, image);

            uint32_t offset = 0;
            while (offset < BLOCK_SIZE) {
                Directory* entry = (Directory*)(block + offset);
                if (entry->inode == 0 || entry->rec_len == 0)
                    break; // Fim das entradas válidas

                if (entry->inode == 0) {
                    offset += entry->rec_len;
                    continue; // Pula entradas removidas, mas continua lendo o bloco
                }
                Directory directory;
                memcpy(&directory, entry, sizeof(Directory));
                // Garante que o nome será terminado por \0
                if (directory.name_len < EXT2_NAME_LEN)
                    directory.name[directory.name_len] = '\0';
                else
                    directory.name[EXT2_NAME_LEN - 1] = '\0';

                directories.push_back(directory);
                

                offset += entry->rec_len;
                if (entry->rec_len == 0)
                    break; // Segurança extra
            }
        }
    }

    return directories;
}

// Procura uma entrada de diretório pelo nome e retorna um ponteiro para uma cópia local (válida enquanto o vetor existir)
Directory* search_directory(FILE* image, Inode* inode, const char* name) {
    static Directory found; // static para garantir validade após retorno (não é thread-safe)
    vector<Directory> directories = read_directories(image, inode);

    size_t name_len = strlen(name);
    for (auto& dir : directories) {
        // Compare exatamente name_len e dir.name_len
        if (dir.name_len == name_len && strncmp(name, dir.name, name_len) == 0) {
            found = dir;
            return &found;
        }
    }
    return nullptr;
}

// Imprime informações de uma entrada de diretório
void print_directory(Directory directory){
    cout <<  directory.name << endl;
    cout << "inode:\t\t\t" << (unsigned) directory.inode << endl;
    cout << "tamanho do registro:\t" << (unsigned) directory.rec_len << endl;
    cout << "tamanho do nome:\t" << (unsigned) directory.name_len << endl;
    cout << "tipo do arquivo:\t" << (unsigned) directory.file_type << endl;
}

// Imprime todas as entradas de um vetor de diretórios
void print_directories(vector<Directory> directories) {
    for(vector<Directory>::iterator it = directories.begin(); it != directories.end(); it++){
        print_directory(*it);
        cout << endl;
    }
}