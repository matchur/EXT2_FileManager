#include "../superblock/superblock.hpp"
#include "file-system-manager.hpp"
#include "../blocks-group-descriptor/blocks-group-descriptor.hpp"
#include "../inode/inode.hpp"
#include "../error/error.hpp"
#include "../utils/utils.hpp"
#include "../utils/bitmap-utils.hpp"
#include <cstring>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#define BLOCK_SIZE 1024

using namespace std;

// Construtor da classe FileSystemManager
// Inicializa o gerenciador de sistema de arquivos carregando o superbloco, descritor de grupo e diretório raiz.
FileSystemManager::FileSystemManager(FILE *image) {
    this->image = image;
    // Lê o superbloco da imagem EXT2
    this->superblock = read_ext2_superblock(image);
    // Lê o descritor de grupo de blocos (assumindo grupo 0)
    this->bgd = read_blocks_group_descriptor(image, block_group_descriptor_address(0));
    // Inicializa a navegação com o diretório raiz
    Directory root;
    root.inode = 2; // inode 2 é o diretório raiz no EXT2
    strcpy(root.name, "/");
    this->navigation.push_back(root);
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

// Exibe informações do descritor de grupo de índice especificado
void FileSystemManager::blocks_group_descriptor_info(int index) {
    BlocksGroupDescriptor *bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(index));
    print_blocks_group_descriptor(bgd);
    free(bgd); // Libera memória alocada
}

// Exibe informações detalhadas de um inode específico
void FileSystemManager::inode_info(unsigned int inode) {
  unsigned int inode_bgd = block_group_from_inode(this->superblock, inode);//retorna a qual block group o inode pertence
  BlocksGroupDescriptor *bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(inode_bgd)); //lê os bytes do bgd
  Inode *found_inode = read_inode(this->image, bgd, inode_relative_position(this->superblock, inode));//lê o inode
  print_inode(found_inode);
  free(node); // Libera memória alocada
  free(bgd);  // Libera também o bgd alocado
}

// Exibe o conteúdo de um arquivo no formato texto
// Procura o arquivo no diretório atual, lê seu inode e imprime o conteúdo.
void FileSystemManager::cat(const char *directory_name) {
    Directory actual_directory = this->navigation.back();
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));
    Directory *entry = search_directory(this->image, actual_inode, directory_name);
    if (!entry || entry->file_type != 1) {
        std::cout << "cat: arquivo não encontrado ou não é um arquivo regular." << std::endl;
        free(actual_inode);
        return;
    }
    unsigned int file_inode_block_group = block_group_from_inode(this->superblock, entry->inode);
    BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(file_inode_block_group));
    Inode *file_inode = read_inode(this->image, bgd_of_inode, inode_relative_position(this->superblock, entry->inode));
    char buffer[BLOCK_SIZE + 1] = {0};
    for (int i = 0; i < 12 && file_inode->i_block[i] != 0; i++) {
        fseek(this->image, file_inode->i_block[i] * BLOCK_SIZE, SEEK_SET);
        fread(buffer, 1, BLOCK_SIZE, this->image);
        buffer[BLOCK_SIZE] = '\0';
        std::cout << buffer;
    }
    std::cout << std::endl;
    free(file_inode);      // Libera memória alocada
    free(bgd_of_inode);    // Libera memória alocada
    free(actual_inode);    // Libera memória alocada
}

// Lista os arquivos e diretórios do diretório corrente
// Percorre as entradas do diretório atual e imprime informações detalhadas de cada uma.
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

// Retorna o caminho absoluto do diretório corrente
// Constrói o caminho a partir do vetor de navegação.
string FileSystemManager::pwd() {
    if (this->navigation.size() == 1)
        return "/";
    std::string path;
    for (size_t i = 1; i < this->navigation.size(); ++i) {
        path += "/";
        // Garante que o nome será lido corretamente, mesmo sem \0
        path.append(this->navigation[i].name, strnlen(this->navigation[i].name, EXT2_NAME_LEN));
    }
    return path.empty() ? "/" : path;
}

