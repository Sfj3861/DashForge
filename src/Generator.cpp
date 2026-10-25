#include "Generator.hpp"
#include "LearnedModel.hpp"
#include <algorithm>
#include <random>
#include <sstream>

namespace dashforge {
GeneratedLayout generateLayout(GeneratorOptions const& input, std::string const& modelPath) {
    GeneratorOptions options = input;
    options.durationSeconds = std::clamp(options.durationSeconds, 5u, 180u);
    options.objectsPerSecond = std::clamp(options.objectsPerSecond, 1u, 8u);
    GeneratedLayout result;
    result.durationSeconds = options.durationSeconds;
    std::mt19937 rng(options.seed);
    float x = 0.f;
    const float targetWidth = static_cast<float>(options.durationSeconds) * 60.f;
    const unsigned targetEvents = options.durationSeconds * options.objectsPerSecond;
    result.events.reserve(targetEvents);

    LearnedModel model;
    result.learnedModelUsed = !modelPath.empty() && model.load(modelPath);
    result.learnedObjectTypes = result.learnedModelUsed ? model.objectTypeCount() : 0;

    if (result.learnedModelUsed) {
        int previousId = 0;
        for (unsigned i = 0; i < targetEvents && x < targetWidth; ++i) {
            int objectId = previousId == 0 ? model.sampleFirst(rng) : model.sampleNext(previousId, rng);
            if (objectId <= 0) break;
            auto stats = model.statsFor(objectId);
            if (!stats) {
                previousId = objectId;
                continue;
            }
            float width = std::clamp(stats->meanDx, 15.f, 120.f);
            float y = std::clamp(stats->meanY, -120.f, 300.f);
            result.events.push_back({EventType::LearnedObject, x, y, width, objectId});
            x += width;
            previousId = objectId;
            if (options.decoration && i % 4 == 0)
                result.events.push_back({EventType::DecorationMarker, x, 45.f, 12.f, 0});
        }
    } else {
        std::uniform_int_distribution<int> choice(0, 99);
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
            result.events.push_back({type, x, 0.f, width, 0});
            x += width + (options.structured ? 15.f : 8.f);
            if (options.decoration && i % 4 == 0)
                result.events.push_back({EventType::DecorationMarker, x, 45.f, 12.f, 0});
        }
    }

    std::ostringstream summary;
    summary << result.events.size() << " prototype events; " << result.durationSeconds
            << "s target; seed " << options.seed << ". ";
    if (result.learnedModelUsed)
        summary << "Learned model active: " << result.learnedObjectTypes
                << " native object IDs; object transitions sampled from training data.";
    else
        summary << "No trained model found; heuristic fallback used.";
    result.summary = summary.str();
    return result;
}
} // namespace dashforge
