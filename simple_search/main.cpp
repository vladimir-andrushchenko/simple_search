#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;


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
    double relevance;
};

struct Query {
    map<string, double> words_to_idf; // idf - indirect document frequence log(все_документы/документ_где_слово встречается) чем чаще встречается слово тем ниже его релевантность
    vector<string> minus_words;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
            for (const string& word : SplitIntoWords(text)) {
                    stop_words_.insert(word);
            }
    }

    void AddDocument(int document_id, const string& document) {
        vector<string> words_in_document = SplitIntoWordsNoStop(document);
        for (const string& word : words_in_document) {
            const double word_appears_n_times = count(words_in_document.begin(), words_in_document.end(), word);
            const double tf = word_appears_n_times / words_in_document.size();
            word_to_document_freqs_[word][document_id] = tf; // стоп слова при подсчете .size() не рассчитываются
        }
        ++number_of_documents_;
    }
// TODO: сюда добавить функцию по которой будет происходить сортировка
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
    map<string, map<int, double>> word_to_document_freqs_; // word -> map<document_id, term frequency in this document>
    set<string> stop_words_;
    int number_of_documents_ = 0;

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
                const string stop_word_without_minus_sign = word.substr(1);
                if (stop_words_.count(stop_word_without_minus_sign) == 1) {
                    continue;
                }
                query.minus_words.push_back(stop_word_without_minus_sign);
            } else {
                if (stop_words_.count(word) == 1) {
                    continue;
                }
                if (word_to_document_freqs_.count(word) == 1) { // смотрю если слово вообще встречается в каком либо документе
                    const double in_how_many_docs_this_word_appears = word_to_document_freqs_.at(word).size();
                    const double ratio_of_all_docs_to_this_word = number_of_documents_ / in_how_many_docs_this_word_appears;
                    const double idf_of_word = log(ratio_of_all_docs_to_this_word);
                    query.words_to_idf[word] = idf_of_word;
                }
                // если слово из запроса не встречается ни в одном документе оно не добавляется в структуру Query
            }
        }
        return query;
    }
    

    vector<Document> FindAllDocuments(const string& query_raw) const {
        // сначала считаю релевантность
        const Query query = ParseQuery(query_raw);
        map<int, double> document_to_relevance;
        for (const auto& [word, idf] : query.words_to_idf) { // цикл по словам запроса
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, tf_of_word_in_this_doc]: word_to_document_freqs_.at(word)) { // цикл по документам где это слово встречается
                document_to_relevance[document_id] += idf * tf_of_word_in_this_doc;
            }
        }
        
        // потом удаляю документы в которых встречается минус слово
        for (const string& minus_word : query.minus_words) {
            if (word_to_document_freqs_.count(minus_word) == 0) {
                    continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(minus_word)) {
                document_to_relevance.erase(document_id);
            }
        }
        
        // потом всё вставляю в структурки и отправляю на выход
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

/*
и в на
3
белый кот и модный ошейник
пушистый кот пушистый хвост
ухоженный пёс выразительные глаза
пушистый ухоженный кот
 */
