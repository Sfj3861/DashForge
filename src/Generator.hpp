#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace dashforge {
enum class EventType { Ground, Spike, DoubleSpike, PlatformGap, Orb, DecorationMarker };
struct LayoutEvent { EventType type; float x; float y; float width; };
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
};
// Creates a reproducible event plan only; no GD GameObjects or saved level yet.
GeneratedLayout generateLayout(GeneratorOptions const& options);
} // namespace dashforge
