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
    string line;
    getline(cin, line);
    return line;
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
    kActual,
    kIrrelevant,
    kBanned,
    kRemoved,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    } // SetStopWords
    
    void AddDocument(int document_id, const string& document,
                     DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        
        // inverse в смыслe (words.size() ^ -1) * words.size() == 1
        const double inverse_word_count = 1.0 / words.size();
        
        for (const string& word : words) {
            word_to_document_id_to_term_frequency_[word][document_id] += inverse_word_count;
        }
        
        document_id_to_document_data_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    } // AddDocument
    
    int GetDocumentCount() const {
        return static_cast<int>(document_id_to_document_data_.size());
    } // GetDocumentCount
    
    template<typename Predicate>
    vector<Document> FindTopDocuments(const string& raw_query, Predicate predicate) const {
        const Query query = ParseQuery(raw_query);
        
        vector<Document> matched_documents = FindAllDocuments(query);
        
        vector<Document> filtered_documents;
        for (const Document& document : matched_documents) {
            const auto document_status = document_id_to_document_data_.at(document.id).status;
            const auto document_rating = document_id_to_document_data_.at(document.id).rating;
            if(predicate(document.id, document_status, document_rating)) {
                filtered_documents.push_back(document);
            }
        }
        
        sort(filtered_documents.begin(), filtered_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < kMinSignificantDifferenceInRelevance) {
                    return lhs.rating > rhs.rating;
                } else {
                    return lhs.relevance > rhs.relevance;
                }
             });
        
        if (static_cast<int>(filtered_documents.size()) > kLimitForHowManyTopDocumentsToReturn) {
            filtered_documents.resize(static_cast<size_t>(kLimitForHowManyTopDocumentsToReturn));
        }
        
        return filtered_documents;
    }
    
    vector<Document> FindTopDocuments(const string& raw_query,
                                      const DocumentStatus& desired_status = DocumentStatus::kActual) const {
        const auto predicate = [desired_status]([[maybe_unused]] int document_id,
                                               DocumentStatus document_status,
                                               [[maybe_unused]] int rating) {
            return document_status == desired_status;
        };
        return FindTopDocuments(raw_query, predicate);
    }
    
    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_id_to_term_frequency_.count(word) == 0) {
                continue;
            }
            
            if (word_to_document_id_to_term_frequency_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        
        for (const string& word : query.minus_words) {
            if (word_to_document_id_to_term_frequency_.count(word) == 0) {
                continue;
            }
            
            if (word_to_document_id_to_term_frequency_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        
        return {matched_words, document_id_to_document_data_.at(document_id).status};
    } // MatchDocument
    
private:
    struct DocumentData {
        int rating = 0;
        DocumentStatus status = DocumentStatus::kActual;
    };
    
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    
    struct QueryWord {
        string data = ""s;
        bool is_minus = false;
        bool is_stop = false;
    };
    
private:
    static constexpr int kLimitForHowManyTopDocumentsToReturn = 5;
    static constexpr double kMinSignificantDifferenceInRelevance = 1e-6;
    
private:
    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        
        return words;
    } // SplitIntoWordsNoStop
    
    static int ComputeAverageRating(const vector<int>& ratings) {
        int rating_sum = 0;
        
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        
        return rating_sum / static_cast<int>(ratings.size());
    } // ComputeAverageRating
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    } // IsStopWord
    
    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        
        return {text, is_minus, IsStopWord(text)};
    } // ParseQueryWord
    
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
    } // ParseQuery
    
    // Existence required
    double ComputeWordInverseDocumentFrequency(const string& word) const {
        const double number_of_documents_constains_word = word_to_document_id_to_term_frequency_.at(word).size();
        return log(static_cast<double>(GetDocumentCount()) / number_of_documents_constains_word);
    } // ComputeWordInverseDocumentFrequency

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> document_id_to_relevance;
        
        for (const string& word : query.plus_words) {
            if (word_to_document_id_to_term_frequency_.count(word) == 0) {
                continue;
            }
            
            const double inverse_document_frequency = ComputeWordInverseDocumentFrequency(word);
            
            for (const auto &[document_id, term_frequency] : word_to_document_id_to_term_frequency_.at(word)) {
                document_id_to_relevance[document_id] += term_frequency * inverse_document_frequency;
            }
        }
        
        for (const string& word : query.minus_words) {
            if (word_to_document_id_to_term_frequency_.count(word) == 0) {
                continue;
            }
            
            for (const auto &[document_id, _] : word_to_document_id_to_term_frequency_.at(word)) {
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
    } // FindAllDocuments

private:
    set<string> stop_words_;
    
    map<string, map<int, double>> word_to_document_id_to_term_frequency_;
    
    map<int, DocumentData> document_id_to_document_data_;
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

    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::kActual, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::kActual, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::kActual, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::kBanned, {9});

    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }
    
    const auto find_documents_actual_status = []([[maybe_unused]] int document_id,
                                DocumentStatus status,
                                [[maybe_unused]] int relevance) {
                                    return status == DocumentStatus::kActual;
                                };
    
    cout << "ACTUAL:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, find_documents_actual_status)) {
        PrintDocument(document);
    }
    
    cout << "BANNED:"s << endl;
        for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::kBanned)) {
            PrintDocument(document);
        }

    const auto find_documents_with_even_id_numbers = [](int document_id,
                                                        [[maybe_unused]] DocumentStatus status,
                                                        [[maybe_unused]] int relevance) {
                                                            return document_id % 2 == 0;
                                                        };
    
    cout << "Even ids:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, find_documents_with_even_id_numbers)) {
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
