//
//  Query.hpp
//  simple_search
//
//  Created by Vladimir Andrushchenko on 22.03.2021.
//

#ifndef Query_hpp
#define Query_hpp

#include <string>
#include <vector>

using namespace std;

struct Query {
    map<string, double> words_to_idf; // idf - indirect document frequence log(все_документы/документ_где_слово встречается) чем чаще встречается слово тем ниже его релевантность
    vector<string> minus_words;
};

#endif /* Query_h */
