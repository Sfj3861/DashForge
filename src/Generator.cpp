#include "Generator.hpp"
#include <algorithm>
#include <random>
#include <sstream>

namespace dashforge {
GeneratedLayout generateLayout(GeneratorOptions const& input) {
    GeneratorOptions options = input;
    options.durationSeconds = std::clamp(options.durationSeconds, 5u, 180u);
    options.objectsPerSecond = std::clamp(options.objectsPerSecond, 1u, 8u);
    GeneratedLayout result;
    result.durationSeconds = options.durationSeconds;
    std::mt19937 rng(options.seed);
    std::uniform_int_distribution<int> choice(0, 99);
    float x = 0.f;
    const float targetWidth = static_cast<float>(options.durationSeconds) * 60.f;
    const unsigned targetEvents = options.durationSeconds * options.objectsPerSecond;
    result.events.reserve(targetEvents);

    for (unsigned i = 0; i < targetEvents && x < targetWidth; ++i) {
        int roll = choice(rng);
        EventType type = EventType::Ground;
        float width = 30.f;
        if (options.structured) {
            if (roll < 12) type = EventType::PlatformGap, width = 60.f;
            else if (roll < 35) type = EventType::Spike;
            else if (roll < 43) type = EventType::DoubleSpike, width = 45.f;
            else if (roll < 57) type = EventType::Orb;
        } else {
            if (roll < 18) type = EventType::PlatformGap, width = 60.f;
            else if (roll < 48) type = EventType::Spike;
            else if (roll < 58) type = EventType::DoubleSpike, width = 45.f;
            else if (roll < 72) type = EventType::Orb;
        }
        if (!result.events.empty()) {
            auto previous = result.events.back().type;
            if (type == EventType::PlatformGap && previous == EventType::PlatformGap)
                type = EventType::Ground, width = 30.f;
            if ((type == EventType::Spike || type == EventType::DoubleSpike) &&
                (previous == EventType::Spike || previous == EventType::DoubleSpike) && i % 3 != 0)
                type = EventType::Ground, width = 30.f;
        }
        result.events.push_back({type, x, 0.f, width});
        x += width + (options.structured ? 15.f : 8.f);
        if (options.decoration && i % 4 == 0)
            result.events.push_back({EventType::DecorationMarker, x, 45.f, 12.f});
    }
    std::ostringstream summary;
    summary << result.events.size() << " prototype events; " << result.durationSeconds
            << "s target; seed " << options.seed << ".";
    result.summary = summary.str();
    return result;
}
} // namespace dashforge
