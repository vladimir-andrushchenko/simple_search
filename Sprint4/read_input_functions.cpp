//
//  read_input_functions.cpp
//  Sprint4
//
//  Created by Vladimir Andrushchenko on 12.05.2021.
//

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
