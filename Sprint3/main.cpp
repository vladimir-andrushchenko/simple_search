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

#include "Test.h"

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

// defined in Test.h
//enum class DocumentStatus {
//    ACTUAL,
//    IRRELEVANT,
//    BANNED,
//    REMOVED,
//};

class SearchServer {
public:
    // Defines an invalid document id
    // You can refer to this constant as SearchServer::INVALID_DOCUMENT_ID
    inline static constexpr int INVALID_DOCUMENT_ID = -1;
    
public:
    SearchServer() = default;
    
    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words) {
        for (const auto stop_word : stop_words) {
            stop_words_.insert(stop_word);
        }
    }
    
    explicit SearchServer(const std::string& stop_words) {
        SetStopWords(stop_words);
    }
    
public:
    void SetStopWords(const std::string& text) {
        for (const std::string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    } // SetStopWords
    
    [[nodiscard]] bool AddDocument(int document_id, const std::string& document,
                                   DocumentStatus status, const std::vector<int>& ratings) {
        if ((std::count(document_ids_.begin(), document_ids_.end(), document_id) > 0) ||
            (document_id < 0)) {
            return false;
        }
        
        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        
        for (const std::string& word : words) {
            if (!IsValidWord(word)) {
                return false;
            }
        }
        
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
    [[nodiscard]] bool FindTopDocuments(const std::string& raw_query, Predicate predicate, std::vector<Document>& result) const {
        if (IsValidQuery(raw_query) == false) {
            return false;
        }
        
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
        
        result = filtered_documents;
        return true;
    } // FindTopDocuments
    
    [[nodiscard]] bool FindTopDocuments(const std::string& raw_query,
                                        const DocumentStatus& desired_status,
                                        std::vector<Document>& result) const {
        const auto predicate = [desired_status](int , DocumentStatus document_status, int ) {
            return document_status == desired_status;
        };
        
        return FindTopDocuments(raw_query, predicate, result);
    } // FindTopDocuments with status as a second argument
    
    [[nodiscard]] bool FindTopDocuments(const std::string& raw_query,
                                        std::vector<Document>& result) const {
        const auto predicate = [](int , DocumentStatus document_status, int ) {
            return document_status == DocumentStatus::ACTUAL;
        };
        
        return FindTopDocuments(raw_query, predicate, result);
    }
    
    [[nodiscard]] bool MatchDocument(const std::string& raw_query, int document_id,
                                     std::tuple<std::vector<std::string>, DocumentStatus>& result) const {
        if (IsValidQuery(raw_query) == false) {
            return false;
        }
        
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
        
        result = {matched_words, document_id_to_document_data_.at(document_id).status};
        return true;
    } // MatchDocument
    
    int GetDocumentId(int index) const {
        if ((index < 0) ||
            (static_cast<size_t>(index) >= document_ids_.size())) {
            return INVALID_DOCUMENT_ID;
        }
        
        return document_ids_.at(index);
    }
    
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
    
    static bool IsValidWord(const std::string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    } // IsValidWord
    
    static bool IsValidQuery(const std::string& query) {
        if ((query.empty()) ||
            (query.back()  == '-') ||                   // ends with minus, meaning empty minus word
            (query.find("--"s) != std::string::npos) || // double minus word
            (query.find("- "s) != std::string::npos)) { // empty muinus word
            return false;
        }
        
        return true;
    }
    
private:
    std::set<std::string> stop_words_;
    
    std::map<std::string, std::map<int, double>> word_to_document_id_to_term_frequency_;
    
    std::map<int, DocumentData> document_id_to_document_data_;
    
    std::vector<int> document_ids_;
};

// tests
void PrintDocument(const Document& document) {
    std::cout << "{ "s
    << "document_id = "s << document.id << ", "s
    << "relevance = "s << document.relevance << ", "s
    << "rating = "s << document.rating
    << " }"s << std::endl;
}

void TestSplitIntoWordsEscapesSpaces() {
    ASSERT_EQUAL((std::vector<std::string> {"hello"s, "bro"s}), SplitIntoWords("   hello    bro    "s));
    ASSERT_EQUAL(std::vector<std::string>{}, SplitIntoWords("                 "));
}

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("in"s, found_docs);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("in"s, found_docs);
        ASSERT_HINT(found_docs.empty(), "Stop words must be excluded from documents"s);
    }
}

