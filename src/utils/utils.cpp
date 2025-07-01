#include "utils.hpp"
#include <ctime>
#include <iostream>

using namespace std;

void print_time(unsigned int timestamp) { // imprime no formato DD/MM/AAAA HH:MM 
    time_t time = timestamp;
    tm* timeinfo = localtime(&time);
    char buffer[20];
    strftime(buffer, 20, "%d/%m/%Y %H:%M", timeinfo);
    std::cout << buffer << std::endl;
}