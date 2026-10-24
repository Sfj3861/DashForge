#include "Corpus.hpp"
#include <sstream>

namespace dashforge {
namespace {
// Conservative starter catalogue. Unknown IDs/creators/cohorts are not guessed.
// Metadata candidates are not ready for training until checked and native data is obtained.
const std::vector<CorpusLevel> kMandatoryCorpus = {
    {"SuperSonic", "unknown", "2.0 (provisional)", "Insane Demon (provisional)", "", "needs-verification",
     "Mandatory example; verify creator, exact ID, rating, release era and native level string."},
    {"Sonic Wave", "unknown", "2.0 (provisional)", "Extreme Demon (provisional)", "", "needs-verification",
     "Mandatory example; verify creator, exact ID, rating, release era and native level string."},
    {"Nine Circles", "unknown", "1.9 (provisional)", "Hard Demon (provisional)", "", "needs-verification",
     "Mandatory example; verify exact identity, rating, release era and native level string."},
    {"Bloodbath", "Riot (provisional)", "1.9 (provisional)", "Extreme Demon (provisional)", "10565740", "metadata-candidate",
     "Dataset seed ID; re-check identity, creator, rating and era against authoritative sources."},
    {"Sonar", "unknown", "unverified", "unverified", "4454123", "metadata-candidate",
     "Dataset seed ID only; verify identity, creator, difficulty and historical era."},
    {"Level Easy", "Cody (dataset attribution)", "1.0 (provisional)", "Easy (provisional)", "11940", "metadata-candidate",
     "Dataset seed ID; confirm historical version and retrieve native level data."},
    {"Future Funk", "JonathanGD (provisional)", "2.1 (provisional)", "Hard Demon (provisional)", "", "needs-verification",
     "Mandatory example; verify exact ID, rating, release era and native level string."},
    {"ReTraY", "unknown", "unverified", "unverified", "6508283", "metadata-candidate",
     "Dataset seed ID only; verify identity, creator, rating and historical era."},
    {"Chill Out", "Darnoc", "1.0 (requested historical target)", "unverified", "", "needs-verification",
     "Mandatory historical example; confirm exact identity and obtain native level data."}
};
}

std::vector<CorpusLevel> const& mandatoryCorpus() {
    return kMandatoryCorpus;
}

CorpusStatus getCorpusStatus() {
    CorpusStatus status{};
    status.targetSlots = 1820;
    status.mandatoryEntries = kMandatoryCorpus.size();
    for (auto const& level : kMandatoryCorpus) {
        if (std::string(level.status) == "verified") ++status.verifiedEntries;
        if (std::string(level.status) == "metadata-candidate") ++status.metadataCandidates;
        if (std::string(level.status) == "native-data-verified") ++status.nativeDataEntries;
    }
    std::ostringstream out;
    out << "Community corpus: " << status.targetSlots << " target slots (13 eras x 14 cohorts x 10 levels). "
        << status.mandatoryEntries << " mandatory seed records are tracked in this build; "
        << status.verifiedEntries << " fully verified, " << status.metadataCandidates
        << " metadata candidates, " << status.nativeDataEntries << " with verified native data. "
        << "The 1,820-level corpus is not yet complete.";
    status.summary = out.str();
    return status;
}

std::string makeCorpusReport() {
    auto status = getCorpusStatus();
    std::ostringstream out;
    out << status.summary << "\n\nMandatory seed catalogue:\n";
    for (auto const& level : mandatoryCorpus()) {
        out << "- " << level.name << " | " << level.era << " | " << level.cohort
            << " | ID: " << (level.levelId[0] ? level.levelId : "pending")
            << " | " << level.status << "\n";
    }
    out << "\nImportant: metadata is not native level data. This catalogue does not train the generator yet. "
        << "Each entry must be checked against an authoritative source and its native level string must be obtained "
        << "before it can be used for object-placement learning. Historical era/category labels marked provisional "
        << "must be verified before corpus selection.";
    return out.str();
}
} // namespace dashforge
