#include "ModListView.hpp"
#include "ModInfoLayer.hpp"

ModCell::ModCell(const char* name, CCSize size) :
    TableViewCell(name, size.width, size.height) {}

void ModCell::draw() {
    reinterpret_cast<StatsCell*>(this)->StatsCell::draw();
}

void ModCell::loadFromMod(ModObject* Mod) {
    this->m_pMod = Mod->m_pMod;

    this->m_pLayer->setVisible(true);
    this->m_pBGLayer->setOpacity(255);
    
    auto menu = CCMenu::create();
    menu->setPosition(this->m_fWidth - this->m_fHeight, this->m_fHeight / 2);
    this->m_pLayer->addChild(menu);

    auto titleLabel = CCLabelBMFont::create(
        this->m_pMod->getName().c_str(), "bigFont.fnt"
    );
    titleLabel->setAnchorPoint({ .0f, .5f });
    titleLabel->setScale(.5f);
    titleLabel->setPosition(this->m_fHeight / 2, this->m_fHeight / 2 + 7.f);
    this->m_pLayer->addChild(titleLabel);
    
    auto creatorStr = "by " + this->m_pMod->getDeveloper();
    auto creatorLabel = CCLabelBMFont::create(
        creatorStr.c_str(), "goldFont.fnt"
    );
    creatorLabel->setAnchorPoint({ .0f, .5f });
    creatorLabel->setScale(.43f);
    creatorLabel->setPosition(this->m_fHeight / 2, this->m_fHeight / 2 - 7.f);
    this->m_pLayer->addChild(creatorLabel);

    auto viewSpr = ButtonSprite::create(
        "View", 0, 0, "bigFont.fnt", "GJ_button_01.png", 0, .8f
    );
    viewSpr->setScale(.65f);

    auto viewBtn = CCMenuItemSpriteExtra::create(
        viewSpr, this, menu_selector(ModCell::onInfo)
    );
    menu->addChild(viewBtn);

    this->m_pEnableToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(ModCell::onEnable), .7f
    );
    this->m_pEnableToggle->setPosition(-50.f, 0.f);
    menu->addChild(this->m_pEnableToggle);

    auto exMark = CCSprite::createWithSpriteFrameName("exMark_001.png");
    exMark->setScale(.5f);

    this->m_pUnresolvedExMark = CCMenuItemSpriteExtra::create(
        exMark, this, menu_selector(ModCell::onUnresolvedInfo)
    );
    this->m_pUnresolvedExMark->setPosition(-80.f, 0.f);
    menu->addChild(this->m_pUnresolvedExMark);

    this->updateState();
}

void ModCell::onInfo(CCObject*) {
    ModInfoLayer::create(this->m_pMod)->show();
}

void ModCell::onEnable(CCObject* pSender) {
    if (!as<CCMenuItemToggler*>(pSender)->isToggled()) {
        auto res = this->m_pMod->enable();
        if (!res) {
            FLAlertLayer::create(
                nullptr,
                "Error Enabling Mod",
                "OK", nullptr,
                res.error()
            )->show();
        }
    } else {
        auto res = this->m_pMod->disable();
        if (!res) {
            FLAlertLayer::create(
                nullptr,
                "Error Disabling Mod",
                "OK", nullptr,
                res.error()
            )->show();
        }
    }
    this->m_pList->updateAllStates(this);
}

void ModCell::onUnresolvedInfo(CCObject* pSender) {
    std::string info = "This mod has the following <cr>unresolved dependencies</c>: ";
    for (auto const& dep : this->m_pMod->getUnresolvedDependencies()) {
        info += "<cg>" + dep.m_id + "</c> (<cy>" + dep.m_version.toString() + "</c>), ";
    }
    info.pop_back();
    info.pop_back();
    FLAlertLayer::create(
        nullptr,
        "Unresolved Dependencies",
        "OK", nullptr,
        400.f, info
    )->show();
}

bool ModCell::init(ModListView* list) {
    this->m_pList = list;
    return true;
}

void ModCell::updateState(bool invert) {
    this->m_pEnableToggle->toggle(this->m_pMod->isEnabled() ^ invert);

    bool unresolved = this->m_pMod->hasUnresolvedDependencies();
    this->m_pEnableToggle->setEnabled(!unresolved);
    this->m_pEnableToggle->m_pOffButton->setOpacity(unresolved ? 100 : 255);
    this->m_pEnableToggle->m_pOffButton->setColor(unresolved ? cc3x(155) : cc3x(255));
    this->m_pEnableToggle->m_pOnButton->setOpacity(unresolved ? 100 : 255);
    this->m_pEnableToggle->m_pOnButton->setColor(unresolved ? cc3x(155) : cc3x(255));

    this->m_pUnresolvedExMark->setVisible(unresolved);
}

ModCell* ModCell::create(ModListView* list, const char* key, CCSize size) {
    auto pRet = new ModCell(key, size);
    if (pRet && pRet->init(list)) {
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}


void ModListView::updateAllStates(ModCell* toggled) {
    CCARRAY_FOREACH_B_TYPE(this->m_pTableView->m_pCellArray, cell, ModCell) {
        cell->updateState(toggled == cell);
    }
}

void ModListView::setupList() {
    this->m_fItemSeparation = 40.0f;

    if (!this->m_pEntries->count()) return;

    this->m_pTableView->reloadData();

    if (this->m_pEntries->count() == 1)
        this->m_pTableView->moveToTopWithOffset(this->m_fItemSeparation);
    
    this->m_pTableView->moveToTop();
}

TableViewCell* ModListView::getListCell(const char* key) {
    return ModCell::create(this, key, { this->m_fWidth, this->m_fItemSeparation });
}

void ModListView::loadCell(TableViewCell* cell, unsigned int index) {
    as<ModCell*>(cell)->loadFromMod(
        as<ModObject*>(this->m_pEntries->objectAtIndex(index))
    );
    as<StatsCell*>(cell)->updateBGColor(index);
}

ModListView* ModListView::create(
    CCArray* actions
) {
    auto pRet = new ModListView;
    if (pRet) {
        if (pRet->init(actions, kBoomListType_Mod, 356.f, 220.f)) {
            pRet->autorelease();
            return pRet;
        }
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}
