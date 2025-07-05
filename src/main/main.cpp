#include "shell.hpp"
#include "../utils/utils.hpp"
#include "../error/error.hpp"
#include <limits>

using std::cout;
using std::endl;

int main()
{
    FILE *ext2_image = NULL;
    FileSystemManager *fsm;

    // Abre automaticamente a imagem EXT2_TESTE.img
    ext2_image = get_file("EXT2_TESTE.img");
    if (!ext2_image) {
        cout << "Não foi possível abrir EXT2_TESTE.img" << endl;
        return 1;
    }
    fsm = new FileSystemManager(ext2_image);
    shell(fsm);

    return 0;
}