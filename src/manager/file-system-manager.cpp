#include "../superblock/superblock.hpp"
#include "file-system-manager.hpp"
#include "../blocks-group-descriptor/blocks-group-descriptor.hpp"
#include "../inode/inode.hpp"
#include "../error/error.hpp"
#include "../utils/utils.hpp"
#include <cstring>
#include <iomanip>
#define BLOCK_SIZE 1024

using namespace std;

FileSystemManager::FileSystemManager(FILE *image){
  this->image = image; //inicializa a imagem
  this->superblock = read_ext2_superblock(this->image); //lê o superbloco
  this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(0)); //carrega o descritor do block group 0
  Inode* first_inode = read_inode(this->image, this->bgd, 2); //carrega o inode do diretório raíz(2)
  vector<Directory> directories = read_directories(this->image, first_inode); //extrai todas as entradas (arquivos/diretórios) da raíz
  this->navigation.push_back(directories.at(0)); //inicializa o vetor navigation com o diretório raíz
}

void FileSystemManager::info(){

  // Total de grupos no sistema
  uint32_t group_count = this->superblock->s_blocks_count / this->superblock->s_blocks_per_group;

  // Blocos ocupados pela tabela de inodes em cada grupo
  uint32_t inode_table_blocks = (this->superblock->s_inodes_per_group * this->superblock->s_inode_size) / BLOCK_SIZE;

  // Cálculo do espaço livre (descontando blocos reservados)
  uint32_t usable_free_blocks = this->superblock->s_free_blocks_count - this->superblock->s_r_blocks_count;

  cout << "Volume name.....: " << this->superblock->s_volume_name << endl;
  cout << "Image size......: " << (this->superblock->s_blocks_count * BLOCK_SIZE) << " bytes" << endl;
  cout << "Free space......: " << (usable_free_blocks * BLOCK_SIZE / 1024) << " KiB" << endl;
  cout << "Free inodes.....: " << this->superblock->s_free_inodes_count << endl;
  cout << "Free blocks.....: " << this->superblock->s_free_blocks_count << " blocks" << endl;
  cout << "Blocks size.....: " << BLOCK_SIZE << " bytes" << endl;
  cout << "Inode size......: " << this->superblock->s_inode_size << " bytes" << endl;
  cout << "Group count.....: " << group_count << endl;
  cout << "Group size......: " << this->superblock->s_blocks_per_group << " blocks" << endl;
  cout << "Group inodes....: " << this->superblock->s_inodes_per_group << " inodes" << endl;
  cout << "Inodetable size.: " << inode_table_blocks << " blocks" << endl;
}

void FileSystemManager::superblock_info(){
  print_superblock(this->superblock); //imprime os campos do superbloco
}

void FileSystemManager::blocks_group_descriptor_info(int index) {
  uint32_t address = block_group_descriptor_address(index);//retorna o endereço em que se encontra o bgd de index especificado
  BlocksGroupDescriptor *bgd = read_blocks_group_descriptor(this->image, address); //lê os bytes do bgd
  print_blocks_group_descriptor(bgd);
}

void FileSystemManager::inode_info(unsigned int inode) {
  unsigned int inode_bgd = block_group_from_inode(this->superblock, inode);//retorna a qual block group o inode pertence
  BlocksGroupDescriptor *bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(inode_bgd)); //lê os bytes do bgd
  Inode *found_inode = read_inode(this->image, bgd, inode_relative_position(this->superblock, inode));//lê o inode
  print_inode(found_inode);
}

void FileSystemManager::cat(const char *directory_name) {

  // obtém o diretório atual
  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);

  // leitura do inode do diretório atual;
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));

  // procura o arquivo pelo nome no conteúdo do diretório, retornando uma struct Directory se achar
  Directory *directory = search_directory(this->image, actual_inode, directory_name);

  if(!directory) throw new Error("file not found."); // verifica se existe
  if(directory->file_type != 1) throw new Error("it's not a file."); // verifica se é arquivo tipo 1

  // identifica em qual grupo de blocos o inode do arquivo encontrado está;
  unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, directory->inode);

  // lê o descritor do grupo identificado;
  BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));

  // lê inode do arquivo
  Inode *directory_inode = read_inode(this->image, bgd_of_inode, inode_relative_position(this->superblock, directory->inode));

  // imprime conteúdo
  print_inode_blocks_content(this->image, directory_inode);
}

