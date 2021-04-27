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
        
        if (std::count(document_ids_.begin(), document_ids_.end(), document_id) > 0) {
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
        if ((index < 0) ||
            (static_cast<size_t>(index) >= document_ids_.size())) {
            throw std::out_of_range("index of document is out of range"s);
        }
        
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
        if (text.substr(0, 2) == "--"s) {
            throw std::invalid_argument("double minus words are not allowed"s);
        }
        
        if (!IsValidWord(text)) {
            throw std::invalid_argument("special symbols in words are not allowed"s);
        }
        
        if (text.back() == '-') {
            throw std::invalid_argument("empty minus words are not allowed"s);
        }
        
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
    
private:
    std::set<std::string> stop_words_;
    
    std::map<std::string, std::map<int, double>> word_to_document_id_to_term_frequency_;
    
    std::map<int, DocumentData> document_id_to_document_data_;
    
    std::vector<int> document_ids_;
};

// tests
void TestStopWordsExclusion() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("in"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        
        ASSERT_EQUAL(found_docs[0].id, 42);
    }
    
    {
        SearchServer server("in the"s);
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("in"s);
        
        ASSERT_HINT(found_docs.empty(), "Stop words must be excluded from documents"s);
    }
}

void TestAddedDocumentsCanBeFound() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("cat in the city"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].id, 42);
    }
    
    {
        SearchServer server;
        
        server.AddDocument(42, "", DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("cat"s);
        
        ASSERT(found_docs.empty());
    }
}

void TestMinusWordsExcludeDocuments() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("-cat"s);
        
        ASSERT(found_docs.empty());
    }
    
    // minus words dont exclude documents where they dont appear in
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(43, "happy dog"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("-cat dog"s);
        
        ASSERT(found_docs.size() == 1);
    }
    
}

void TestMatchDocumentResults() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        const auto [words, status] = server.MatchDocument("fat cat out of city"s, 42);
        
        std::vector<std::string> desired_matched_words{"cat"s, "city"s};
        
        ASSERT_EQUAL(words, desired_matched_words);
        ASSERT_EQUAL(status, DocumentStatus::kActual);
    }
    
    // status kBanned
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(43, "happy dog"s, DocumentStatus::kBanned, ratings);
        
        const auto [words, status] = server.MatchDocument("fat cat out of city and a cute dog"s, 43);
        
        std::vector<std::string> desired_matched_words{"dog"s};
        
        ASSERT_EQUAL(words, desired_matched_words);
        ASSERT_EQUAL(status, DocumentStatus::kBanned);
    }
}

void TestFindTopDocumentsResultsSorting() {
    constexpr double kAccuracy = 1e-6;
    
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(2, "dog city potato"s, DocumentStatus::kActual, ratings);
        server.AddDocument(3, "dog city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(4, "lorem ipsum"s, DocumentStatus::kActual, ratings);
        server.AddDocument(5, "city"s, DocumentStatus::kBanned, ratings); // kkBanned
        server.AddDocument(6, "frog city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(7, "the cat says meow to dog"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("dog in the city"s);
        
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
        
        server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(2, "dog city potato"s, DocumentStatus::kActual, ratings);
        server.AddDocument(3, "dog city"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("cat loves NY city"s);
        
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

void TestRatingsCalculation() {
    const std::string content = "cat city"s;
    
    // positive ratings
    {
        SearchServer server;
        
        const std::vector<int> ratings = {1, 2, 3};
        
        server.AddDocument(1, content, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("cat loves NY city"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].rating, 2);
    }
    
    // negative ratings
    {
        SearchServer server;
        
        const std::vector<int> ratings = {-1, -2, -3};
        
        (void) server.AddDocument(1, content, DocumentStatus::kActual, ratings);
        
        std::vector<Document> found_docs = server.FindTopDocuments("cat loves NY city"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].rating, -2);
    }
}

void TestFilteringByPredicate() {
    const std::vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    
    server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
    server.AddDocument(2, "dog city potato"s, DocumentStatus::kBanned, ratings);
    server.AddDocument(3, "dog city"s, DocumentStatus::kRemoved, ratings);
    
    // status predicate
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s, [](int , DocumentStatus status, int ) {
            return status == DocumentStatus::kActual;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 1);
    }
    
    // rating predicate
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s, [](int , DocumentStatus , int rating) {
            return rating == 2;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 3u); // only a doc with higher rating is found
        ASSERT_EQUAL(filtered_docs[0].rating, 2);
    }
    
    // id predicate
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s, [](int document_id, DocumentStatus , int ) {
            return document_id == 1;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 1);
    }
}

void TestFilteringByStatus() {
    const std::vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    
    server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
    server.AddDocument(2, "dog city potato"s, DocumentStatus::kBanned, ratings);
    
    // explicit status
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s, DocumentStatus::kBanned);
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 2);
    }
    
    // default status
    {
        const auto& filtered_docs = server.FindTopDocuments("city"s);
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 1);
    }
}

void TestRelevanceCalculation() {
    constexpr double kAccuracy = 1e-6;
    
    SearchServer server;
    
    server.AddDocument(0, "cat cat city dog"s, DocumentStatus::kActual, {1});
    server.AddDocument(1, "city dog"s, DocumentStatus::kActual, {1});
    server.AddDocument(2, "cat city potato"s, DocumentStatus::kActual, {1});
    
    // positive case
    {
        std::vector<Document> found_docs = server.FindTopDocuments("cat"s);
        
        // log(documents_count * 1.0 / a) * (b / c)
        double expected_relevance_doc_0 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (2.0 / 4.0);
        double expected_relevance_doc_2 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (1.0 / 3.0);
        
        ASSERT_EQUAL(found_docs.size(), 2u);
        
        ASSERT(std::abs(found_docs[0].relevance - expected_relevance_doc_0) < kAccuracy);
        ASSERT(std::abs(found_docs[1].relevance - expected_relevance_doc_2) < kAccuracy);
    }
    
    // zero relevance
    {
        std::vector<Document> found_docs = server.FindTopDocuments("city"s);
        
        ASSERT_EQUAL(found_docs.size(), 3u);
        
        ASSERT(std::abs(found_docs[0].relevance) < kAccuracy);
        ASSERT(std::abs(found_docs[1].relevance) < kAccuracy);
        ASSERT(std::abs(found_docs[2].relevance) < kAccuracy);
    }
}

