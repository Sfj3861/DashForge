#include "LearnedModel.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace dashforge {
bool LearnedModel::load(std::string const& path) {
    objects.clear();
    transitions.clear();
    globalIds.clear();
    std::ifstream input(path);
    if (!input) return false;

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream row(line);
        char kind = 0;
        row >> kind;
        if (kind == 'O') {
            LearnedObjectStats item;
            row >> item.objectId >> item.count >> item.meanDx >> item.meanY;
            if (!row || item.objectId <= 0 || item.count == 0) continue;
            item.meanDx = std::clamp(item.meanDx, 15.f, 120.f);
            item.meanY = std::clamp(item.meanY, -120.f, 300.f);
            objects[item.objectId] = item;
        } else if (kind == 'T') {
            int previous = 0;
            LearnedTransition transition;
            row >> previous >> transition.nextObjectId >> transition.count;
            if (!row || previous <= 0 || transition.nextObjectId <= 0 || transition.count == 0) continue;
            transitions[previous].push_back(transition);
        }
    }
    globalIds.reserve(objects.size());
    for (auto const& [id, item] : objects) {
        (void)item;
        globalIds.push_back(id);
    }
    std::sort(globalIds.begin(), globalIds.end());
    return isReady();
}

int LearnedModel::sampleFirst(std::mt19937& rng) const {
    if (globalIds.empty()) return 0;
    std::uint64_t total = 0;
    for (int id : globalIds) total += objects.at(id).count;
    if (!total) return globalIds.front();
    std::uniform_int_distribution<std::uint64_t> pick(1, total);
    auto target = pick(rng);
    for (int id : globalIds) {
        auto count = objects.at(id).count;
        if (target <= count) return id;
        target -= count;
    }
    return globalIds.back();
}

int LearnedModel::sampleNext(int previousObjectId, std::mt19937& rng) const {
    auto found = transitions.find(previousObjectId);
    if (found == transitions.end() || found->second.empty()) return sampleFirst(rng);
    std::uint64_t total = 0;
    for (auto const& edge : found->second) total += edge.count;
    if (!total) return sampleFirst(rng);
    std::uniform_int_distribution<std::uint64_t> pick(1, total);
    auto target = pick(rng);
    for (auto const& edge : found->second) {
        if (target <= edge.count) return edge.nextObjectId;
        target -= edge.count;
    }
    return found->second.back().nextObjectId;
}

LearnedObjectStats const* LearnedModel::statsFor(int objectId) const {
    auto found = objects.find(objectId);
    return found == objects.end() ? nullptr : &found->second;
}
} // namespace dashforge
