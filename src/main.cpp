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
        auto layout = dashforge::generateLayout(options);
        std::string message = "DashForge's offline prototype generator ran successfully.\n\n" +
            layout.summary +
            "\n\nThis build creates an internal event plan only. It does not yet place Geometry Dash objects or save/import a playable level.";
        FLAlertLayer::create("DashForge - Prototype", message, "OK")->show();
    }

    void onDashForgeCorpus(CCObject*) {
        FLAlertLayer::create("DashForge - Level Corpus",
            dashforge::makeCorpusReport(), "OK")->show();
    }
};