void TestSplitIntoWordsEscapesSpaces() {
    ASSERT_EQUAL((std::vector<std::string> {"hello"s, "bro"s}), SplitIntoWords("   hello    bro    "s));
    ASSERT_EQUAL(std::vector<std::string>{}, SplitIntoWords("                 "s));
}

void TestGetDocumentIdReturnsId() {
    SearchServer search_server;
    
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::kActual, {7, 2, 7});
    search_server.AddDocument(2, "смешной пёс"s, DocumentStatus::kActual, {7, 2, 7});
    
    ASSERT_EQUAL(search_server.GetDocumentId(0), 1);
    ASSERT_EQUAL(search_server.GetDocumentId(1), 2);
}

void TestAddDocumentWithRepeatingId() {
    SearchServer search_server;
    
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::kActual, {7, 2, 7});
    
    try {
        search_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::kActual, {1, 2});
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "adding document with repeating id is not handled"s);
}

void TestAddDocumentWithNegativeId() {
    SearchServer search_server;
    
    try {
        search_server.AddDocument(-1, "пушистый пёс и модный ошейник"s, DocumentStatus::kActual, {1, 2});
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "adding document with negative id is not handled"s);
}

void TestAddDocumentWithSpecialSymbol() {
    SearchServer search_server;
    
    try {
        search_server.AddDocument(1, "большой пёс скво\x12рец"s, DocumentStatus::kActual, {1, 2});
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "adding document containing unaccaptable symbol is not handled"s);
    
}

void TestQueryWithSpecialSymbol() {
    SearchServer search_server;
    
    try {
        search_server.FindTopDocuments("большой пёс скво\x12рец"s);
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "query with symbols in not handled"s);
}

void TestDoubleMinusWord() {
    SearchServer search_server;
    
    try {
        search_server.FindTopDocuments("--пушистый"s);
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "query with double minus in not handled"s);
}

void TestEmptyMinusWord() {
    SearchServer search_server;
    
    try {
        search_server.FindTopDocuments("пушистый -"s);
    } catch (std::invalid_argument& e) {
        return;
    }
    
    ASSERT_HINT(false, "query with empty minus word is not handled"s);
}

void TestSearchServer() {
    RUN_TEST(TestStopWordsExclusion);
    RUN_TEST(TestAddedDocumentsCanBeFound);
    RUN_TEST(TestMinusWordsExcludeDocuments);
    RUN_TEST(TestMatchDocumentResults);
    RUN_TEST(TestFindTopDocumentsResultsSorting);
    RUN_TEST(TestRatingsCalculation);
    RUN_TEST(TestFilteringByPredicate);
    RUN_TEST(TestFilteringByStatus);
    RUN_TEST(TestRelevanceCalculation);
    
    // tests for Sprint3
    RUN_TEST(TestSplitIntoWordsEscapesSpaces);
    RUN_TEST(TestGetDocumentIdReturnsId);
    RUN_TEST(TestAddDocumentWithRepeatingId);
    RUN_TEST(TestAddDocumentWithNegativeId);
    RUN_TEST(TestAddDocumentWithSpecialSymbol);
    RUN_TEST(TestDoubleMinusWord);
    RUN_TEST(TestQueryWithSpecialSymbol);
    RUN_TEST(TestEmptyMinusWord);
}

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
        std::cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << std::endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    std::cout << "Результаты поиска по запросу: "s << raw_query << std::endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    } catch (const std::exception& e) {
        std::cout << "Ошибка поиска: "s << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    try {
        std::cout << "Матчинг документов по запросу: "s << query << std::endl;
        const int document_count = search_server.GetDocumentCount();
        for (int index = 0; index < document_count; ++index) {
            const int document_id = search_server.GetDocumentId(index);
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const std::exception& e) {
        std::cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << std::endl;
    }
}

int main() {
    TestSearchServer();
    
    std::cout << "\n";
    
    SearchServer search_server("и в на"s);
    
    AddDocument(search_server, 1, "пушистый кот пушистый хвост"s, DocumentStatus::kActual, {7, 2, 7});
    AddDocument(search_server, 1, "пушистый пёс и модный ошейник"s, DocumentStatus::kActual, {1, 2});
    AddDocument(search_server, -1, "пушистый пёс и модный ошейник"s, DocumentStatus::kActual, {1, 2});
    AddDocument(search_server, 3, "большой пёс скво\x12рец евгений"s, DocumentStatus::kActual, {1, 3, 2});
    AddDocument(search_server, 4, "большой пёс скворец евгений"s, DocumentStatus::kActual, {1, 1, 1});
    
    FindTopDocuments(search_server, "пушистый -пёс"s);
    FindTopDocuments(search_server, "пушистый --кот"s);
    FindTopDocuments(search_server, "пушистый -"s);
    FindTopDocuments(search_server, "скво\x12рец"s);
    
    MatchDocuments(search_server, "пушистый пёс"s);
    MatchDocuments(search_server, "модный -кот"s);
    MatchDocuments(search_server, "модный --пёс"s);
    MatchDocuments(search_server, "пушистый - хвост"s);
}
