all: compile image-restore

compile:
	g++ ./src/main/main.cpp ./src/main/shell.cpp ./src/file/file-operations.cpp ./src/manager/file-system-manager.cpp ./src/superblock/superblock.cpp ./src/directory/directory.cpp ./src/blocks-group-descriptor/blocks-group-descriptor.cpp ./src/inode/inode.cpp -o ext2

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
