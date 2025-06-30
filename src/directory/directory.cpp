#include "directory.hpp"
#include <sys/stat.h>
#include <cstring>
#define BLOCK_SIZE 1024
#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block - 1) * BLOCK_SIZE)

typedef char Block[BLOCK_SIZE];

vector<Directory> read_directories(FILE* image, Inode* inode) {

    vector<Directory> directories;
  
    if(S_ISDIR(inode->i_mode)) {
  
      for(int i = 0; i < 12; i++){
        Block* block = (Block*)malloc(sizeof(Block));
  
        if(!inode->i_block[i]) break;
  
        fseek(image, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
        fread(block, 1, sizeof(Block), image);
  
        
        Directory* worked_directory = (Directory *) block;
        int block_position = 0;
        do{
          Directory* directory = (Directory*) malloc(sizeof(Directory));
          memcpy(directory, worked_directory, sizeof(Directory));
          (directory->name)[directory->name_len] = '\0';
          directories.push_back(*directory);
          worked_directory = (Directory *) ((char*) worked_directory + worked_directory->rec_len);
          block_position += worked_directory->rec_len;
        }while((block_position < inode->i_size) && worked_directory->inode);
  
        free(block);
      }
    }
  
    return directories;
}

Directory* search_directory(FILE* image, Inode* inode, const char* name){
    Directory* directory = NULL;
  
    vector<Directory> directories = read_directories(image, inode);
    
    for(vector<Directory>::iterator it = directories.begin(); it != directories.end(); it++){
      const char* iterator_directory_name = (const char*) (*it).name;
      if(!strcmp(name, iterator_directory_name)){
        directory = &(*it);
        break;
      }
    }
  
    return directory;
}

void print_directory(Directory directory){
  cout <<  directory.name << endl;
  cout << "inode:\t\t\t" << (unsigned) directory.inode << endl;
  cout << "record lenght:\t\t" << (unsigned) directory.rec_len << endl;
  cout << "name lenght:\t\t" << (unsigned) directory.name_len << endl;
  cout << "file type:\t\t" << (unsigned) directory.file_type << endl;
}

void print_directories(vector<Directory> directories) {
  for(vector<Directory>::iterator it = directories.begin(); it != directories.end(); it++){
    print_directory(*it);
    cout << endl;
  }
}