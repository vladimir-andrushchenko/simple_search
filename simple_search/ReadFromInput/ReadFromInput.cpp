//
//  ReadFromInput.cpp
//  simple_search
//
//  Created by Vladimir Andrushchenko on 22.03.2021.
//

#include "ReadFromInput.hpp"

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}
