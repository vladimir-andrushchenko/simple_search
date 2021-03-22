//
//  SearchServer.hpp
//  simple_search
//
//  Created by Vladimir Andrushchenko on 22.03.2021.
//

#ifndef SearchServer_hpp
#define SearchServer_hpp

//#include <stdio.h>
#include <string>
#include <set>
#include <map>

#include "../Query.hpp"
#include "../Document.hpp"

const int MAX_RESULT_DOCUMENT_COUNT = 5;

using namespace std;

class SearchServer {
public:
    void SetStopWords(const string& text);

    void AddDocument(int document_id, const string& document);

    vector<Document> FindTopDocuments(const string& query) const;

private:
    map<string, map<int, double>> word_to_document_freqs_; // word -> map<document_id, term frequency in this document>
    set<string> stop_words_;
    int number_of_documents_ = 0;

    vector<string> SplitIntoWordsNoStop(const string& text) const;
    
    Query ParseQuery(const string& query_raw) const;
    
    vector<Document> FindAllDocuments(const string& query_raw) const;
};

SearchServer CreateSearchServer();

#endif /* SearchServer_hpp */
