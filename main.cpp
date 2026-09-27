#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::vector<std::string> tokens(const std::string& text) {
    std::vector<std::string> found;
    std::string word;
    for (unsigned char ch : text) {
        if (ch < 128 && std::isalnum(ch)) {
            word += static_cast<char>(std::tolower(ch));
        } else if (!word.empty()) {
            found.push_back(word);
            word.clear();
        }
    }
    if (!word.empty()) found.push_back(word);
    return found;
}

void run(const fs::path& root, const std::string& query) {
    if (!fs::is_directory(root)) throw std::runtime_error("corpus directory does not exist");
    const auto query_words = tokens(query);
    if (query_words.empty()) throw std::runtime_error("query needs an ASCII letter or digit");
    std::set<std::string> terms(query_words.begin(), query_words.end());
    std::vector<fs::path> files;
    for (const auto& item : fs::directory_iterator(root)) {
        if (!item.is_symlink() && item.is_regular_file() && item.path().extension() == ".txt") {
            const auto name = item.path().filename().string();
            if (name.find_first_of(",\"\r\n") != std::string::npos)
                throw std::runtime_error("document filename cannot contain CSV punctuation");
            files.push_back(item.path());
        }
    }
    if (files.empty()) throw std::runtime_error("corpus directory has no .txt files");
    if (files.size() > 10000) throw std::runtime_error("corpus has too many documents");
    std::sort(files.begin(), files.end());
    std::map<std::string, std::map<std::string, int>> index;
    for (const auto& file : files) {
        if (fs::file_size(file) > 10000000) throw std::runtime_error("document exceeds 10 MB: " + file.filename().string());
        std::ifstream in(file, std::ios::binary);
        if (!in) throw std::runtime_error("cannot read document: " + file.filename().string());
        const std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        if (!in.eof() && in.fail()) throw std::runtime_error("error reading document");
        const std::string name = file.filename().string();
        for (const auto& word : tokens(data)) ++index[word][name];
    }
    std::map<std::string, long long> scores;
    for (const auto& term : terms) {
        const auto found = index.find(term);
        if (found != index.end()) for (const auto& [name, count] : found->second) scores[name] += count;
    }
    std::vector<std::pair<std::string, long long>> ranked(scores.begin(), scores.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
    std::cout << "document,score\n";
    for (const auto& [name, score] : ranked) std::cout << name << ',' << score << '\n';
}

int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::runtime_error("usage: search <corpus-directory> <query>");
        run(argv[1], argv[2]);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
