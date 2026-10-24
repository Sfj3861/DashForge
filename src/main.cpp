#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "Generator.hpp"

using namespace geode::prelude;

class $modify(DashForgeMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) {
            log::warn("DashForge: bottom-menu node not found; skipping entry button.");
            return true;
        }
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png");
        if (!sprite) sprite = CCSprite::create();
        if (sprite) {
            auto button = CCMenuItemSpriteExtra::create(
                sprite, this, menu_selector(DashForgeMenuLayer::onDashForge));
            if (button) {
                button->setID("dashforge-entry"_spr);
                menu->addChild(button);
                menu->updateLayout();
            }
        }
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
            "\n\nThis build creates an internal event plan only. It does not yet place Geometry Dash objects or save/import a playable level. That integration is the next milestone.";
        FLAlertLayer::create("DashForge - Prototype", message, "OK")->show();
    }
};
