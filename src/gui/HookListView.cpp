#include "HookListView.hpp"

HookCell::HookCell(const char* name, CCSize size) :
    TableViewCell(name, size.width, size.height) {}

void HookCell::draw() {
    reinterpret_cast<StatsCell*>(this)->StatsCell::draw();
}

void HookCell::onEnable(CCObject* pSender) {
    auto toggle = as<CCMenuItemToggler*>(pSender);
    if (!toggle->isToggled()) {
        auto res = this->m_mod->enableHook(this->m_hook);
        if (!res) {
            FLAlertLayer::create(
                nullptr, "Error Enabling Hook",
                "OK", nullptr,
                280.f,
                std::string(res.error())
            )->show();
        }
    } else {
        auto res = this->m_mod->disableHook(this->m_hook);
        if (!res) {
            FLAlertLayer::create(
                nullptr, "Error Disabling Hook",
                "OK", nullptr,
                280.f,
                std::string(res.error())
            )->show();
        }
    }
    toggle->toggle(!this->m_hook->isEnabled());
}

void HookCell::loadFromHook(Hook* hook, Mod* Mod) {
    this->m_hook = hook;
    this->m_mod = Mod;

    this->m_mainLayer->setVisible(true);
    this->m_BGLayer->setOpacity(255);
    
    auto menu = CCMenu::create();
    menu->setPosition(this->m_width - this->m_height, this->m_height / 2);
    this->m_mainLayer->addChild(menu);

    auto enableBtn = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(HookCell::onEnable), .6f
    );
    enableBtn->setPosition(0, 0);
    enableBtn->toggle(hook->isEnabled());
    menu->addChild(enableBtn);

    std::stringstream moduleName;
    auto addr = hook->getAddress();

    HMODULE module;
    if (GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        as<LPCSTR>(hook->getAddress()),
        &module
    )) {
        addr -= as<uintptr_t>(module);
        char name[MAX_PATH];
        if (GetModuleFileNameA(
            module, name, sizeof name
        )) {
            auto fileName = std::filesystem::path(name).stem();
            moduleName << fileName.string() << " + ";
        }
    }

    moduleName << "0x" << std::hex << addr;
    auto label = CCLabelBMFont::create(moduleName.str().c_str(), "bigFont.fnt");
    label->setPosition(this->m_height / 2, this->m_height / 2);
    label->setScale(.4f);
    label->setAnchorPoint({ .0f, .5f });
    this->m_mainLayer->addChild(label);
}

HookCell* HookCell::create(const char* key, CCSize size) {
    auto pRet = new HookCell(key, size);
    if (pRet) {
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}


void HookListView::setupList() {
    this->m_fItemSeparation = 30.0f;

    if (!this->m_pEntries->count()) return;

    this->m_pTableView->reloadData();

    if (this->m_pEntries->count() == 1)
        this->m_pTableView->moveToTopWithOffset(this->m_fItemSeparation);
    
    this->m_pTableView->moveToTop();
}

TableViewCell* HookListView::getListCell(const char* key) {
    return HookCell::create(key, { this->m_width, this->m_fItemSeparation });
}

void HookListView::loadCell(TableViewCell* cell, unsigned int index) {
    as<HookCell*>(cell)->loadFromHook(
        as<HookItem*>(this->m_pEntries->objectAtIndex(index))->m_hook,
        this->m_mod
    );
    as<StatsCell*>(cell)->updateBGColor(index);
}

HookListView* HookListView::create(
    CCArray* hooks, Mod* Mod,
    float width, float height
) {
    auto pRet = new HookListView;
    if (pRet) {
        pRet->m_mod = Mod;
        if (pRet->init(hooks, kBoomListType_Hooks, width, height)) {
            pRet->autorelease();
            return pRet;
        }
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}


