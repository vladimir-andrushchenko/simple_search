#pragma once

#include <deque>

#include "document.hpp"
#include "search_server.hpp"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
public:
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentStatus status = DocumentStatus::kActual);
    
    int GetNoResultRequests() const;
    
private:
    struct QueryResult {
    public:
        QueryResult(const std::string& raw_query, int results): raw_query(raw_query), results(results) {}
        
    public:
        const std::string raw_query;
        const int results;
    };
    
private:
    static constexpr int kMinutessInADay = 1440;
    
private:
    std::deque<QueryResult> requests_;
    const SearchServer& server_;
    int no_result_requests_counter_ = 0;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    if (requests_.size() >= kMinutessInADay) {
        if(requests_.front().results == 0) {
            --no_result_requests_counter_;
        }
        
        requests_.pop_front();
    }
    
    const std::vector<Document>& results = server_.FindTopDocuments(raw_query, document_predicate);
    
    requests_.push_back(QueryResult(raw_query, static_cast<int>(results.size())));
    
    if (results.empty()) {
        ++no_result_requests_counter_;
    }
    
    return results;
}
