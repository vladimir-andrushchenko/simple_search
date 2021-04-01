#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

string ReadLine() {
    string input;
    getline(cin, input);
    return input;
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
    for (const char character : text) {
        if (character == ' ') {
            words.push_back(word);
            word = "";
        } else {
            word += character;
        }
    }
    words.push_back(word);
    
    return words;
}
    
struct Document {
    int id = -1; // нумерация начинается с нуля. нетипичное значение для инициализации.
    double relevance = 0;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }
    
    void AddDocument(int document_id, const string& document,
                     DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        
        const double inverse_word_count = 1.0 / words.size();
        
        for (const string& word : words) {
            word_to_document_id_to_term_frequencies_[word][document_id] += inverse_word_count;
        }
        
        document_id_to_document_data_.emplace(document_id,
            DocumentData{
                ComputeAverageRating(ratings),
                status
            });
    }
    
    int GetDocumentCount() const {
        return static_cast<int>(document_id_to_document_data_.size());
    }
    
    template<typename Predicate>
    vector<Document> FindTopDocuments(const string& raw_query, Predicate predicate) const {
        return GetTopFilteredDocuments(raw_query, predicate);
    }
    
    vector<Document> FindTopDocuments(const string& raw_query, const DocumentStatus& desiredStatus = DocumentStatus::ACTUAL) const {
        const auto predicate = [desiredStatus](int _, DocumentStatus documentsStatus, int __) {
            return documentsStatus == desiredStatus;
        };
        return GetTopFilteredDocuments(raw_query, predicate);
    }
    
    template<typename Predicate>
    vector<Document> GetTopFilteredDocuments(const string& raw_query, Predicate predicate) const {
        const Query query = ParseQuery(raw_query);
        
        vector<Document> matched_documents = FindAllDocuments(query);
        
        vector<Document> filtered_documents;
        for (const Document& document : matched_documents) {
            if(predicate(document.id, document_id_to_document_data_.at(document.id).status, document_id_to_document_data_.at(document.id).rating)) {
                filtered_documents.push_back(document);
            }
        }
        
        sort(filtered_documents.begin(), filtered_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                } else {
                    return lhs.relevance > rhs.relevance;
                }
             });
        
        if (filtered_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            filtered_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return filtered_documents;
    } // GetTopFilteredDocuments
    
    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_id_to_term_frequencies_.count(word) == 0) {
                continue;
            }
            
            if (word_to_document_id_to_term_frequencies_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        
        for (const string& word : query.minus_words) {
            if (word_to_document_id_to_term_frequencies_.count(word) == 0) {
                continue;
            }
            
            if (word_to_document_id_to_term_frequencies_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        
        return {matched_words, document_id_to_document_data_.at(document_id).status};
    }
    
private:
    static constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;
    
    struct DocumentData {
        int rating = 0;
        DocumentStatus status = DocumentStatus::ACTUAL;
    };

    set<string> stop_words_;
    
    map<string, map<int, double>> word_to_document_id_to_term_frequencies_;
    
    map<int, DocumentData> document_id_to_document_data_;
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }
    
    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        
        return words;
    }
    
    static int ComputeAverageRating(const vector<int>& ratings) {
        int rating_sum = 0;
        
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        
        return rating_sum / static_cast<int>(ratings.size());
    }
    
    struct QueryWord {
        string data = ""s;
        bool is_minus = false;
        bool is_stop = false;
    };
    
    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        
        return {
            text,
            is_minus,
            IsStopWord(text)
        };
    }
    
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    
    Query ParseQuery(const string& text) const {
        Query query;
        
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        
        return query;
    }
    
    // Existence required
    double ComputeWordInverseDocumentFrequency(const string& word) const {
        const double numberOfAllDocuments = GetDocumentCount();
        const double numberOfDocumentsWhereGivenWordAppears = word_to_document_id_to_term_frequencies_.at(word).size();
        return log(numberOfAllDocuments / numberOfDocumentsWhereGivenWordAppears);
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> document_id_to_relevance;
        
        for (const string& word : query.plus_words) {
            if (word_to_document_id_to_term_frequencies_.count(word) == 0) {
                continue;
            }
            
            const double inverse_document_frequency = ComputeWordInverseDocumentFrequency(word);
            
            for (const auto &[document_id, term_frequency] : word_to_document_id_to_term_frequencies_.at(word)) {
                document_id_to_relevance[document_id] += term_frequency * inverse_document_frequency;
            }
        }
        
        for (const string& word : query.minus_words) {
            if (word_to_document_id_to_term_frequencies_.count(word) == 0) {
                continue;
            }
            
            for (const auto &[document_id, _] : word_to_document_id_to_term_frequencies_.at(word)) {
                document_id_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto &[document_id, relevance] : document_id_to_relevance) {
            matched_documents.push_back({
                document_id,
                relevance,
                document_id_to_document_data_.at(document_id).rating
            });
        }
        
        return matched_documents;
    }
};

// ==================== для примера =========================

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}

int main() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});

    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }

    cout << "ACTUAL:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; })) {
        PrintDocument(document);
    }
    
    cout << "BANNED:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
            PrintDocument(document);
        }

    cout << "Even ids:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }

    return 0;
}

/*
 ACTUAL by default:
 { document_id = 1, relevance = 0.866434, rating = 5 }
 { document_id = 0, relevance = 0.173287, rating = 2 }
 { document_id = 2, relevance = 0.173287, rating = -1 }
 ACTUAL:
 { document_id = 1, relevance = 0.866434, rating = 5 }
 { document_id = 0, relevance = 0.173287, rating = 2 }
 { document_id = 2, relevance = 0.173287, rating = -1 }
 Even ids:
 { document_id = 0, relevance = 0.173287, rating = 2 }
 { document_id = 2, relevance = 0.173287, rating = -1 }
 */

/*
 ACTUAL by default:
 { document_id = 1, relevance = 0.866434, rating = 5 }
 { document_id = 0, relevance = 0.173287, rating = 2 }
 { document_id = 2, relevance = 0.173287, rating = -1 }
 ACTUAL:
 { document_id = 1, relevance = 0.866434, rating = 5 }
 { document_id = 0, relevance = 0.173287, rating = 2 }
 { document_id = 2, relevance = 0.173287, rating = -1 }
 BANNED:
 { document_id = 3, relevance = 0.231049, rating = 9 }
 Even ids:
 { document_id = 0, relevance = 0.173287, rating = 2 }
 { document_id = 2, relevance = 0.173287, rating = -1 }
 */
