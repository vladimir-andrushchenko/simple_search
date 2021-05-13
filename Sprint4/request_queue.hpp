#ifndef request_queue_hpp
#define request_queue_hpp

#include <deque>

#include "document.hpp"
#include "search_server.hpp"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server): server_(search_server) {}
    
public:
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentStatus status = DocumentStatus::ACTUAL);
    
    int GetNoResultRequests() const;
    
private:
    struct QueryResult {
    public:
        QueryResult(u_int64_t current_time): time_created(current_time) {}
        
    public:
        u_int64_t time_created;
    };
    
private:
    static constexpr int kSecondsInADay = 1440;
    
private:
    std::deque<QueryResult> requests_;
    const SearchServer& server_;
    u_int64_t time_ = 0;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    ++time_;
    
    if (!requests_.empty() && ((time_ - requests_.front().time_created) >= kSecondsInADay)) {
        requests_.pop_front();
    }
    
    const std::vector<Document>& results = server_.FindTopDocuments(raw_query, document_predicate);
    
    if (results.empty()) {
        requests_.push_back(QueryResult(time_));
    }
    
    return results;
}

#endif /* request_queue_hpp */
