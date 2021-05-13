#include "request_queue.hpp"

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query,
                                     DocumentStatus status)  {
    return AddFindRequest(raw_query, [&status](int, DocumentStatus doc_status, int){
        return doc_status == status;
    });
}

int RequestQueue::GetNoResultRequests() const {
    return static_cast<int>(requests_.size());
}
