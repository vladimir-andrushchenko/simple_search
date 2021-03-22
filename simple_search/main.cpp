#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>



#include "SearchServer.hpp"
#include "ReadFromInput.hpp"


int main() {
    const SearchServer search_server = CreateSearchServer();
    
    const string query = ReadLine();
    for (auto [document_id, relevance] : search_server.FindTopDocuments(query)) {
            cout << "{ document_id = " << document_id << ", relevance = " << relevance << " }" << endl;
    }
}

/*
и в на
3
белый кот и модный ошейник
пушистый кот пушистый хвост
ухоженный пёс выразительные глаза
пушистый ухоженный кот
 */
