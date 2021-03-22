//
//  SplitIntoWords.cpp
//  simple_search
//
//  Created by Vladimir Andrushchenko on 22.03.2021.
//

#include "SplitIntoWords.hpp"
#include <string>
#include <vector>

using namespace std;

vector<string> SplitIntoWords(const string& text) {
        vector<string> words;
        string word;
        for (const char c : text) {
                if (c == ' ') {
                        words.push_back(word);
                        word = "";
                } else {
                        word += c;
                }
        }
        words.push_back(word);
        
        return words;
}