// Altera o diretório corrente para o especificado
// Procura o diretório pelo nome e atualiza o vetor de navegação.
void FileSystemManager::cd(const char *directory_name) {
    if (strcmp(directory_name, ".") == 0) {
        // Não faz nada, permanece no diretório atual
        return;
    }
    if (strcmp(directory_name, "..") == 0) {
        // Volta para o diretório pai, se não estiver na raiz
        if (this->navigation.size() > 1) {
            this->navigation.pop_back();
        }
        return;
    }
    Directory actual_directory = this->navigation.back();
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));
    Directory *entry = search_directory(this->image, actual_inode, directory_name);
    if (!entry || entry->file_type != 2) {
        std::cout << "cd: diretório não encontrado." << std::endl;
        free(actual_inode);
        return;
    }
    Directory new_dir;
    new_dir.inode = entry->inode;
    strncpy(new_dir.name, directory_name, EXT2_NAME_LEN);
    this->navigation.push_back(new_dir);
    free(actual_inode);
}

// Exibe os atributos de um arquivo ou diretório
// Mostra informações do inode correspondente ao nome informado.
void FileSystemManager::attr(const char *directory_name) {
    Directory actual_directory = this->navigation.back();
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));
    Directory *entry = search_directory(this->image, actual_inode, directory_name);
    if (!entry) {
        std::cout << "attr: arquivo ou diretório não encontrado." << std::endl;
        return;
    }
    unsigned int inode_block_group = block_group_from_inode(this->superblock, entry->inode);
    BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(inode_block_group));
    Inode *node = read_inode(this->image, bgd_of_inode, inode_relative_position(this->superblock, entry->inode));
    print_inode(node);
}

void FileSystemManager::cp(const char *source_path, const char *target_path) {
    // Tamanho do bloco fixo conforme especificação
    const uint32_t block_size = BLOCK_SIZE;
    char block[BLOCK_SIZE];

    // 1. Localizar o arquivo na imagem EXT2 (diretório corrente)
    Directory actual_directory = this->navigation.back();
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));
    Directory *file_entry = search_directory(this->image, actual_inode, source_path);

    if (!file_entry)
        throw new Error("cp: arquivo não encontrado.");

    if (file_entry->file_type != 1)
        throw new Error("cp: não é um arquivo regular.");

    // 2. Ler o inode do arquivo
    unsigned int file_inode_block_group = block_group_from_inode(this->superblock, file_entry->inode);
    BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(file_inode_block_group));
    Inode *file_inode = read_inode(this->image, bgd_of_inode, inode_relative_position(this->superblock, file_entry->inode));

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

