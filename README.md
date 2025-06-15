# Ext2Cat

Este programa implementa as estruturas de dados e operações para manipular a imagem (.iso) de um sistema de arquivos EXT2. As
operações são invocadas a partir de um prompt (shell). O shell executa as operações a partir da referência
do diretório corrente. O shell sempre inicia no raiz (/) da imagem informada por parâmetro.

As estruturas e a lógica de manipulação foram implementadas a partir da especificação do ext2:
https://www.nongnu.org/ext2-doc/ext2.html

Operações:

- `info`: exibe informações do disco e do sistema de arquivos.
- `cat <filename>`: exibe o conteúdo de um arquivo no formato texto.
- `attr <file | dir>`: exibe os atributos de um arquivo (*file*) ou diretório (*dir*).
- `cd <path>`: altera o diretório corrente para o definido como *path*.
- `ls`: listar os arquivos e diretórios do diretório corrente.
- `pwd`: exibe o diretório corrente (caminho absoluto).
- `touch <filename>`: cria o arquivo *filename* com conteúdo vazio.
- `mkdir <dir>`: cria o diretório *dir* vazio.
- `rm <filename>`: remove o arquivo *filename*, se existir.
- `rmdir <dir>`: remove o diretório *dir*, se existir e estiver vazio.
- `rename <filename> <newfilename>` : renomeia arquivo *filename* para *newfilename*.
- `cp <source_path> <target_path>`: copia um arquivo de origem (*source_path*) para destino (*target_path*). A origem e o destino devem ser uma partição do disco e o  volume ext2 ou vice-versa.
- `echo <filename> text`: substitui o conteúdo de um arquivo existente com o valor de *text*.
- `print [ superblock | groups | inode | rootdir | dir | inodebitmap | blockbitmap | attr | block ]`: exibe informações do sistema ext2.

Limitações:

- não processa arquivos com ponteiros triplamente indiretos.
- escreve entradas no arquivo de diretórios, limitando-se ao tamanho do bloco.
- realiza a leitura de arquivos de diretório que usa apenas ponteiros diretos.

Para compilar:
```console
# make
```

Para executar:
```console
# ./ext2cat <ext2-image-file>
```


## Criação e geração da imagem de volume ext2

Gerando imagens ext2 (64MiB com blocos de 1K):
```console
# dd if=/dev/zero of=./myext2image.img bs=1024 count=64K
# mkfs.ext2 -b 1024 ./myext2image.img
```

Verificando a integridade de um sistema ext2:
```console
# e2fsck myext2image.img
```

Montando a imagem do volume com ext2:
```console
# sudo mount myext2image.img /mnt
```

Estrutura original de arquivos do volume (comando `tree` via bash):
```
/
├── [1.0K]  documentos
│   ├── [1.0K]  emptydir
│   ├── [9.2K]  alfabeto.txt
│   └── [   0]  vazio.txt
├── [1.0K]  imagens
│   ├── [8.1M]  one_piece.jpg
│   ├── [391K]  saber.jpg
│   └── [ 11M]  toscana_puzzle.jpg
├── [1.0K]  livros
│   ├── [1.0K]  classicos
│   │   ├── [506K]  A Journey to the Centre of the Earth - Jules Verne.txt
│   │   ├── [409K]  Dom Casmurro - Machado de Assis.txt
│   │   ├── [861K]  Dracula-Bram_Stoker.txt
│   │   ├── [455K]  Frankenstein-Mary_Shelley.txt
│   │   └── [232K]  The Worderful Wizard of Oz - L. Frank Baum.txt
│   └── [1.0K]  religiosos
│       └── [3.9M]  Biblia.txt
├── [ 12K]  lost+found
└── [  29]  hello.txt

```

Informações de espaço  (comando `df` via bash):
```
Blocos de 1k: 62186
Usado: 26777 KiB
Disponível: 32133 KiB
```

Desmontando a imagem do volume com ext2:
```console
# sudo umount /mnt
```


## Exemplos

Os exemplos são executados na imagem `myext2image.img` 

Informações do volume e do ext2 (comando `info`):
```console
[/]$> info
Volume name.....: SO-UTFPR-1k
Image size......: 67108864 bytes
Free space......: 32133 KiB
Free inodes.....: 16355
Free blocks.....: 35409
Block size......: 1024 bytes
Inode size......: 128 bytes
Groups count....: 8
Groups size.....: 8192 blocks
Groups inodes...: 2048 inodes
Inodetable size.: 256 blocks
```

Exibição do superbloco (comando `print superblock`):
```console
[/]$> print superblock
inodes count: 16384
blocks count: 65536
reserved blocks count: 3276
free blocks count: 35409
free inodes count: 16355
first data block: 1
block size: 1024
fragment size: 1024
blocks per group: 8192
fragments per group: 8192
inodes per group: 2048
mount time: 1668913172
write time: 1668913483
mount count: 2
max mount count: 65535
magic signature: 0xef53
file system state: 1
errors: 1
minor revision level: 0
time of last check: 19/11/2022 23:34
max check interval: 0
creator OS: 0
revision level: 1
default uid reserved blocks: 0
defautl gid reserved blocks: 0
first non-reserved inode: 11
inode size: 128
block group number: 0
compatible feature set: 56
incompatible feature set: 2
read only comp feature set: 3
volume UUID: 184054db7e714a6a9689bbfc5e74c187
volume name: SO-UTFPR-1k
volume last mounted: /home/rodrigo/temp/mnt
algorithm usage bitmap: 0
blocks to try to preallocate: 0
blocks preallocate dir: 0
journal UUID:                 
journal INum: 0
journal Dev: 0
last orphan: 0
hash seed: d877f07405b44118bdbea37b6e06c1ad
default hash version: 1
default mount options: 12
first meta: 0
```

