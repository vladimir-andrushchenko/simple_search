#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <numeric>
#include <sstream>

using namespace std::literals;

std::string ReadLine() {
    std::string line;
    getline(std::cin, line);
    return line;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::istringstream text_stream(text);
    
    std::vector<std::string> words;
    
    std::string word;
    while (text_stream >> word) {
        words.push_back(word);
    }
    
    return words;
}

struct Document {
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
    
    Document(): id(0), relevance(0), rating(0) {};
    Document(int id, double relevance, int rating): id(id), relevance(relevance), rating(rating) {};
};

enum class DocumentStatus {
    kActual,
    kIrrelevant,
    kBanned,
    kRemoved,
};

// log DocumentStatus is necessary for testing framework
std::ostream& operator<<(std::ostream& out, const DocumentStatus status) {
    switch (status) {
        case DocumentStatus::kActual:
            out << "ACTUAL"s;
            break;
        case DocumentStatus::kBanned:
            out << "BANNED"s;
            break;
        case DocumentStatus::kIrrelevant:
            out << "IRRELEVANT"s;
            break;
        case DocumentStatus::kRemoved:
            out << "REMOVED"s;
            break;
    }
    
    return out;
}

class SearchServer {
public:
    SearchServer() = default;
    
    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words) {
        for (const auto& stop_word : stop_words) {
            if (!IsValidWord(stop_word)) {
                throw std::invalid_argument("stop word contains unaccaptable symbol"s);
            }
            
            stop_words_.insert(stop_word);
        }
    }
    
    explicit SearchServer(const std::string& stop_words) {
        if (!IsValidWord(stop_words)) {
            throw std::invalid_argument("stop word contains unaccaptable symbol"s);
        }
        
        SetStopWords(stop_words);
    }
    