// Cria um novo arquivo vazio no diretório atual
void FileSystemManager::touch(const char *file_name) {
    Directory actual_directory = this->navigation.back();
    // Lê o inode do diretório atual uma única vez e reutiliza
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));
    Directory *existing = search_directory(this->image, actual_inode, file_name);
    if (existing) {
        std::cout << "touch: arquivo '" << file_name << "' já existe." << std::endl;
        free(actual_inode);
        return;
    }

    // Usa funções do bitmap-utils para manipular o bitmap de inodes
    uint8_t inode_bitmap[BLOCK_SIZE];
    read_bitmap(this->image, this->bgd->bg_inode_bitmap, inode_bitmap, BLOCK_SIZE);

    uint32_t inode_count = this->superblock->s_inodes_count;
    uint32_t first_inode = 0;
    for (uint32_t i = 0; i < inode_count; i++) {
        if (!is_bitmap_bit_set(inode_bitmap, i)) {
            first_inode = i + 1;
            set_bitmap_bit(inode_bitmap, i);
            break;
        }
    }
    if (first_inode == 0) {
        std::cout << "touch: não há inodes livres." << std::endl;
        free(actual_inode);
        return;
    }
    write_bitmap(this->image, this->bgd->bg_inode_bitmap, inode_bitmap, BLOCK_SIZE);

    this->superblock->s_free_inodes_count--;
    fseek(this->image, 1024, SEEK_SET);
    fwrite(this->superblock, sizeof(Superblock), 1, this->image);

    this->bgd->bg_free_inodes_count--;
    fseek(this->image, block_group_descriptor_address(0), SEEK_SET);
    fwrite(this->bgd, sizeof(BlocksGroupDescriptor), 1, this->image);

    Inode new_inode = {};
    new_inode.i_mode = 0x81A4; // arquivo regular, permissão 644
    new_inode.i_uid = 0;
    new_inode.i_gid = 0;
    new_inode.i_size = 0;
    new_inode.i_links_count = 1;
    new_inode.i_blocks = 0;
    new_inode.i_atime = new_inode.i_ctime = new_inode.i_mtime = time(nullptr);

    uint32_t inode_table_block = this->bgd->bg_inode_table;
    uint32_t inode_size = this->superblock->s_inode_size;
    uint32_t inode_offset = inode_table_block * BLOCK_SIZE + (first_inode - 1) * inode_size;
    fseek(this->image, inode_offset, SEEK_SET);
    fwrite(&new_inode, inode_size, 1, this->image);

    // Lê o bloco de diretório uma única vez e reutiliza
    uint32_t dir_block = actual_inode->i_block[0];
    uint8_t buffer[BLOCK_SIZE];
    fseek(this->image, dir_block * BLOCK_SIZE, SEEK_SET);
    fread(buffer, 1, BLOCK_SIZE, this->image);

    uint32_t offset = 0;
    while (offset < BLOCK_SIZE) {
        Directory *entry = (Directory *)(buffer + offset);
        uint32_t real_len = 8 + entry->name_len + ((4 - (entry->name_len % 4)) % 4);
        if (entry->rec_len > real_len) {
            uint16_t old_rec_len = entry->rec_len;
            entry->rec_len = real_len;

            Directory *new_entry = (Directory *)(buffer + offset + real_len);
            new_entry->inode = first_inode;
            new_entry->rec_len = old_rec_len - real_len;
            new_entry->name_len = strlen(file_name);
            new_entry->file_type = 1; // arquivo regular
            memset(new_entry->name, 0, EXT2_NAME_LEN);
            strncpy(new_entry->name, file_name, EXT2_NAME_LEN);

            fseek(this->image, dir_block * BLOCK_SIZE, SEEK_SET);
            fwrite(buffer, 1, BLOCK_SIZE, this->image);

            std::cout << "touch: arquivo '" << file_name << "' criado." << std::endl;
            free(actual_inode);
            return;
        }
        offset += entry->rec_len;
    }
    std::cout << "touch: não há espaço no diretório para criar o arquivo." << std::endl;
    free(actual_inode);
}

