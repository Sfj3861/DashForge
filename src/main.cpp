#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "Generator.hpp"
#include "Corpus.hpp"

using namespace geode::prelude;

class $modify(DashForgeMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) {
            log::warn("DashForge: bottom-menu node not found; skipping entry buttons.");
            return true;
        }

        auto makeButton = [this, menu](char const* frame, char const* id, SEL_MenuHandler callback) {
            auto sprite = CCSprite::createWithSpriteFrameName(frame);
            if (!sprite) sprite = CCSprite::create();
            if (!sprite) return;
            auto button = CCMenuItemSpriteExtra::create(sprite, this, callback);
            if (!button) return;
            button->setID(id);
            menu->addChild(button);
        };

        makeButton("GJ_likeBtn_001.png", "dashforge-entry",
                   menu_selector(DashForgeMenuLayer::onDashForge));
        makeButton("GJ_searchBtn_001.png", "dashforge-corpus",
                   menu_selector(DashForgeMenuLayer::onDashForgeCorpus));
        menu->updateLayout();
        return true;
    }

    void onDashForge(CCObject*) {
        dashforge::GeneratorOptions options;
        options.seed = static_cast<std::uint32_t>(
            Mod::get()->getSettingValue<int64_t>("generation-seed"));
        options.durationSeconds = static_cast<unsigned>(
            Mod::get()->getSettingValue<int64_t>("layout-length"));
        options.structured = true;
        auto modelPath = (Mod::get()->getResourcesDir() / "dashforge-model.tsv").string();
        auto layout = dashforge::generateLayout(options, modelPath);
        std::string message = "DashForge's offline generator ran.\n\n" +
            layout.summary +
            "\n\nImportant: this is a learned event/object-ID plan, not yet a playable level. The mod still needs the editor integration that creates real GameObjects, validates them and saves a separate level.";
        FLAlertLayer::create("DashForge - Generator", message, "OK")->show();
    }

    void onDashForgeCorpus(CCObject*) {
        FLAlertLayer::create("DashForge - Level Corpus",
            dashforge::makeCorpusReport(), "OK")->show();
    }
};
