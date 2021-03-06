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

// logging functionality for containers
// log pair
template<typename First, typename Second>
std::ostream& operator<<(std::ostream& out, const std::pair<First, Second>& container) {
    out << container.first << ": "s << container.second;
    
    return out;
}

template<typename Container>
void Print(std::ostream& out, const Container& container) {
    bool isFirst = true;
    
    for (const auto& element : container) {
        if(isFirst) {
            out << element;
            
            isFirst = false;
            
            continue;
        }
        
        out << ", "s << element;
    }
}

// log vector
template<typename Element>
std::ostream& operator<<(std::ostream& out, const std::vector<Element>& container) {
    out << '[';
    
    Print(out, container);
    
    out << ']';
    
    return out;
}

// log set
template<typename Element>
std::ostream& operator<<(std::ostream& out, const std::set<Element>& container) {
    out << '{';
    
    Print(out, container);
    
    out << '}';
    
    return out;
}

// log map
template<typename Key, typename Value>
std::ostream& operator<<(std::ostream& out, const std::map<Key, Value>& container) {
    out << '{';
    
    Print(out, container);
    
    out << '}';
    
    return out;
}

// log DocumentStatus
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

// testing framework
template <typename TestFunction>
void RunTestImplementation(TestFunction test_function, const std::string& function_name) {
    test_function();
    
    std::cerr << function_name << " OK\n"s;
}

template <typename T, typename U>
void AssertEqualImplementation(const T& t, const U& u, const std::string& t_str, const std::string& u_str,
                               const std::string& file, const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cerr << std::boolalpha;
        
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        
        std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        
        std::cerr << t << " != "s << u << "."s;
        
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        
        std::cerr << std::endl;
        
        abort();
    }
}

