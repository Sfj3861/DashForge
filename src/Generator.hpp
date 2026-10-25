#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace dashforge {
enum class EventType { Ground, Spike, DoubleSpike, PlatformGap, Orb, DecorationMarker, LearnedObject };
struct LayoutEvent {
    EventType type;
    float x;
    float y;
    float width;
    int objectId = 0; // Native GD object ID when produced by a trained model.
};
struct GeneratorOptions {
    std::uint32_t seed = 3861;
    unsigned durationSeconds = 30;
    unsigned objectsPerSecond = 3;
    bool structured = true;
    bool decoration = false;
};
struct GeneratedLayout {
    std::vector<LayoutEvent> events;
    unsigned durationSeconds = 0;
    std::string summary;
    bool learnedModelUsed = false;
    std::size_t learnedObjectTypes = 0;
};
// If modelPath points to a valid trained TSV model, object IDs and transitions
// are sampled from it. Otherwise the deterministic heuristic fallback is used.
// The output remains a plan; editor-object placement/export is a separate stage.
GeneratedLayout generateLayout(GeneratorOptions const& options, std::string const& modelPath = {});
} // namespace dashforge
