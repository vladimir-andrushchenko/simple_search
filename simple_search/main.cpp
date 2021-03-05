#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

typedef vector<string> vstr;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

// Reading from stdio ====================
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

// Splitting words =======================
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

// ========================================

struct Document {
    int id;
    int relevance;
};

struct Query {
    vstr words;
    vstr minus_words;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
            for (const string& word : SplitIntoWords(text)) {
                    stop_words_.insert(word);
            }
    }

    void AddDocument(int document_id, const string& document) {
            for (const string& word : SplitIntoWordsNoStop(document)) {
                    word_to_documents_[word].insert(document_id);
            }
    }

    vector<Document> FindTopDocuments(const string& query) const {
            auto matched_documents = FindAllDocuments(query);
            
            sort(
                    matched_documents.begin(),
                    matched_documents.end(),
                    [](const Document& lhs, const Document& rhs) {
                            return lhs.relevance > rhs.relevance;
                    }
            );
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return matched_documents;
    }

private:
    map<string, set<int>> word_to_documents_;
    set<string> stop_words_;

    vector<string> SplitIntoWordsNoStop(const string& text) const {
            vector<string> words;
            for (const string& word : SplitIntoWords(text)) {
                    if (stop_words_.count(word) == 0) {
                            words.push_back(word);
                    }
            }
            return words;
    }
    
    Query ParseQuery(const string& query_raw) const {
        Query query;
        for (const string& word : SplitIntoWords(query_raw)) {
            if (word[0] == '-'){
                string word_without_minus_sign = word.substr(1);
                if (stop_words_.count(word_without_minus_sign) == 1) {
                    continue;
                }
                query.minus_words.push_back(word_without_minus_sign);
            } else {
                if (stop_words_.count(word) == 1) {
                    continue;
                }
                query.words.push_back(word);
            }
        }
        return query;
    }
    

    vector<Document> FindAllDocuments(const string& query_raw) const {
        const Query query = ParseQuery(query_raw);
        map<int, int> document_to_relevance;
        
        for (const string& word : query.words) {
            if (word_to_documents_.count(word) == 0) {
                continue;
            }
            for (const int document_id : word_to_documents_.at(word)) {
                ++document_to_relevance[document_id];
            }
        }
        
        for (const string& minus_word : query.minus_words) {
            if (word_to_documents_.count(minus_word) == 0) {
                    continue;
            }
            for (const int document_id : word_to_documents_.at(minus_word)) {
                document_to_relevance.erase(document_id);
            }
        }
        vector<Document> matched_documents;
        for (auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({document_id, relevance});
        }
    
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
            search_server.AddDocument(document_id, ReadLine());
    }
    
    return search_server;
}


int main() {
    const SearchServer search_server = CreateSearchServer();
    
    const string query = ReadLine();
    for (auto [document_id, relevance] : search_server.FindTopDocuments(query)) {
            cout << "{ document_id = " << document_id << ", relevance = " << relevance << " }" << endl;
    }
}
