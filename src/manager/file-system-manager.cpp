#include "../superblock/superblock.hpp"
#include "file-system-manager.hpp"
#include "../blocks-group-descriptor/blocks-group-descriptor.hpp"
#include "../inode/inode.hpp"
#include "../error/error.hpp"
#include "../utils/utils.hpp"
#include <cstring>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#define BLOCK_SIZE 1024

using namespace std;

FileSystemManager::FileSystemManager(FILE *image){
  this->image = image;
  this->superblock = read_ext2_superblock(this->image);
  this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(0));
  Inode* first_inode = read_inode(this->image, this->bgd, 2);
  vector<Directory> directories = read_directories(this->image, first_inode);
  this->navigation.push_back(directories.at(0));
}

void FileSystemManager::info(){
  uint32_t group_count = this->superblock->s_blocks_per_group/BLOCK_SIZE;
  uint32_t inode_table_blocks = this->superblock->s_inodes_per_group/group_count;

  cout << "Volume name.....: " << this->superblock->s_volume_name << endl;
  cout << "Image size......: " << this->superblock->s_blocks_count << " KiB" << endl;
  cout << "Free space......: " << this->superblock->s_free_blocks_count << " KiB" << endl;
  cout << "Free inodes.....: " << this->superblock->s_free_inodes_count << endl;
  cout << "Free blocks.....: " << this->superblock->s_free_blocks_count << endl;
  cout << "Blocks size.....: " << BLOCK_SIZE << " bytes" << endl;
  cout << "Inode size......: " << this->superblock->s_inode_size << " bytes" << endl;
  cout << "Group count.....: " << group_count << endl;
  cout << "Group size......: " << this->superblock->s_blocks_per_group << " blocks" << endl;
  cout << "Group inodes....: " << this->superblock->s_inodes_per_group << " inodes" << endl;
  cout << "Inodetable size.: " << inode_table_blocks << " blocks" << endl;
}

void FileSystemManager::superblock_info(){
  print_superblock(this->superblock);
}

void FileSystemManager::blocks_group_descriptor_info(int index) {
  uint32_t address = block_group_descriptor_address(index);
  BlocksGroupDescriptor *bgd = read_blocks_group_descriptor(this->image, address);
  print_blocks_group_descriptor(bgd);
}

void FileSystemManager::inode_info(unsigned int inode) {
  unsigned int inode_bgd = block_group_from_inode(this->superblock, inode);
  BlocksGroupDescriptor *bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(inode_bgd));
  Inode *found_inode = read_inode(this->image, bgd, inode_order(this->superblock, inode));
  print_inode(found_inode);
}

void FileSystemManager::cat(const char *directory_name) {

  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_order(this->superblock, actual_directory.inode));

  Directory *directory = search_directory(this->image, actual_inode, directory_name);

  if(!directory) throw new Error("file not found.");
  if(directory->file_type != 1) throw new Error("it's not a file.");

  unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, directory->inode);
  BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));

  Inode *directory_inode = read_inode(this->image, bgd_of_inode, inode_order(this->superblock, directory->inode));
  print_inode_blocks_content(this->image, directory_inode);
}

void FileSystemManager::ls() {
  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_order(this->superblock, actual_directory.inode));
  vector<Directory> directories = read_directories(this->image, actual_inode);
  print_directories(directories);
}

string FileSystemManager::pwd() {
  string str;
  for (vector<Directory>::iterator i = this->navigation.begin(); i != this->navigation.end(); i++)
  {
    if (std::strcmp((*i).name, "."))
      str = str.append((*i).name);
    str = str.append("/");
  }

  return str;
}

void FileSystemManager::cd(const char *directory_name) {

  if (!std::strcmp(directory_name, ".")) {
    print_directory(this->navigation.back());
    return;
  }
  if (!std::strcmp(directory_name, "..") && this->navigation.size() == 1)
    throw new Error("no directories to go back.");

  if (!std::strcmp(directory_name, "..")) {
    this->navigation.pop_back();
    Directory actual_directory = this->navigation.back();
    unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, actual_directory.inode);
    this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));
    print_directory(this->navigation.back());
    return;
  }

  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_order(this->superblock, actual_directory.inode));
  Directory *directory = search_directory(this->image, actual_inode, directory_name);

  if(!directory) throw new Error("directory not found.");
  if (directory->file_type != 2)  throw new Error("it's not a directory.");

  unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, directory->inode);
  this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));

  this->navigation.push_back(*directory);
  print_directory(this->navigation.back());
}