public:
    void SetStopWords(const std::string& text) {
        for (const std::string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    } // SetStopWords
    
    bool AddDocument(int document_id, const std::string& document,
                     DocumentStatus status, const std::vector<int>& ratings) {
        if (document_id < 0) {
            throw std::invalid_argument("negative ids are not allowed"s);
        }
        
        if (document_id_to_document_data_.count(document_id) > 0) {
            throw std::invalid_argument("repeating ids are not allowed"s);
        }
        
        if (!IsValidWord(document)) {
            throw std::invalid_argument("word in document contains unaccaptable symbol"s);
        }
        
        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        
        const double inverse_word_count = 1.0 / static_cast<double>(words.size());
        
        for (const std::string& word : words) {
            word_to_document_id_to_term_frequency_[word][document_id] += inverse_word_count;
        }
        
        document_ids_.push_back(document_id);
        
        document_id_to_document_data_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        
        return true;
    } // AddDocument
    
    int GetDocumentCount() const {
        return static_cast<int>(document_id_to_document_data_.size());
    } // GetDocumentCount
    
    template<typename Predicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicate predicate) const {
        const Query query = ParseQuery(raw_query);
        
        std::vector<Document> matched_documents = FindAllDocuments(query);
        
        std::vector<Document> filtered_documents;
        for (const Document& document : matched_documents) {
            const auto document_status = document_id_to_document_data_.at(document.id).status;
            const auto document_rating = document_id_to_document_data_.at(document.id).rating;
            
            if (predicate(document.id, document_status, document_rating)) {
                filtered_documents.push_back(document);
            }
        }
        
        std::sort(filtered_documents.begin(), filtered_documents.end(),
                  [](const Document& left, const Document& right) {
            if (std::abs(left.relevance - right.relevance) < kAccuracy) {
                return left.rating > right.rating;
            } else {
                return left.relevance > right.relevance;
            }
        });
        
        if (static_cast<int>(filtered_documents.size()) > kMaxResultDocumentCount) {
            filtered_documents.resize(static_cast<size_t>(kMaxResultDocumentCount));
        }
        
        return filtered_documents;
    } // FindTopDocuments
    
    std::vector<Document> FindTopDocuments(const std::string& raw_query,
                                           const DocumentStatus& desired_status = DocumentStatus::kActual) const {
        const auto predicate = [desired_status](int , DocumentStatus document_status, int ) {
            return document_status == desired_status;
        };
        
        return FindTopDocuments(raw_query, predicate);
    } // FindTopDocuments with status as a second argument
    
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        
        std::vector<std::string> matched_words;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_id_to_term_frequency_.count(word) == 0) {
                continue;
            }
            
            if (word_to_document_id_to_term_frequency_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        
        for (const std::string& word : query.minus_words) {
            if (word_to_document_id_to_term_frequency_.count(word) == 0) {
                continue;
            }
            
            if (word_to_document_id_to_term_frequency_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        
        return std::tuple<std::vector<std::string>, DocumentStatus>{matched_words, document_id_to_document_data_.at(document_id).status};
    } // MatchDocument
    
    int GetDocumentId(int index) const {
        return document_ids_.at(index);
    }
    
private:
    struct DocumentData {
        int rating = 0;
        DocumentStatus status = DocumentStatus::kActual;
    };
    
    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };
    
    struct QueryWord {
        std::string data;
        bool is_minus = false;
        bool is_stop = false;
    };
    
private:
    static constexpr int kMaxResultDocumentCount = 5;
    static constexpr double kAccuracy = 1e-6;
    
private:
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const {
        std::vector<std::string> words;
        for (const std::string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        
        return words;
    } // SplitIntoWordsNoStop
    
    static int ComputeAverageRating(const std::vector<int>& ratings) {
        int rating_sum = 0;
        
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        
        return rating_sum / static_cast<int>(ratings.size());
    } // ComputeAverageRating
    
    bool IsStopWord(const std::string& word) const {
        return stop_words_.count(word) > 0;
    } // IsStopWord
    
    QueryWord ParseQueryWord(std::string text) const {
        bool is_minus = false;
        
        if (text.empty()) {
            throw std::invalid_argument("caught empty word, check for double spaces"s);
        }
        
        if (text[0] == '-') {
            text = text.substr(1);
            
            if (text.empty()) {
                throw std::invalid_argument("empty minus words are not allowed"s);
            }
            
            if (text[0] == '-') {
                throw std::invalid_argument("double minus words are not allowed"s);
            }
            
            is_minus = true;
        }
        
        if (!IsValidWord(text)) {
            throw std::invalid_argument("special symbols in words are not allowed"s);
        }
        
        return {text, is_minus, IsStopWord(text)};
    } // ParseQueryWord
    
    Query ParseQuery(const std::string& text) const {
        Query query;
        
        for (const std::string& word : SplitIntoWords(text)) {
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
    double ComputeWordInverseDocumentFrequency(const std::string& word) const {
        assert(word_to_document_id_to_term_frequency_.count(word) != 0);
        
        const size_t number_of_documents_constains_word = word_to_document_id_to_term_frequency_.at(word).size();
        
        assert(number_of_documents_constains_word != 0);
        
        return log(static_cast<double>(GetDocumentCount()) / number_of_documents_constains_word);
    } // ComputeWordInverseDocumentFrequency
    
    std::vector<Document> FindAllDocuments(const Query& query) const {
        std::map<int, double> document_id_to_relevance;
        
        for (const std::string& word : query.plus_words) {
            if (word_to_document_id_to_term_frequency_.count(word) == 0) {
                continue;
            }
            
            const double inverse_document_frequency = ComputeWordInverseDocumentFrequency(word);
            
            for (const auto &[document_id, term_frequency] : word_to_document_id_to_term_frequency_.at(word)) {
                document_id_to_relevance[document_id] += term_frequency * inverse_document_frequency;
            }
        }
        
        for (const std::string& word : query.minus_words) {
            if (word_to_document_id_to_term_frequency_.count(word) == 0) {
                continue;
            }
            
            for (const auto &[document_id, _] : word_to_document_id_to_term_frequency_.at(word)) {
                document_id_to_relevance.erase(document_id);
            }
        }
        
        std::vector<Document> matched_documents;
        for (const auto &[document_id, relevance] : document_id_to_relevance) {
            matched_documents.push_back({ document_id, relevance,
                document_id_to_document_data_.at(document_id).rating});
        }
        
        return matched_documents;
    } // FindAllDocuments
    
    static bool IsValidWord(const std::string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    } // IsValidWord
    
private:
    std::set<std::string> stop_words_;
    
    std::map<std::string, std::map<int, double>> word_to_document_id_to_term_frequency_;
    
    std::map<int, DocumentData> document_id_to_document_data_;
    
    std::vector<int> document_ids_;
};

void PrintDocument(const Document& document) {
    std::cout << "{ "s
    << "document_id = "s << document.id << ", "s
    << "relevance = "s << document.relevance << ", "s
    << "rating = "s << document.rating << " }"s << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) {
    std::cout << "{ "s
    << "document_id = "s << document_id << ", "s
    << "status = "s << static_cast<int>(status) << ", "s
    << "words ="s;
    for (const std::string& word : words) {
        std::cout << ' ' << word;
    }
    std::cout << "}"s << std::endl;
}

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    } catch (const std::exception& e) {
        std::cout << "???????????? ???????????????????? ?????????????????? "s << document_id << ": "s << e.what() << std::endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    std::cout << "???????????????????? ???????????? ???? ??????????????: "s << raw_query << std::endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    } catch (const std::exception& e) {
        std::cout << "???????????? ????????????: "s << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    try {
        std::cout << "?????????????? ???????????????????? ???? ??????????????: "s << query << std::endl;
        const int document_count = search_server.GetDocumentCount();
        for (int index = 0; index < document_count; ++index) {
            const int document_id = search_server.GetDocumentId(index);
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const std::exception& e) {
        std::cout << "???????????? ???????????????? ???????????????????? ???? ???????????? "s << query << ": "s << e.what() << std::endl;
    }
}

int main() {
    SearchServer search_server;
    
    try {
        search_server = SearchServer("?? ?? ????"s);
    } catch (const std::invalid_argument& e) {
        std::cout << "???????????? ???????????????? search_server "s << ": "s << e.what() << std::endl;
    }
    
    AddDocument(search_server, 1, "???????????????? ?????? ???????????????? ??????????"s, DocumentStatus::kActual, {7, 2, 7});
    AddDocument(search_server, 1, "???????????????? ?????? ?? ???????????? ??????????????"s, DocumentStatus::kActual, {1, 2});
    AddDocument(search_server, -1, "???????????????? ?????? ?? ???????????? ??????????????"s, DocumentStatus::kActual, {1, 2});
    AddDocument(search_server, 3, "?????????????? ?????? ????????\x12?????? ??????????????"s, DocumentStatus::kActual, {1, 3, 2});
    AddDocument(search_server, 4, "?????????????? ?????? ?????????????? ??????????????"s, DocumentStatus::kActual, {1, 1, 1});
    
    FindTopDocuments(search_server, "???????????????? -??????"s);
    FindTopDocuments(search_server, "???????????????? --??????"s);
    FindTopDocuments(search_server, "???????????????? -"s);
    FindTopDocuments(search_server, "????????\x12??????"s);
    
    MatchDocuments(search_server, "???????????????? ??????"s);
    MatchDocuments(search_server, "???????????? -??????"s);
    MatchDocuments(search_server, "???????????? --??????"s);
    MatchDocuments(search_server, "???????????????? - ??????????"s);
}
