#include "bitmap-utils.hpp"
#define BLOCK_SIZE 1024

void read_bitmap(FILE* image, uint32_t block, uint8_t* buffer, size_t size) {
    fseek(image, block * BLOCK_SIZE, SEEK_SET);
    fread(buffer, 1, size, image);
}

void write_bitmap(FILE* image, uint32_t block, uint8_t* buffer, size_t size) {
    fseek(image, block * BLOCK_SIZE, SEEK_SET);
    fwrite(buffer, 1, size, image);
}

void set_bitmap_bit(uint8_t* buffer, uint32_t idx) {
    buffer[idx / 8] |= (1 << (idx % 8));
}

void clear_bitmap_bit(uint8_t* buffer, uint32_t idx) {
    buffer[idx / 8] &= ~(1 << (idx % 8));
}

bool is_bitmap_bit_set(uint8_t* buffer, uint32_t idx) {
    return buffer[idx / 8] & (1 << (idx % 8));
}