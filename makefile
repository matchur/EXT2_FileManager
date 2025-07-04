all: compile image-restore

compile:
	g++ ./src/main/main.cpp ./src/main/shell.cpp ./src/manager/file-system-manager.cpp ./src/superblock/superblock.cpp ./src/blocks-group-descriptor/blocks-group-descriptor.cpp ./src/inode/inode.cpp ./src/directory/directory.cpp ./src/utils/utils.cpp ./src/file/file-operations.cpp ./src/utils/bitmap-utils.cpp -o ext2

run:
	./ext2

image-restore:
	rm -rf ./EXT2_TESTE.img
	cp ./imagem/EXT2_TESTE.img ./

clear:
	rm -rf *.o
	rm -rf *.exe

mount:
	sudo mount EXT2_TESTE.img /mnt
	ls /mnt

umount:
	sudo umount /mnt
