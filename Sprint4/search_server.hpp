//
//  search_server.hpp
//  Sprint4
//
//  Created by Vladimir Andrushchenko on 12.05.2021.
//

#ifndef search_server_hpp
#define search_server_hpp

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "document.hpp"

class SearchServer {
public:
    SearchServer() = default;
    
    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words);
    
    explicit SearchServer(const std::string& stop_words);
    
public:
    void SetStopWords(const std::string& text);
    
    bool AddDocument(int document_id, const std::string& document,
                     DocumentStatus status, const std::vector<int>& ratings);
    
    int GetDocumentCount() const;
    
    template<typename Predicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicate predicate) const;
    
    std::vector<Document> FindTopDocuments(const std::string& raw_query,
                                           const DocumentStatus& desired_status = DocumentStatus::ACTUAL) const;
    
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;
    
    int GetDocumentId(int index) const;
    
private:
    struct DocumentData {
        int rating = 0;
        DocumentStatus status = DocumentStatus::ACTUAL;
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
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    
    static int ComputeAverageRating(const std::vector<int>& ratings);
    
    bool IsStopWord(const std::string& word) const;
    
    QueryWord ParseQueryWord(std::string text) const;
    
    Query ParseQuery(const std::string& text) const;
    
    // Existence required
    double ComputeWordInverseDocumentFrequency(const std::string& word) const;
    
    std::vector<Document> FindAllDocuments(const Query& query) const;
    
    static bool IsValidWord(const std::string& word);
    
private:
    std::set<std::string> stop_words_;
    
    std::map<std::string, std::map<int, double>> word_to_document_id_to_term_frequency_;
    
    std::map<int, DocumentData> document_id_to_document_data_;
    
    std::vector<int> document_ids_;
};

template <typename StringCollection>
SearchServer::SearchServer(const StringCollection& stop_words) {
    using namespace std::literals;
    
    for (const auto& stop_word : stop_words) {
        if (!IsValidWord(stop_word)) {
            throw std::invalid_argument("stop word contains unaccaptable symbol"s);
        }
        
        stop_words_.insert(stop_word);
    }
}

template<typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, Predicate predicate) const {
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

#endif /* search_server_hpp */