// Remove um arquivo do diretório atual
void FileSystemManager::rm(const char *file_name) {
    Directory actual_directory = this->navigation.back();
    // Lê o inode do diretório atual uma única vez e reutiliza
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));
    Directory *entry = search_directory(this->image, actual_inode, file_name);
    if (!entry) {
        std::cout << "rm: arquivo '" << file_name << "' não encontrado." << std::endl;
        free(actual_inode);
        return;
    }
    if (entry->file_type != 1) {
        std::cout << "rm: '" << file_name << "' não é um arquivo regular." << std::endl;
        free(actual_inode);
        return;
    }

    // Lê o inode do arquivo a ser removido uma única vez e reutiliza
    unsigned int file_inode_block_group = block_group_from_inode(this->superblock, entry->inode);
    BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(file_inode_block_group));
    Inode *file_inode = read_inode(this->image, bgd_of_inode, inode_relative_position(this->superblock, entry->inode));

    uint32_t dir_block = actual_inode->i_block[0];
    uint8_t buffer[BLOCK_SIZE];
    fseek(this->image, dir_block * BLOCK_SIZE, SEEK_SET);
    fread(buffer, 1, BLOCK_SIZE, this->image);

    uint32_t offset = 0;
    uint32_t prev_offset = 0;
    Directory *prev_entry = nullptr;
    while (offset < BLOCK_SIZE) {
        Directory *curr_entry = (Directory *)(buffer + offset);
        if (strncmp(curr_entry->name, file_name, curr_entry->name_len) == 0 && curr_entry->name_len == strlen(file_name)) {
            if (prev_entry) {
                prev_entry->rec_len += curr_entry->rec_len;
            } else {
                curr_entry->inode = 0;
            }
            break;
        }
        prev_offset = offset;
        prev_entry = curr_entry;
        offset += curr_entry->rec_len;
    }
    fseek(this->image, dir_block * BLOCK_SIZE, SEEK_SET);
    fwrite(buffer, 1, BLOCK_SIZE, this->image);

    // Libera o inode no bitmap de inodes usando bitmap-utils
    uint8_t inode_bitmap[BLOCK_SIZE];
    read_bitmap(this->image, this->bgd->bg_inode_bitmap, inode_bitmap, BLOCK_SIZE);
    uint32_t inode_idx = entry->inode - 1;
    clear_bitmap_bit(inode_bitmap, inode_idx);
    write_bitmap(this->image, this->bgd->bg_inode_bitmap, inode_bitmap, BLOCK_SIZE);

    this->superblock->s_free_inodes_count++;
    fseek(this->image, 1024, SEEK_SET);
    fwrite(this->superblock, sizeof(Superblock), 1, this->image);

    this->bgd->bg_free_inodes_count++;
    fseek(this->image, block_group_descriptor_address(0), SEEK_SET);
    fwrite(this->bgd, sizeof(BlocksGroupDescriptor), 1, this->image);

    // Libera blocos de dados do arquivo usando bitmap-utils
    uint8_t block_bitmap[BLOCK_SIZE];
    read_bitmap(this->image, this->bgd->bg_block_bitmap, block_bitmap, BLOCK_SIZE);
    for (int i = 0; i < 12; i++) {
        if (file_inode->i_block[i] != 0) {
            uint32_t block_idx = file_inode->i_block[i] - 1;
            clear_bitmap_bit(block_bitmap, block_idx);
        }
    }
    write_bitmap(this->image, this->bgd->bg_block_bitmap, block_bitmap, BLOCK_SIZE);

    std::cout << "rm: arquivo '" << file_name << "' removido." << std::endl;
    free(file_inode);
    free(bgd_of_inode);
    free(actual_inode);
}

// Renomeia um arquivo ou diretório no diretório atual
void FileSystemManager::rename(const char *old_name, const char *new_name) {
    // Obtém o diretório atual
    Directory actual_directory = this->navigation.back();
    // Lê o inode do diretório atual
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));

    // Procura a entrada do arquivo/diretório a ser renomeado
    Directory *entry = search_directory(this->image, actual_inode, old_name);
    if (!entry) {
        std::cout << "rename: '" << old_name << "' não encontrado." << std::endl;
        free(actual_inode);
        return;
    }

    // Verifica se já existe uma entrada com o novo nome
    Directory *conflict = search_directory(this->image, actual_inode, new_name);
    if (conflict) {
        std::cout << "rename: já existe um arquivo ou diretório com o nome '" << new_name << "'." << std::endl;
        free(actual_inode);
        return;
    }

    // Atualiza o nome na entrada de diretório
    uint32_t dir_block = actual_inode->i_block[0];
    uint8_t buffer[BLOCK_SIZE];
    fseek(this->image, dir_block * BLOCK_SIZE, SEEK_SET);
    fread(buffer, 1, BLOCK_SIZE, this->image);

    uint32_t offset = 0;
    while (offset < BLOCK_SIZE) {
        Directory *curr_entry = (Directory *)(buffer + offset);
        if (strncmp(curr_entry->name, old_name, curr_entry->name_len) == 0 && curr_entry->name_len == strlen(old_name)) {
            // Limpa o nome antigo e copia o novo nome
            memset(curr_entry->name, 0, EXT2_NAME_LEN);
            strncpy(curr_entry->name, new_name, EXT2_NAME_LEN);
            curr_entry->name_len = strlen(new_name);
            break;
        }
        offset += curr_entry->rec_len;
    }

    // Escreve o bloco de diretório atualizado
    fseek(this->image, dir_block * BLOCK_SIZE, SEEK_SET);
    fwrite(buffer, 1, BLOCK_SIZE, this->image);

    std::cout << "rename: '" << old_name << "' renomeado para '" << new_name << "'." << std::endl;
    free(actual_inode);

}

