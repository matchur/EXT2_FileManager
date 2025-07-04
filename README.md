# EXT2_FileManager

EXT2_FileManager é uma ferramenta de linha de comando para manipulação direta de imagens de sistemas de arquivos EXT2 (`.img` ou `.iso`). O projeto permite explorar, modificar e gerenciar arquivos e diretórios dentro da imagem, simulando operações típicas de um shell Linux, mas atuando diretamente sobre a estrutura EXT2.

## Operações Disponíveis

O shell do EXT2_FileManager suporta os seguintes comandos:

- `info`  
  Exibe informações gerais do disco e do sistema de arquivos EXT2.

- `cat <filename>`  
  Exibe o conteúdo de um arquivo de texto.

- `attr <file|dir>`  
  Mostra os atributos (permissões, UID, GID, tamanho, data de modificação) de um arquivo ou diretório.

- `cd <path>`  
  Altera o diretório corrente para o caminho especificado.  
  Use `cd ..` para voltar ao diretório pai e `cd .` para permanecer no diretório atual.

- `ls`  
  Lista arquivos e diretórios do diretório corrente, exibindo informações detalhadas de cada entrada.

- `pwd`  
  Mostra o caminho absoluto do diretório corrente.

- `print [superblock|groups|group <número>|inode <número>]`  
  Exibe informações detalhadas sobre as estruturas internas do EXT2.

- `touch <filename>`  
  Cria um novo arquivo vazio no diretório atual.

- `rm <filename>`  
  Remove um arquivo do diretório atual.

- `rename <oldname> <newname>`  
  Renomeia um arquivo ou diretório no diretório atual.

- `mkdir <dirname>`  
  Cria um novo diretório no diretório atual.

- `rmdir <dirname>`  
  Remove um diretório vazio do diretório atual.

- `cp <arquivo_origem> <destino>`  
  Copia um arquivo da imagem EXT2 para o sistema de arquivos do host.

- `exit`  
  Encerra o shell do EXT2_FileManager.

## Compilação e Execução

Para compilar:
```sh
make
```

Para executar:
```sh
./ext2
```

## Referências

* https://www.science.smith.edu/~nhowe/262/oldlabs/kernel/ext2_fs.h
* https://www.nongnu.org/ext2-doc/ext2.html
* https://docs.kernel.org/filesystems/ext2.html
* http://web.mit.edu/tytso/www/linux/ext2intro.html

## Autores

* Rafael Roseira Machado
* Matheus Vinicius da Costa
* Christopher Eduardo Zai