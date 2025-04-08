#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <cstdint>
#include <cassert>
#include <numeric>
#include <limits>

struct Weight {
    char name;
    int weight;
};

class AliasMethod {
private:
    std::vector<Weight> table;
    std::vector<std::size_t> alias;
    std::vector<double> prob;
    std::mt19937 gen;

public:
    explicit AliasMethod(const std::vector<Weight>& entries)
        : gen(std::random_device{}()) {

        if (entries.empty()) {
            throw std::invalid_argument("AliasMethod: input weight table is empty.");
        }

        table = entries;
        const std::size_t N = table.size();
        alias.resize(N);
        prob.resize(N);

        std::uint64_t totalWeight = 0;
        for (const auto& entry : table) {
            if (entry.weight <= 0) {
                throw std::invalid_argument("AliasMethod: weight must be positive.");
            }
            totalWeight += static_cast<std::uint64_t>(entry.weight);
        }

        std::vector<double> normalized(N);
        for (std::size_t i = 0; i < N; ++i) {
            normalized[i] = static_cast<double>(table[i].weight) / static_cast<double>(totalWeight) * static_cast<double>(N);
        }

        std::vector<std::size_t> small;
        std::vector<std::size_t> large;
        small.reserve(N);
        large.reserve(N);

        for (std::size_t i = 0; i < N; ++i) {
            if (normalized[i] < 1.0)
                small.push_back(i);
            else
                large.push_back(i);
        }

        while (!small.empty() && !large.empty()) {
            std::size_t smallIdx = small.back(); small.pop_back();
            std::size_t largeIdx = large.back(); large.pop_back();

            prob[smallIdx] = std::clamp(normalized[smallIdx], 0.0, 1.0);
            alias[smallIdx] = largeIdx;

            normalized[largeIdx] = (normalized[largeIdx] + normalized[smallIdx]) - 1.0;

            if (normalized[largeIdx] < 1.0)
                small.push_back(largeIdx);
            else
                large.push_back(largeIdx);
        }

        for (auto idx : small) prob[idx] = 1.0;
        for (auto idx : large) prob[idx] = 1.0;

        for (std::size_t i = 0; i < alias.size(); ++i) {
            if (alias[i] >= N) {
                alias[i] = i; // Fallback to self to prevent UB
            }
        }
    }

    char pick() {
        if (table.empty()) {
            throw std::runtime_error("AliasMethod: Cannot pick from empty table.");
        }

        std::uniform_int_distribution<std::size_t> colDist(0, prob.size() - 1);
        std::uniform_real_distribution<double> probDist(0.0, 1.0);

        std::size_t col = colDist(gen);
        double p = probDist(gen);

        assert(col < prob.size() && col < alias.size() && col < table.size());
        std::size_t chosenIndex = (p < prob[col]) ? col : alias[col];

        if (chosenIndex >= table.size()) {
            throw std::runtime_error("AliasMethod: Chosen index out of bounds.");
        }

        return table[chosenIndex].name;
    }
};


int main() {
    try {
        std::vector<Weight> table = { {'A', 4}, {'B', 1}, {'C', 3}, {'D', 2} };

        AliasMethod picker(table);

        constexpr int trials = 10000;
        std::map<char, int> frequency;

        for (int i = 0; i < trials; ++i) {
            char selected = picker.pick();
            frequency[selected]++;
        }

        for (const auto& [key, count] : frequency) {
            std::cout << key << " was selected " << count << " times ("
                      << (100.0 * count / trials) << "%)\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}