void TestAddedDocumentsCanBeFoundUsingQuery() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("cat in the city", found_docs);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].id, doc_id);
    }
    
    {
        SearchServer server;
        (void) server.AddDocument(doc_id, "", DocumentStatus::ACTUAL, ratings);
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("cat", found_docs);
        
        ASSERT(found_docs.empty());
    }
}

void TestMinusWordsInQueryExcludeDocumentsFromSearchResults() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    
    const int doc_id2 = 43;
    const std::string content2 = "happy dog"s;
    
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("-cat", found_docs);
        
        ASSERT(found_docs.empty());
    }
    
    // minus words dont exclude documents where they dont appear
    {
        SearchServer server;
        
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("-cat dog", found_docs);
        
        ASSERT(found_docs.size() == 1);
    }
}

void TestMatchDocumentReturnsIntersectionOfWordsFromQueryAndWordsInDocument() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    
    const int doc_id2 = 43;
    const std::string content2 = "happy dog"s;
    
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        
        std::tuple<std::vector<std::string>, DocumentStatus> words_and_status;
        (void) server.MatchDocument("fat cat out of city", doc_id, words_and_status);
        
        const auto& [words, status] = words_and_status;
        
        std::vector<std::string> desired_matched_words{"cat", "city"};
        
        ASSERT_EQUAL(words, desired_matched_words);
        ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
    }
    
    // status banned
    {
        SearchServer server;
        
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id2, content2, DocumentStatus::BANNED, ratings);
        
        std::tuple<std::vector<std::string>, DocumentStatus> words_and_status;
        (void) server.MatchDocument("fat cat out of city and a cute dog", doc_id2, words_and_status);
        
        const auto& [words, status] = words_and_status;
        
        std::vector<std::string> desired_matched_words{"dog"};
        
        ASSERT_EQUAL(words, desired_matched_words);
        ASSERT_EQUAL(status, DocumentStatus::BANNED);
    }
}

void TestDocumentsFoundBySearchServerAreSortedByRelevanceInDescendingOrder() {
    constexpr double kAccuracy = 1e-6;
    
    const int doc_id = 1;
    const std::string content = "cat city"s; // only one word matches query
    
    const int doc_id_2 = 2;
    const std::string content_2 = "dog city potato"s;
    
    const int doc_id_3 = 3;
    const std::string content_3 = "dog city"s;
    
    const int doc_id_4 = 4;
    const std::string content_4 = "lorem ipsum"s;
    
    const int doc_id_5 = 5;
    const std::string content_5 = "city"s;
    
    const int doc_id_6 = 6;
    const std::string content_6 = "frog city"s;
    
    const int doc_id_7 = 7;
    const std::string content_7 = "the cat says meow to dog"s;
    
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id_4, content_4, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id_5, content_5, DocumentStatus::BANNED, ratings); // kBanned
        (void) server.AddDocument(doc_id_6, content_6, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id_7, content_7, DocumentStatus::ACTUAL, ratings);
        
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("dog in the city", found_docs);
        
        ASSERT_EQUAL(found_docs.size(), 5u);
        
        ASSERT(std::is_sorted(found_docs.begin(), found_docs.end(), [](const Document& left, const Document& right) {
            if (std::abs(left.relevance - right.relevance) < kAccuracy) {
                return left.rating > right.rating;
            } else {
                return left.relevance > right.relevance;
            }
        }));
    }
    
    {
        SearchServer server;
        
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings);
        (void) server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings);
        
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("cat loves NY city", found_docs);
        
        ASSERT(found_docs.size() == 3);
        
        ASSERT(std::is_sorted(found_docs.begin(), found_docs.end(), [](const Document& left, const Document& right) {
            if (std::abs(left.relevance - right.relevance) < kAccuracy) {
                return left.rating > right.rating;
            } else {
                return left.relevance > right.relevance;
            }
        }));
    }
}

void TestRatingOfFoundDocumentIsAverageOfRatings() {
    const int doc_id = 1;
    const std::string content = "cat city"s;
    
    // positive ratings
    {
        SearchServer server;
        
        const std::vector<int> ratings = {1, 2, 3};
        
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("cat loves NY city", found_docs);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].rating, 2);
    }
    
    // negative ratings
    {
        SearchServer server;
        
        const std::vector<int> ratings = {-1, -2, -3};
        
        (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("cat loves NY city", found_docs);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].rating, -2);
    }
}

