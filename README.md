# EXT2_FileManager

EXT2_FileManager é uma ferramenta de linha de comando para manipulação direta de imagens de sistemas de arquivos EXT2 (`.img` ou `.iso`). O projeto permite explorar, modificar e gerenciar arquivos e diretórios dentro da imagem, simulando operações típicas de um shell Linux, mas atuando diretamente sobre a estrutura EXT2.

## Objetivo

O objetivo do EXT2_FileManager é fornecer uma interface prática para estudo, análise e manipulação de sistemas de arquivos EXT2, permitindo compreender como as operações de alto nível (como criar, remover ou listar arquivos) afetam as estruturas internas do sistema de arquivos.

## Estrutura de Arquivos EXT2

O EXT2 (Second Extended File System) é um sistema de arquivos tradicional do Linux, conhecido por sua simplicidade e eficiência. Ele organiza os dados em blocos e utiliza inodes para armazenar metadados dos arquivos.

Principais componentes manipulados pelo EXT2_FileManager:

- **Superbloco:** Armazena informações globais do sistema de arquivos, como número total de blocos, inodes, tamanho dos blocos, etc.
- **Group Descriptor Table:** Divide o sistema de arquivos em grupos de blocos, facilitando a alocação e gerenciamento.
- **Bitmap de blocos e inodes:** Controlam quais blocos e inodes estão livres ou ocupados.
- **Tabela de inodes:** Cada arquivo ou diretório possui um inode, que armazena permissões, tamanho, ponteiros para blocos de dados, timestamps, entre outros.
- **Blocos de dados:** Onde o conteúdo real dos arquivos e diretórios é armazenado.
- **Diretórios:** São arquivos especiais que contêm entradas apontando para outros arquivos ou diretórios, associando nomes a inodes.

O EXT2_FileManager interage diretamente com essas estruturas, permitindo visualizar e modificar o sistema de arquivos em baixo nível.

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

- `ls`  
  Lista arquivos e diretórios do diretório corrente, exibindo informações básicas de cada entrada.

- `pwd`  
  Mostra o caminho absoluto do diretório corrente.

- `touch <filename>`  
  Cria um novo arquivo vazio.

- `mkdir <dir>`  
  Cria um novo diretório vazio.

- `rm <filename>`  
  Remove um arquivo existente.

- `rmdir <dir>`  
  Remove um diretório vazio.

- `rename <filename> <newfilename>`  
  Renomeia um arquivo ou diretório.

- `cp <source_path> <target_path>`  
  Copia um arquivo entre o sistema de arquivos EXT2 e o sistema de arquivos do host (e vice-versa).

- `echo <filename> text`  
  Substitui o conteúdo de um arquivo existente pelo texto informado.

- `print [superblock|groups|inode|rootdir|dir|inodebitmap|blockbitmap|attr|block]`  
  Exibe informações detalhadas sobre as estruturas internas do EXT2.

## Limitações

- Não suporta arquivos que utilizam ponteiros triplamente indiretos.
- Escrita em diretórios limitada ao tamanho do bloco.
- Leitura de diretórios apenas com ponteiros diretos.
- Não implementa journaling (EXT2 não possui).
- Operações de cópia são restritas a arquivos (não diretórios).

## Compilação e Execução

Para compilar:
```sh
make
```

Para executar:
```sh
./ext2cat <imagem-ext2>
```

## Criação de Imagem EXT2

Para criar uma imagem EXT2 para testes:
```sh
dd if=/dev/zero of=./myext2image.img bs=1024 count=64K
mkfs.ext2 -b 1024 ./myext2image.img
```

## Por que estudar a estrutura de arquivos EXT2?

Compreender a estrutura de arquivos EXT2 é fundamental para quem deseja aprofundar conhecimentos em sistemas operacionais, recuperação de dados, análise forense e desenvolvimento de ferramentas de baixo nível. Manipular diretamente as estruturas internas (superbloco, inodes, bitmaps) permite visualizar como o sistema operacional organiza, localiza e protege os dados, além de facilitar a identificação de problemas e otimizações.

## Autores

- Matheus Vinicius da Costa [@matchur](https://github.com/matchur)
- Rafael Roseira [@rafaroseira](https://github.com/rafaroseira)
- Christopher Zai [@Chris-cez](https://github.com/Chris-cez)