// Cria um novo diretório no diretório atual
void FileSystemManager::mkdir(const char *dir_name) {
    // Obtém o diretório atual

    Directory actual_directory = this->navigation.back();
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));

    // Verifica se já existe um diretório com esse nome
    Directory *existing = search_directory(this->image, actual_inode, dir_name);
    if (existing) {
        std::cout << "mkdir: diretório '" << dir_name << "' já existe." << std::endl;
        free(actual_inode);
        return;
    }

    // Procura um inode livre no bitmap de inodes
    uint32_t inode_count = this->superblock->s_inodes_count;
    uint32_t first_inode = 0;
    uint8_t buffer[BLOCK_SIZE];
    read_bitmap(this->image, this->bgd->bg_inode_bitmap, buffer, BLOCK_SIZE);

    for (uint32_t i = 0; i < inode_count; i++) {
        if (!is_bitmap_bit_set(buffer, i)) {
            first_inode = i + 1;
            set_bitmap_bit(buffer, i);
            break;
        }
    }
    if (first_inode == 0) {
        std::cout << "mkdir: não há inodes livres." << std::endl;
        free(actual_inode);
        return;
    }

    write_bitmap(this->image, this->bgd->bg_inode_bitmap, buffer, BLOCK_SIZE);

    // Atualiza superbloco e bgd
    this->superblock->s_free_inodes_count--;
    fseek(this->image, 1024, SEEK_SET);
    fwrite(this->superblock, sizeof(Superblock), 1, this->image);

    this->bgd->bg_free_inodes_count--;
    fseek(this->image, block_group_descriptor_address(0), SEEK_SET);
    fwrite(this->bgd, sizeof(BlocksGroupDescriptor), 1, this->image);

    // Procura um bloco livre para o novo diretório
    uint32_t block_count = this->superblock->s_blocks_count;
    uint32_t first_block = 0;
    read_bitmap(this->image, this->bgd->bg_block_bitmap, buffer, BLOCK_SIZE);

    for (uint32_t i = 0; i < block_count; i++) {
        if (!is_bitmap_bit_set(buffer, i)) {
            first_block = i + 1;
            set_bitmap_bit(buffer, i);
            break;
        }
    }
    if (first_block == 0) {
        std::cout << "mkdir: não há blocos livres." << std::endl;
        free(actual_inode);
        return;
    }

    write_bitmap(this->image, this->bgd->bg_block_bitmap, buffer, BLOCK_SIZE);

    // Atualiza superbloco e bgd para blocos
    this->superblock->s_free_blocks_count--;
    fseek(this->image, 1024, SEEK_SET);
    fwrite(this->superblock, sizeof(Superblock), 1, this->image);

    this->bgd->bg_free_blocks_count--;
    fseek(this->image, block_group_descriptor_address(0), SEEK_SET);
    fwrite(this->bgd, sizeof(BlocksGroupDescriptor), 1, this->image);

    // Inicializa o novo inode como diretório
    Inode new_inode = {};
    new_inode.i_mode = 0x41ED; // diretório, permissão 755
    new_inode.i_uid = 0;
    new_inode.i_gid = 0;
    new_inode.i_size = BLOCK_SIZE;
    new_inode.i_links_count = 2; // "." e ".."
    new_inode.i_blocks = 2; // em blocos de 512 bytes
    new_inode.i_atime = new_inode.i_ctime = new_inode.i_mtime = time(nullptr);
    new_inode.i_block[0] = first_block;
    for (int i = 1; i < 15; i++) new_inode.i_block[i] = 0;

    // Escreve o novo inode na tabela de inodes
    uint32_t inode_table_block = this->bgd->bg_inode_table;
    uint32_t inode_size = this->superblock->s_inode_size;
    uint32_t inode_offset = inode_table_block * BLOCK_SIZE + (first_inode - 1) * inode_size;
    fseek(this->image, inode_offset, SEEK_SET);
    fwrite(&new_inode, inode_size, 1, this->image);

    // Cria as entradas "." e ".." no novo diretório
    uint8_t dir_block[BLOCK_SIZE] = {0};
    Directory *dot = (Directory *)dir_block;
    dot->inode = first_inode;
    dot->rec_len = 12;
    dot->name_len = 1;
    dot->file_type = 2;
    strcpy(dot->name, ".");

    Directory *dotdot = (Directory *)(dir_block + 12);
    dotdot->inode = actual_directory.inode;
    dotdot->rec_len = BLOCK_SIZE - 12;
    dotdot->name_len = 2;
    dotdot->file_type = 2;
    strcpy(dotdot->name, "..");

    // Escreve o bloco do novo diretório
    fseek(this->image, first_block * BLOCK_SIZE, SEEK_SET);
    fwrite(dir_block, 1, BLOCK_SIZE, this->image);

    // Adiciona a entrada do novo diretório no diretório pai
    uint32_t parent_block = actual_inode->i_block[0];
    fseek(this->image, parent_block * BLOCK_SIZE, SEEK_SET);
    fread(buffer, 1, BLOCK_SIZE, this->image);

    uint32_t offset = 0;
    while (offset < BLOCK_SIZE) {
        Directory *entry = (Directory *)(buffer + offset);
        uint32_t real_len = 8 + entry->name_len + ((4 - (entry->name_len % 4)) % 4);
        if (entry->rec_len > real_len) {
            uint16_t old_rec_len = entry->rec_len;
            entry->rec_len = real_len;

            Directory *new_entry = (Directory *)(buffer + offset + real_len);
            new_entry->inode = first_inode;
            new_entry->rec_len = old_rec_len - real_len;
            new_entry->name_len = strlen(dir_name);
            new_entry->file_type = 2; // diretório
            memset(new_entry->name, 0, EXT2_NAME_LEN);
            strncpy(new_entry->name, dir_name, EXT2_NAME_LEN);

            fseek(this->image, parent_block * BLOCK_SIZE, SEEK_SET);
            fwrite(buffer, 1, BLOCK_SIZE, this->image);

            std::cout << "mkdir: diretório '" << dir_name << "' criado." << std::endl;
            free(actual_inode);
            return;
        }
        offset += entry->rec_len;
    }

    std::cout << "mkdir: não há espaço no diretório para criar o diretório." << std::endl;
    free(actual_inode);
}

