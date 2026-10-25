#pragma once
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace dashforge {
struct LearnedObjectStats {
    int objectId = 0;
    std::uint64_t count = 0;
    float meanDx = 30.f;
    float meanY = 0.f;
};
struct LearnedTransition {
    int nextObjectId = 0;
    std::uint64_t count = 0;
};

class LearnedModel {
public:
    bool load(std::string const& path);
    bool isReady() const { return !objects.empty() && !transitions.empty(); }
    std::size_t objectTypeCount() const { return objects.size(); }
    int sampleFirst(std::mt19937& rng) const;
    int sampleNext(int previousObjectId, std::mt19937& rng) const;
    LearnedObjectStats const* statsFor(int objectId) const;

private:
    std::unordered_map<int, LearnedObjectStats> objects;
    std::unordered_map<int, std::vector<LearnedTransition>> transitions;
    std::vector<int> globalIds;
};
} // namespace dashforge