Exibição dos grupos de blocos (comando `print groups`):
```console
[/]$> print groups

Block Group Descriptor 0: 
block bitmap: 258
inode bitmap: 259
inode table: 260
free blocks count: 4897
free inodes count: 2036
used dirs count: 2

Block Group Descriptor 1: 
block bitmap: 8450
inode bitmap: 8451
inode table: 8452
free blocks count: 2964
free inodes count: 2048
used dirs count: 0

Block Group Descriptor 2: 
block bitmap: 16385
inode bitmap: 16386
inode table: 16387
free blocks count: 765
free inodes count: 2044
used dirs count: 1

Block Group Descriptor 3: 
block bitmap: 24834
inode bitmap: 24835
inode table: 24836
free blocks count: 6239
free inodes count: 2048
used dirs count: 0

Block Group Descriptor 4: 
block bitmap: 32769
inode bitmap: 32770
inode table: 32771
free blocks count: 2683
free inodes count: 2039
used dirs count: 3

Block Group Descriptor 5: 
block bitmap: 41218
inode bitmap: 41219
inode table: 41220
free blocks count: 2253
free inodes count: 2048
used dirs count: 0

Block Group Descriptor 6: 
block bitmap: 49153
inode bitmap: 49154
inode table: 49155
free blocks count: 7932
free inodes count: 2044
used dirs count: 2

Block Group Descriptor 7: 
block bitmap: 57602
inode bitmap: 57603
inode table: 57604
free blocks count: 7676
free inodes count: 2048
used dirs count: 0
```



Listagem do diretório raiz (comando `ls`):
```console
[/]$> ls
.
inode: 2
record lenght: 12
name lenght: 1
file type: 2

..
inode: 2
record lenght: 12
name lenght: 2
file type: 2

lost+found
inode: 11
record lenght: 20
name lenght: 10
file type: 2

documentos
inode: 12289
record lenght: 20
name lenght: 10
file type: 2

livros
inode: 8193
record lenght: 16
name lenght: 6
file type: 2

imagens
inode: 4097
record lenght: 16
name lenght: 7
file type: 2

hello.txt
inode: 12
record lenght: 928
name lenght: 9
file type: 1
```

Exibição de inode (comando `print inode`):
```console
[/]$> print inode 2
file format and access rights: 0x41ed
user id: 0
lower 32-bit file size: 1024
access time: 1668911918
creation time: 1668911917
modification time: 1668911917
deletion time: 0
group id: 0
link count inode: 6
512-bytes blocks: 2
ext2 flags: 0
reserved (Linux): 4
pointer[0]: 516
pointer[1]: 0
pointer[2]: 0
pointer[3]: 0
pointer[4]: 0
pointer[5]: 0
pointer[6]: 0
pointer[7]: 0
pointer[8]: 0
pointer[9]: 0
pointer[10]: 0
pointer[11]: 0
pointer[12]: 0
pointer[13]: 0
pointer[14]: 0
file version (nfs): 0
block number extended attributes: 0
higher 32-bit file size: 0
location file fragment: 0
```

Navegação e atributos (comandos `cd` e `attr`)
```console
[/]$> cd livros
livros
inode: 8193
record lenght: 16
name lenght: 6
file type: 2
[/livros/]$> cd classicos
classicos
inode: 8194
record lenght: 20
name lenght: 9
file type: 2
[/livros/classicos/]$> attr Dracula-Bram_Stoker.txt
permissões  uid  gid      tamanho      modificado em
frw-r--r--    0    0     860.6 KiB    19/11/2022 23:41
[/livros/classicos/]$> cd ..
..
inode: 8193
record lenght: 12
name lenght: 2
file type: 2
[/livros/]$> 
```

Exibição de arquivos no formato texto (comando `cat`)
```console
[/]$> cat hello.txt
Hello Sistemas Operacionais.
```

Tratamento de erros básicos
```console
[/]$> cd docs
[error: 2] directory not found.
[/]$> cat hello.pdf
[error: 1] file not found.
[/]$> dog hello.txt
command not found.
[/]$> cat
invalid sintax.
[/]$> touch imagens
[error: 4] file already exists.
[/]$> rmdir imagens
[error: 5] directory not empty.
[/]$> rmdir hello.txt
[error: 1] file not found.
[/imagens/]$> cp one_piece.jpg /dir_inexistente
[error: 3] destination directory not exists.
```

---

### Notas

Desenvolvido por Rodrigo Campiolo (@rcampiolo) em novembro/2022.