void FileSystemManager::rmdir(const char *dir_name) {
    // Obtém o diretório atual
    Directory actual_directory = this->navigation.back();
    Inode *actual_inode = read_inode(this->image, this->bgd, inode_relative_position(this->superblock, actual_directory.inode));

    // Procura a entrada do diretório a ser removido
    Directory *entry = search_directory(this->image, actual_inode, dir_name);
    if (!entry) {
        std::cout << "rmdir: diretório '" << dir_name << "' não encontrado." << std::endl;
        free(actual_inode);
        return;
    }
    if (entry->file_type != 2) {
        std::cout << "rmdir: '" << dir_name << "' não é um diretório." << std::endl;
        free(actual_inode);
        return;
    }

    // Lê o inode do diretório a ser removido
    unsigned int dir_inode_block_group = block_group_from_inode(this->superblock, entry->inode);
    BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(dir_inode_block_group));
    Inode *dir_inode = read_inode(this->image, bgd_of_inode, inode_relative_position(this->superblock, entry->inode));

    // Verifica se o diretório está vazio (apenas "." e "..")
    uint8_t dir_block[BLOCK_SIZE];
    fseek(this->image, dir_inode->i_block[0] * BLOCK_SIZE, SEEK_SET);
    fread(dir_block, 1, BLOCK_SIZE, this->image);

    uint32_t offset = 0;
    int entries = 0;
    while (offset < BLOCK_SIZE) {
        Directory *curr_entry = (Directory *)(dir_block + offset);
        if (curr_entry->inode != 0) {
            // Conta entradas válidas
            if (strncmp(curr_entry->name, ".", curr_entry->name_len) != 0 &&
                strncmp(curr_entry->name, "..", curr_entry->name_len) != 0) {
                entries++;
            }
        }
        offset += curr_entry->rec_len;
    }
    if (entries > 0) {
        std::cout << "rmdir: diretório '" << dir_name << "' não está vazio." << std::endl;
        free(dir_inode);
        free(bgd_of_inode);
        free(actual_inode);
        return;
    }

    // Remove a entrada do diretório pai (ajusta rec_len da entrada anterior)
    uint32_t parent_block = actual_inode->i_block[0];
    uint8_t buffer[BLOCK_SIZE];
    fseek(this->image, parent_block * BLOCK_SIZE, SEEK_SET);
    fread(buffer, 1, BLOCK_SIZE, this->image);

    offset = 0;
    uint32_t prev_offset = 0;
    Directory *prev_entry = nullptr;
    while (offset < BLOCK_SIZE) {
        Directory *curr_entry = (Directory *)(buffer + offset);
        if (strncmp(curr_entry->name, dir_name, curr_entry->name_len) == 0 && curr_entry->name_len == strlen(dir_name)) {
            if (prev_entry) {
                prev_entry->rec_len += curr_entry->rec_len;
            } else {
                curr_entry->inode = 0;
            }
            break;
        }
        prev_offset = offset;
        prev_entry = curr_entry;
        offset += curr_entry->rec_len;
    }

    // Escreve o bloco de diretório atualizado
    fseek(this->image, parent_block * BLOCK_SIZE, SEEK_SET);
    fwrite(buffer, 1, BLOCK_SIZE, this->image);

    // Libera o inode no bitmap de inodes
    read_bitmap(this->image, this->bgd->bg_inode_bitmap, buffer, BLOCK_SIZE);
    uint32_t inode_idx = entry->inode - 1;
    clear_bitmap_bit(buffer, inode_idx);
    write_bitmap(this->image, this->bgd->bg_inode_bitmap, buffer, BLOCK_SIZE);

    // Atualiza contadores de inodes livres
    this->superblock->s_free_inodes_count++;
    fseek(this->image, 1024, SEEK_SET);
    fwrite(this->superblock, sizeof(Superblock), 1, this->image);

    this->bgd->bg_free_inodes_count++;
    fseek(this->image, block_group_descriptor_address(0), SEEK_SET);
    fwrite(this->bgd, sizeof(BlocksGroupDescriptor), 1, this->image);

    // Libera o bloco do diretório no bitmap de blocos
    read_bitmap(this->image, this->bgd->bg_block_bitmap, buffer, BLOCK_SIZE);
    uint32_t block_idx = dir_inode->i_block[0] - 1;
    clear_bitmap_bit(buffer, block_idx);
    write_bitmap(this->image, this->bgd->bg_block_bitmap, buffer, BLOCK_SIZE);

    // Atualiza contadores de blocos livres
    this->superblock->s_free_blocks_count++;
    fseek(this->image, 1024, SEEK_SET);
    fwrite(this->superblock, sizeof(Superblock), 1, this->image);

    this->bgd->bg_free_blocks_count++;
    fseek(this->image, block_group_descriptor_address(0), SEEK_SET);
    fwrite(this->bgd, sizeof(BlocksGroupDescriptor), 1, this->image);

    std::cout << "rmdir: diretório '" << dir_name << "' removido." << std::endl;
    free(dir_inode);
    free(bgd_of_inode);
    free(actual_inode);
}