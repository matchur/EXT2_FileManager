#include "directory.hpp"
#include <sys/stat.h>
#include <cstring>
#define BLOCK_SIZE 1024
#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block - 1) * BLOCK_SIZE)

typedef char Block[BLOCK_SIZE];

// Lê todas as entradas de diretório de um inode de diretório e retorna um vetor de Directory
vector<Directory> read_directories(FILE* image, Inode* inode) {
    vector<Directory> directories;

    if(S_ISDIR(inode->i_mode)) {
        for(int i = 0; i < 12; i++){
            Block* block = (Block*)malloc(sizeof(Block));
            if(!inode->i_block[i]) {
                free(block);
                break;
            }

            fseek(image, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
            fread(block, 1, sizeof(Block), image);

            Directory* worked_directory = (Directory *) block;
            int block_position = 0;
            do{
                Directory directory;
                memcpy(&directory, worked_directory, sizeof(Directory));
                directory.name[directory.name_len] = '\0';
                directories.push_back(directory);
                block_position += worked_directory->rec_len;
                worked_directory = (Directory *) ((char*) worked_directory + worked_directory->rec_len);
            }while((block_position < inode->i_size) && worked_directory->inode);

            free(block);
        }
    }

    return directories;
}

// Procura uma entrada de diretório pelo nome e retorna um ponteiro para uma cópia local (válida enquanto o vetor existir)
Directory* search_directory(FILE* image, Inode* inode, const char* name){
    Directory* directory = NULL;
    // O vetor directories é local, então o ponteiro retornado só é válido dentro deste escopo!
    static Directory found; // static para garantir validade após retorno (não é thread-safe)
    vector<Directory> directories = read_directories(image, inode);

    for(vector<Directory>::iterator it = directories.begin(); it != directories.end(); it++){
        const char* iterator_directory_name = (const char*) (*it).name;
        if(!strcmp(name, iterator_directory_name)){
            found = *it;
            directory = &found;
            break;
        }
    }

    return directory;
}

// Imprime informações de uma entrada de diretório
void print_directory(Directory directory){
    cout <<  directory.name << endl;
    cout << "inode:\t\t\t" << (unsigned) directory.inode << endl;
    cout << "record lenght:\t\t" << (unsigned) directory.rec_len << endl;
    cout << "name lenght:\t\t" << (unsigned) directory.name_len << endl;
    cout << "file type:\t\t" << (unsigned) directory.file_type << endl;
}

// Imprime todas as entradas de um vetor de diretórios
void print_directories(vector<Directory> directories) {
    for(vector<Directory>::iterator it = directories.begin(); it != directories.end(); it++){
        print_directory(*it);
        cout << endl;
    }
}