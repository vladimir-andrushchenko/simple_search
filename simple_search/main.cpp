#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

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

set<string> ParseStopWords(const string& text) {
    set<string> stop_words;
    for (const string& word : SplitIntoWords(text)) {
        stop_words.insert(word);
    }
    return stop_words;
}

vector<string> SplitIntoWordsNoStop(const string& text, const set<string>& stop_words) {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        if (stop_words.count(word) == 0) {
            words.push_back(word);
        }
    }
    return words;
}

void AddDocument(map<string, set<int>>& word_to_documents,
                 const set<string>& stop_words,
                 int document_id,
                 const string& document) {
    for (const string& word : SplitIntoWordsNoStop(document, stop_words)) {
        word_to_documents[word].insert(document_id);
    }
}

bool ComparePairBySecondThenFirst(pair<int, int> a, pair<int, int> b) {
    if (a.second > b.second) {
        return true;
    } else if (a.second == b.second) {
        if (a.first > b.first) {
            return true;
        }
    }
    return false;
}

// For each document returns its id and relevance
vector<pair<int, int>> FindAllDocuments(
        const map<string, set<int>>& word_to_documents,
        const set<string>& stop_words,
        const string& query) {
    
    const vector<string> query_words = SplitIntoWordsNoStop(query, stop_words);
    map<int, int> document_to_relevance;
    
    for (const string& word : query_words) {
        // if a word doesn't appear in any document => continue
        if (word_to_documents.count(word) == 0) {
            continue;
        }
        // relevance counted by how many words from the query appear in a document
        for (const int document_id : word_to_documents.at(word)) {
            ++document_to_relevance[document_id];
        }
    }

    vector<pair<int, int>> found_documents;
    for (auto [document_id, relevance] : document_to_relevance) {
        found_documents.push_back({document_id, relevance});
    }
    sort(found_documents.begin(), found_documents.end()
         , ComparePairBySecondThenFirst
         );
    
    return found_documents;
}



vector<pair<int, int>>  FindTopDocuments( const map<string, set<int>>& word_to_documents, const set<string>& stop_words, const string& query) {
    vector<pair<int, int>> documents = FindAllDocuments(word_to_documents, stop_words, query);
    // если размер меньше ограничения то возвращаю без изменений
    if (documents.size() <= MAX_RESULT_DOCUMENT_COUNT) {
        return documents;
    }
    // если размер больше то обрезаю
    vector<pair<int, int>> topDocuments(documents.begin(), documents.begin()+MAX_RESULT_DOCUMENT_COUNT);
    return topDocuments;
}

int main() {
    const string stop_words_joined = ReadLine();
    const set<string> stop_words = ParseStopWords(stop_words_joined);

    // Read documents
    map<string, set<int>> word_to_documents;
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        AddDocument(word_to_documents, stop_words, document_id, ReadLine());
    }

    const string query = ReadLine();
    // FindTopDocuments goes here
    vector<pair<int, int>> top_documents = FindTopDocuments(word_to_documents, stop_words, query);
    
    for (auto [document_id, relevance] : FindTopDocuments(word_to_documents, stop_words, query)) {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s << endl;
    }
}

// что делать если документы пустые

/*
 a the on cat
 6
 a fat cat sat on a mat and ate a fat rat
 fat rat
 a fat cat rat
 a fat cat sat
 a fat cat
 a fat dog
 funny fat cat on a mat with rat
 */

/*
 
 2
 hi bye
 hi
 hi buy
 */
