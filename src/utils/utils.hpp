#pragma once

#include <cstdint>
#include <cstdio>

void print_time(unsigned int timestamp);
int get_block_offset(uint32_t block, int base_offset, int block_size);
FILE *get_file(const char *ptr);