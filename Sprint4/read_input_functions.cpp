#include "read_input_functions.hpp"

#include <iostream>

std::string ReadLine() {
    std::string line;
    getline(std::cin, line);
    return line;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}
