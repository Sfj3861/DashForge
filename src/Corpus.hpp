#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace dashforge {
struct CorpusLevel {
    const char* name;
    const char* creator;
    const char* era;
    const char* cohort;
    const char* levelId;
    const char* status;
    const char* note;
};

struct CorpusStatus {
    std::size_t targetSlots;
    std::size_t verifiedEntries;
    std::size_t metadataCandidates;
    std::size_t nativeDataEntries;
    std::size_t mandatoryEntries;
    std::string summary;
};

std::vector<CorpusLevel> const& mandatoryCorpus();
CorpusStatus getCorpusStatus();
std::string makeCorpusReport();
} // namespace dashforge