void TestPredicateFilteringOfSearchResults() {
    const int doc_id = 1;
    const std::string content = "cat city"s; // only one word matches query
    
    const int doc_id_2 = 2;
    const std::string content_2 = "dog city potato"s;
    
    const int doc_id_3 = 3;
    const std::string content_3 = "dog city"s;
    
    const std::vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    
    (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    (void) server.AddDocument(doc_id_2, content_2, DocumentStatus::BANNED, ratings);
    (void) server.AddDocument(doc_id_3, content_3, DocumentStatus::REMOVED, ratings);
    
    // status predicate
    {
        std::vector<Document> filtered_docs;
        (void) server.FindTopDocuments("city", [](int , DocumentStatus status, int ) {
            return status == DocumentStatus::ACTUAL;
        }, filtered_docs);
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 1);
    }
    
    // rating predicate
    {
        std::vector<Document> filtered_docs;
        (void) server.FindTopDocuments("city", [](int , DocumentStatus , int rating) {
            return rating == 2;
        }, filtered_docs);
        
        ASSERT_EQUAL(filtered_docs.size(), 3u); // only a doc with higher rating is found
        ASSERT_EQUAL(filtered_docs[0].rating, 2);
    }
    
    // id predicate
    {
        const int document_id_to_search_for = 1;
        
        std::vector<Document> filtered_docs;
        (void) server.FindTopDocuments("city", [](int document_id, DocumentStatus , int ) {
            return document_id == document_id_to_search_for;
        }, filtered_docs);
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, document_id_to_search_for);
    }
}

void TestStatusFilteringOfSearchResults() {
    const int doc_id = 1;
    const std::string content = "cat city"s; // only one word matches query
    
    const int doc_id_2 = 2;
    const std::string content_2 = "dog city potato"s;
    
    const std::vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    
    (void) server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    (void) server.AddDocument(doc_id_2, content_2, DocumentStatus::BANNED, ratings);
    
    // explicit status
    {
        std::vector<Document> filtered_docs;
        (void) server.FindTopDocuments("city", DocumentStatus::BANNED, filtered_docs);
        
        ASSERT_EQUAL(filtered_docs.size(), 1);
        ASSERT_EQUAL(filtered_docs[0].id, doc_id_2);
    }
    
    // default status
    {
        std::vector<Document> filtered_docs;
        (void) server.FindTopDocuments("city", filtered_docs);
        
        ASSERT_EQUAL(filtered_docs.size(), 1);
        ASSERT_EQUAL(filtered_docs[0].id, doc_id);
    }
}

void TestRelevanceOfTheFoundDocumentsIsCorrect() {
    constexpr double kAccuracy = 1e-6;
    
    SearchServer server;
    
    (void) server.AddDocument(0, "cat cat city dog"s, DocumentStatus::ACTUAL, {1});
    (void) server.AddDocument(1, "city dog"s, DocumentStatus::ACTUAL, {1});
    (void) server.AddDocument(2, "cat city potato"s, DocumentStatus::ACTUAL, {1});
    
    // positive case
    {
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("cat"s, found_docs);
        
        // log(documents_count * 1.0 / a) * (b / c)
        double expected_relevance_doc_0 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (2.0 / 4.0);
        double expected_relevance_doc_2 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (1.0 / 3.0);
        
        ASSERT_EQUAL(found_docs.size(), 2u);
        
        ASSERT(std::abs(found_docs[0].relevance - expected_relevance_doc_0) < kAccuracy);
        ASSERT(std::abs(found_docs[1].relevance - expected_relevance_doc_2) < kAccuracy);
    }
    
    // zero relevance
    {
        std::vector<Document> found_docs;
        (void) server.FindTopDocuments("city"s, found_docs);
        
        ASSERT_EQUAL(found_docs.size(), 3u);
        
        ASSERT(std::abs(found_docs[0].relevance) < kAccuracy);
        ASSERT(std::abs(found_docs[1].relevance) < kAccuracy);
        ASSERT(std::abs(found_docs[2].relevance) < kAccuracy);
    }
}

