#include "hook.hpp"

class $modify(LoadingLayer) {
    bool init(bool fromReload) {
        if (!$LoadingLayer::init(fromReload))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto [count, unresolvedCount] = Loader::get()->getLoadedModCount();

        const char* text = unresolvedCount ?
            CCString::createWithFormat("Geode: Loaded %d mods (%d unresolved)", count, unresolvedCount)->getCString() : 
            CCString::createWithFormat("Geode: Loaded %d mods", count)->getCString();

        auto label = CCLabelBMFont::create(text, "goldFont.fnt");
        label->setPosition(winSize.width / 2, 30.f);
        label->setScale(.45f);
        this->addChild(label);

        return true;
    }
};