void FileSystemManager::attr(const char *directory_name){
  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_order(this->superblock, actual_directory.inode));

  Directory *directory = search_directory(this->image, actual_inode, directory_name);

  if(!directory) throw new Error("not found.");

  unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, directory->inode);
  BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));
  Inode *directory_inode = read_inode(this->image, bgd_of_inode, inode_order(this->superblock, directory->inode));
  
  string permission = get_i_mode_permissions(directory_inode->i_mode); 

  float size_fk = (float) directory_inode->i_size / 1024;
  float size_fm = (float) directory_inode->i_size / (1024 * 1024);

  cout << "permissions\t" << "uid\t" << "gid\t" << "size\t\t\t" << "modified on\t" << endl;
  cout << permission << "\t" << (unsigned)directory_inode->i_uid << "\t";
  cout << (unsigned)directory_inode->i_gid << "\t";
  if(directory_inode->i_size < 1024) cout << (unsigned) directory_inode->i_size << " bytes";
  else if(directory_inode->i_size < (1024 * 1024)) cout << setprecision(2) << size_fk << " KiB";
  else cout << setprecision(2) << size_fm << " MiB  ";
  cout << "\t\t"; 
  print_time((unsigned)directory_inode->i_mtime);
}

void FileSystemManager::cp(const char *source_path, const char *target_path) {
    // Tamanho do bloco fixo conforme especificação
    const uint32_t block_size = BLOCK_SIZE;
    char block[BLOCK_SIZE];

    // 1. Localizar o arquivo na imagem EXT2 (diretório corrente)
    Directory actual_directory = this->navigation.back();
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_order(this->superblock, actual_directory.inode));
    Directory *file_entry = search_directory(this->image, actual_inode, source_path);

    if (!file_entry)
        throw new Error("cp: arquivo não encontrado.");

    if (file_entry->file_type != 1)
        throw new Error("cp: não é um arquivo regular.");

    // 2. Ler o inode do arquivo
    unsigned int file_inode_block_group = block_group_from_inode(this->superblock, file_entry->inode);
    BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(file_inode_block_group));
    Inode *file_inode = read_inode(this->image, bgd_of_inode, inode_order(this->superblock, file_entry->inode));

    // 3. Determinar caminho de destino
    std::string full_target = target_path;
    struct stat st;
    bool is_dir = false;
    if (stat(target_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        is_dir = true;
        if (full_target.back() != '/')
            full_target += "/";
        full_target += source_path;
    }

    // 4. Abrir arquivo de destino para escrita
    std::ofstream out(full_target, std::ios::binary);
    if (!out.is_open())
        throw new Error("cp: não foi possível criar o arquivo de destino.");

    uint32_t bytes_remaining = file_inode->i_size;

    // 5. Copiar blocos diretos
    for (int i = 0; i < 12 && bytes_remaining > 0; i++) {
        if (file_inode->i_block[i] == 0)
            continue;
        fseek(this->image, file_inode->i_block[i] * block_size, SEEK_SET);
        uint32_t to_read = (bytes_remaining < block_size) ? bytes_remaining : block_size;
        fread(block, 1, to_read, this->image);
        out.write(block, to_read);
        bytes_remaining -= to_read;
    }

    // 6. Copiar blocos indiretos simples (se houver)
    if (bytes_remaining > 0 && file_inode->i_block[12] != 0) {
        uint32_t indirect[256];
        fseek(this->image, file_inode->i_block[12] * block_size, SEEK_SET);
        fread(indirect, sizeof(uint32_t), 256, this->image);

        for (int i = 0; i < 256 && bytes_remaining > 0; i++) {
            if (indirect[i] == 0)
                continue;
            fseek(this->image, indirect[i] * block_size, SEEK_SET);
            uint32_t to_read = (bytes_remaining < block_size) ? bytes_remaining : block_size;
            fread(block, 1, to_read, this->image);
            out.write(block, to_read);
            bytes_remaining -= to_read;
        }
    }

    out.close();
    std::cout << "Arquivo '" << source_path << "' copiado para '" << full_target << "' com sucesso." << std::endl;
}