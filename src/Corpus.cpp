#include "Corpus.hpp"
#include <sstream>

namespace dashforge {
namespace {
// Deliberately conservative starter catalogue. Unknown IDs/eras are not guessed.
// "metadata-only" means the identity is a research candidate, not training-ready.
const std::vector<CorpusLevel> kMandatoryCorpus = {
    {"SuperSonic", "unknown", "2.0", "Insane Demon", "", "needs-verification",
     "Mandatory example; verify creator, exact ID, rating and native level string."},
    {"Sonic Wave", "unknown", "2.0", "Extreme Demon", "", "needs-verification",
     "Mandatory example; verify creator, exact ID, rating and native level string."},
    {"Nine Circles", "unknown", "1.9", "Hard Demon", "", "needs-verification",
     "Mandatory example; verify exact level identity and native level string."},
    {"Bloodbath", "Riot", "1.9", "Extreme Demon", "10565740", "metadata-candidate",
     "Dataset seed ID; re-check against the live server and retrieve native level data."},
    {"Sonar", "unknown", "1.9", "N/A (reference)", "4454123", "metadata-candidate",
     "Dataset seed ID; confirm creator, actual release era and current rating."},
    {"Level Easy", "Cody", "1.0", "Easy", "11940", "metadata-candidate",
     "Dataset seed ID; confirm historical version and retrieve native level data."},
    {"Future Funk", "JonathanGD", "2.1", "Hard Demon", "", "needs-verification",
     "Mandatory example; verify exact ID and native level string."},
    {"ReTraY", "unknown", "2.2", "N/A (reference)", "6508283", "metadata-candidate",
     "Dataset seed ID; confirm actual release era and current rating."},
    {"Chill Out", "Darnoc", "1.0", "N/A (reference)", "", "needs-verification",
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
        << "before it can be used for object-placement learning. Historical era labels are provisional until verified.";
    return out.str();
}
} // namespace dashforge
