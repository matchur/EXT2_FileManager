#pragma once
#include <cstdint>
#include <cstdio>

void read_bitmap(FILE* image, uint32_t block, uint8_t* buffer, size_t size);
void write_bitmap(FILE* image, uint32_t block, uint8_t* buffer, size_t size);
void set_bitmap_bit(uint8_t* buffer, uint32_t idx);
void clear_bitmap_bit(uint8_t* buffer, uint32_t idx);
bool is_bitmap_bit_set(uint8_t* buffer, uint32_t idx);