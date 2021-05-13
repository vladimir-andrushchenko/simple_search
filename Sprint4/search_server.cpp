#include "search_server.hpp"
#include "string_processing.hpp"

using namespace std::literals;

// log DocumentStatus is necessary for testing framework
std::ostream& operator<<(std::ostream& out, const DocumentStatus status) {
    switch (status) {
        case DocumentStatus::kActual:
            out << "kActual"s;
            break;
        case DocumentStatus::kBanned:
            out << "kBanned"s;
            break;
        case DocumentStatus::kIrrelevant:
            out << "kIrrelevant"s;
            break;
        case DocumentStatus::kRemoved:
            out << "kRemoved"s;
            break;
    }
    
    return out;
}

SearchServer::SearchServer(const std::string& stop_words) {
    if (!IsValidWord(stop_words)) {
        throw std::invalid_argument("stop word contains unaccaptable symbol"s);
    }
    
    SetStopWords(stop_words);
}


void SearchServer::SetStopWords(const std::string& text) {
    for (const std::string& word : SplitIntoWords(text)) {
        stop_words_.insert(word);
    }
} // SetStopWords

bool SearchServer::AddDocument(int document_id, const std::string& document,
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

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(document_id_to_document_data_.size());
} // GetDocumentCount



std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
                                                     const DocumentStatus& desired_status) const {
    const auto predicate = [desired_status](int , DocumentStatus document_status, int ) {
        return document_status == desired_status;
    };
    
    return FindTopDocuments(raw_query, predicate);
} // FindTopDocuments with status as a second argument

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
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

int SearchServer::GetDocumentId(int index) const {
    return document_ids_.at(index);
}


std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    
    return words;
} // SplitIntoWordsNoStop

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    int rating_sum = 0;
    
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    
    return rating_sum / static_cast<int>(ratings.size());
} // ComputeAverageRating

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
} // IsStopWord

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    if (text.empty()) {
        throw std::invalid_argument("caught empty word, check for double spaces"s);
    }
    
    bool is_minus = false;
    
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

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
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
double SearchServer::ComputeWordInverseDocumentFrequency(const std::string& word) const {
    assert(word_to_document_id_to_term_frequency_.count(word) != 0);
    
    const size_t number_of_documents_constains_word = word_to_document_id_to_term_frequency_.at(word).size();
    
    assert(number_of_documents_constains_word != 0);
    
    return log(static_cast<double>(GetDocumentCount()) / number_of_documents_constains_word);
} // ComputeWordInverseDocumentFrequency

std::vector<Document> SearchServer::FindAllDocuments(const Query& query) const {
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

bool SearchServer::IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
} // IsValidWord