void TestGetDocumentId() {
    SearchServer search_server;
    (void) search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    (void) search_server.AddDocument(2, "смешной пёс"s, DocumentStatus::ACTUAL, {7, 2, 7});
    
    ASSERT_EQUAL(search_server.GetDocumentId(0), 1);
    ASSERT_EQUAL(search_server.GetDocumentId(1), 2);
    
    ASSERT_EQUAL(search_server.GetDocumentId(-1), SearchServer::INVALID_DOCUMENT_ID);
    ASSERT_EQUAL(search_server.GetDocumentId(2), SearchServer::INVALID_DOCUMENT_ID);
}

void TestAddDocumentWithRepeatingId() {
    SearchServer search_server;
    
    (void) search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    
    ASSERT_HINT(search_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2}) == false,
                "adding document with already existing id return false");
    
    std::tuple<std::vector<std::string>, DocumentStatus> words_and_status;
    (void) search_server.MatchDocument("кот", 1, words_and_status);
    
    const auto& [words, doc] = words_and_status;
    
    ASSERT(words.size() == 1);
}

void TestAddDocumentWithNegativeId() {
    SearchServer search_server;
    
    ASSERT_HINT(search_server.AddDocument(-1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2}) == false,
                "adding document with negative id should return false");
}

void TestAddDocumentWithSpecialSymbolIsHandled() {
    SearchServer search_server;
    ASSERT_HINT(search_server.AddDocument(3, "большой пёс скво\x12рец"s, DocumentStatus::ACTUAL, {1, 3, 2}) == false,
                "adding document with special simbol should return false");
}

void TestDoubleMinusIsHandled() {
    SearchServer search_server;
    
    (void) search_server.AddDocument(1, "пушистый кот пушистый хвост иван-чай"s, DocumentStatus::ACTUAL, {7, 2, 7});
    
    std::vector<Document> documents;
    ASSERT(search_server.FindTopDocuments("--пушистый"s, documents) == false);
    ASSERT(search_server.FindTopDocuments("иван-чай"s, documents) == true);
}

void TestMinusWithoutWordIsHandled() {
    SearchServer search_server;
    
    (void) search_server.AddDocument(1, "пушистый кот пушистый хвост иван-чай"s, DocumentStatus::ACTUAL, {7, 2, 7});
    
    std::vector<Document> documents;
    ASSERT(search_server.FindTopDocuments("пушистый -"s, documents) == false);
    ASSERT(search_server.FindTopDocuments("пушистый - кот"s, documents) == false);
}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddedDocumentsCanBeFoundUsingQuery);
    RUN_TEST(TestMinusWordsInQueryExcludeDocumentsFromSearchResults);
    RUN_TEST(TestMatchDocumentReturnsIntersectionOfWordsFromQueryAndWordsInDocument);
    RUN_TEST(TestDocumentsFoundBySearchServerAreSortedByRelevanceInDescendingOrder);
    RUN_TEST(TestRatingOfFoundDocumentIsAverageOfRatings);
    RUN_TEST(TestPredicateFilteringOfSearchResults);
    RUN_TEST(TestStatusFilteringOfSearchResults);
    RUN_TEST(TestRelevanceOfTheFoundDocumentsIsCorrect);
    RUN_TEST(TestSplitIntoWordsEscapesSpaces);
    RUN_TEST(TestGetDocumentId);
    RUN_TEST(TestAddDocumentWithRepeatingId);
    RUN_TEST(TestAddDocumentWithNegativeId);
    RUN_TEST(TestAddDocumentWithSpecialSymbolIsHandled);
    RUN_TEST(TestDoubleMinusIsHandled);
    RUN_TEST(TestMinusWithoutWordIsHandled);
}

int main() {
    TestSearchServer();
    
    //    SearchServer search_server("и в на"s);
    ////    search_server.SetStopWords("и в на"s);
    //    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    //    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    //    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    //    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
    //    std::cout << "ACTUAL by default:"s << std::endl;
    //    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
    //        PrintDocument(document);
    //    }
    //    std::cout << "BANNED:"s << std::endl;
    //    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
    //        PrintDocument(document);
    //    }
    //    std::cout << "Even ids:"s << std::endl;
    //    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus , int ) { return document_id % 2 == 0; })) {
    //        PrintDocument(document);
    //    }
}
