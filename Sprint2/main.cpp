#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

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
    std::vector<std::string> words;
    
    std::string word;
    for (const char character : text) {
        if (character == ' ') {
            words.push_back(word);
            word = ""s;
        } else {
            word += character;
        }
    }
    words.push_back(word);
    
    return words;
}
    
struct Document {
    int id = 0;
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
    void SetStopWords(const std::string& text) {
        for (const std::string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    } // SetStopWords
    
    void AddDocument(int document_id, const std::string& document,
                     DocumentStatus status, const std::vector<int>& ratings) {
        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        
        const double inverse_word_count = 1.0 / static_cast<double>(words.size());
        
        for (const std::string& word : words) {
            word_to_document_id_to_term_frequency_[word][document_id] += inverse_word_count;
        }
        
        document_id_to_document_data_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
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
                      if (abs(left.relevance - right.relevance) < kAccuracy) {
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
        const auto predicate = [desired_status]([[maybe_unused]] int document_id, DocumentStatus document_status,
                                                [[maybe_unused]] int rating) {
            return document_status == desired_status;
        };
        
        return FindTopDocuments(raw_query, predicate);
    } // FindTopDocuments with status as a second argument
    
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
                                                                       int document_id) const {
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
        
        return {matched_words, document_id_to_document_data_.at(document_id).status};
    } // MatchDocument
    
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
        
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
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
        const double number_of_documents_constains_word =
            static_cast<double>(word_to_document_id_to_term_frequency_.at(word).size());
        
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

private:
    std::set<std::string> stop_words_;
    
    std::map<std::string, std::map<int, double>> word_to_document_id_to_term_frequency_;
    
    std::map<int, DocumentData> document_id_to_document_data_;
};

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        assert(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        assert(doc0.id == doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        assert(server.FindTopDocuments("in"s).empty());
    }
}

/*
Разместите код остальных тестов здесь
*/

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