void FileSystemManager::ls() {
  /*
    acessa a última entrada no vetor navigation (pilha que armazena o caminho percorrido). 
  */
  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);

  // lê o inode do diretório atual
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));

  // lê todas as entradas do diretório (arquivos/subdiretórios)
  vector<Directory> directories = read_directories(this->image, actual_inode);
  print_directories(directories);
}

string FileSystemManager::pwd() {
  string str; //caminho final

  // i inicia com o primeiro elemento de navigation (caminho percorrido) e vai até o último
  for (vector<Directory>::iterator i = this->navigation.begin(); i != this->navigation.end(); i++)
  {
    if (std::strcmp((*i).name, ".")) //ignora o "." na impressão do caminho
      str = str.append((*i).name); // adiciona o nome do diretório à string final
    str = str.append("/"); //concatena /
  }

  return str;
}

void FileSystemManager::cd(const char *directory_name) {

  // se o nome for ".", imprime o diretório atual
  if (!std::strcmp(directory_name, ".")) {
    print_directory(this->navigation.back());
    return;
  }

  // proíbe o usuário de subir além da raíz
  if (!std::strcmp(directory_name, "..") && this->navigation.size() == 1)
    throw new Error("no directories to go back.");

  // processa ".."
  if (!std::strcmp(directory_name, "..")) {
    this->navigation.pop_back();
    Directory actual_directory = this->navigation.back();
    unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, actual_directory.inode);
    this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));
    print_directory(this->navigation.back());
    return;
  }

  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);

  // lê o inode do diretório atual
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));

  // encontra o diretório de destino no conteúdo do diretório atual.
  Directory *directory = search_directory(this->image, actual_inode, directory_name);

  if(!directory) throw new Error("directory not found."); //verifica se existe
  if (directory->file_type != 2)  throw new Error("it's not a directory."); //verifica se é tipo 2 (diretório)

  unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, directory->inode);

  // atualiza bgd
  this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));

  // atualiza navigation com o novo diretório
  this->navigation.push_back(*directory);
  print_directory(this->navigation.back());
}

void FileSystemManager::attr(const char *directory_name){

  // acessa o diretório atual
  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);

  // lê inode do diretório atual
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));

  // busca pela entrada do arquivo/diretório
  Directory *directory = search_directory(this->image, actual_inode, directory_name);

  if(!directory) throw new Error("not found."); //verifica se existe

  // calcula o grupo de blocos onde o inode do alvo está
  unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, directory->inode);

  // carrega o bgd correspondente
  BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));

  // lê o inode do arquivo/diretório atual
  Inode *directory_inode = read_inode(this->image, bgd_of_inode, inode_relative_position(this->superblock, directory->inode));
  
  // imprime atributos
  string permission = get_i_mode_permissions(directory_inode->i_mode); 

  float size_kb = (float) directory_inode->i_size / 1024;
  float size_mb = (float) directory_inode->i_size / (1024 * 1024);

  cout << "permissions\t" << "uid\t" << "gid\t" << "size\t\t\t" << "modified on\t" << endl;
  cout << permission << "\t" << (unsigned)directory_inode->i_uid << "\t";
  cout << (unsigned)directory_inode->i_gid << "\t";

  //exibe em bytes se pequeno
  if(directory_inode->i_size < 1024) cout << (unsigned) directory_inode->i_size << " bytes";

  //exibe em KB para arquivos médios (1KB <= tamanho < 1MB)
  else if(directory_inode->i_size < (1024 * 1024)) cout << setprecision(2) << size_kb << " KiB";

  //exibe em MB para arquivos com tamanho maior que 1MB
  else cout << setprecision(2) << size_mb << " MiB  ";
  cout << "\t\t"; 
  print_time((unsigned)directory_inode->i_mtime);
}