void AssertImplementation(bool value, const std::string& expr_str, const std::string& file,
                          const std::string& func, unsigned line, const std::string& hint) {
    if (!value) {
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        
        std::cerr << "ASSERT("s << expr_str << ") failed."s;
        
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        
        std::cerr << std::endl;
        
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImplementation((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImplementation((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT(expression) AssertImplementation((expression), #expression, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expression, hint) AssertImplementation((expression), #expression, __FILE__, __FUNCTION__, __LINE__, (hint))

#define RUN_TEST(test_function) RunTestImplementation((test_function), #test_function)

void TestExcludeStopWords() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(doc_id, content, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("in"s);
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        
        ASSERT_EQUAL(found_docs[0].id, doc_id);
    }
    
    {
        SearchServer server;
        
        server.SetStopWords("in the"s);
        
        server.AddDocument(doc_id, content, DocumentStatus::kActual, ratings);
        
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

void TestAddedDocumentsCanBeFound() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(doc_id, content, DocumentStatus::kActual, ratings);
        
        const auto& found_docs = server.FindTopDocuments("cat in the city");
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].id, doc_id);
    }
    
    {
        SearchServer server;
        
        server.AddDocument(doc_id, "", DocumentStatus::kActual, ratings);
        
        const auto& found_docs = server.FindTopDocuments("cat");
        
        ASSERT(found_docs.empty());
    }
}

void TestMinusWordsExcludeDocuments() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("-cat");
        
        ASSERT(found_docs.empty());
    }
    
    // minus words dont exclude documents where they dont appear
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(43, "happy dog"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("-cat dog");
        
        ASSERT(found_docs.size() == 1);
    }
}

void TestMatchDocument() {
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        
        const auto [words, status] = server.MatchDocument("fat cat out of city", 42);
        
        std::vector<std::string> desired_matched_words{"cat", "city"};
        
        ASSERT_EQUAL(words, desired_matched_words);
        ASSERT_EQUAL(status, DocumentStatus::kActual);
    }
    
    // status banned
    {
        SearchServer server;
        
        server.AddDocument(42, "cat in the city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(43, "happy dog"s, DocumentStatus::kBanned, ratings);
        
        const auto [words, status] = server.MatchDocument("fat cat out of city and a cute dog", 43);
        
        std::vector<std::string> desired_matched_words{"dog"};
        
        ASSERT_EQUAL(words, desired_matched_words);
        ASSERT_EQUAL(status, DocumentStatus::kBanned);
    }
}

void TestFoundDocumentsAreSortedByRelevance() {
    constexpr double kAccuracy = 1e-6;
    
    const std::vector<int> ratings = {1, 2, 3};
    
    {
        SearchServer server;
        
        server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(2, "dog city potato"s, DocumentStatus::kActual, ratings);
        server.AddDocument(3, "dog city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(4, "lorem ipsum"s, DocumentStatus::kActual, ratings);
        server.AddDocument(5, "city"s, DocumentStatus::kBanned, ratings); // kBanned
        server.AddDocument(6, "frog city"s, DocumentStatus::kActual, ratings);
        server.AddDocument(7, "the cat says meow to dog"s, DocumentStatus::kActual, ratings);
        
        const auto found_docs = server.FindTopDocuments("dog in the city");
        
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
        
        const auto found_docs = server.FindTopDocuments("cat loves NY city");
        
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

void TestRatingCalculation() {
    const int doc_id = 1;
    const std::string content = "cat city"s;
    
    // positive ratings
    {
        SearchServer server;
        
        server.AddDocument(doc_id, content, DocumentStatus::kActual, {1, 2, 3});
        
        const auto found_docs = server.FindTopDocuments("cat loves NY city");
        
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].rating, 2);
    }
    
    // negative ratings
    {
        SearchServer server;
        
        server.AddDocument(doc_id, content, DocumentStatus::kActual, {-1, -2, -3});
        
        const auto found_docs = server.FindTopDocuments("cat loves NY city");
        
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
        const auto& filtered_docs = server.FindTopDocuments("city", [](int , DocumentStatus status, int ) {
            return status == DocumentStatus::kActual;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 1);
    }
    
    // rating predicate
    {
        const auto& filtered_docs = server.FindTopDocuments("city", [](int , DocumentStatus , int rating) {
            return rating == 2;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 3u); // only a doc with higher rating is found
        ASSERT_EQUAL(filtered_docs[0].rating, 2);
    }
    
    // id predicate
    {
        const int document_id_to_search_for = 1;
        
        const auto& filtered_docs = server.FindTopDocuments("city", [](int document_id, DocumentStatus , int ) {
            return document_id == document_id_to_search_for;
        });
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, document_id_to_search_for);
    }
}

void TestFilteringByStatus() {
    const std::vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    
    server.AddDocument(1, "cat city"s, DocumentStatus::kActual, ratings);
    server.AddDocument(2, "dog city potato"s, DocumentStatus::kBanned, ratings);
    
    // explicit status
    {
        const auto& filtered_docs = server.FindTopDocuments("city", DocumentStatus::kBanned);
        
        ASSERT_EQUAL(filtered_docs.size(), 1u);
        ASSERT_EQUAL(filtered_docs[0].id, 2);
    }
    
    // default status
    {
        const auto& filtered_docs = server.FindTopDocuments("city");
        
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
        const auto& found_docs = server.FindTopDocuments("cat"s);
        
        // log(documents_count * 1.0 / a) * (b / c)
        double expected_relevance_doc_0 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (2.0 / 4.0);
        double expected_relevance_doc_2 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (1.0 / 3.0);
        
        ASSERT_EQUAL(found_docs.size(), 2u);
        
        ASSERT(std::abs(found_docs[0].relevance - expected_relevance_doc_0) < kAccuracy);
        ASSERT(std::abs(found_docs[1].relevance - expected_relevance_doc_2) < kAccuracy);
    }
    
    // zero relevance
    {
        const auto& found_docs = server.FindTopDocuments("city"s);
        
        ASSERT_EQUAL(found_docs.size(), 3u);
        
        ASSERT(std::abs(found_docs[0].relevance) < kAccuracy);
        ASSERT(std::abs(found_docs[1].relevance) < kAccuracy);
        ASSERT(std::abs(found_docs[2].relevance) < kAccuracy);
    }
}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWords);
    RUN_TEST(TestAddedDocumentsCanBeFound);
    RUN_TEST(TestMinusWordsExcludeDocuments);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestFoundDocumentsAreSortedByRelevance);
    RUN_TEST(TestRatingCalculation);
    RUN_TEST(TestFilteringByPredicate);
    RUN_TEST(TestFilteringByStatus);
    RUN_TEST(TestRelevanceCalculation);
}

int main() {
    TestSearchServer();
    
    std::cout << "Search server testing finished"s << std::endl;
}
