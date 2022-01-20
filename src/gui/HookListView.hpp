#pragma once

#include <Geode>

USE_GEODE_NAMESPACE();

static constexpr const BoomListType kBoomListType_Hooks
    = static_cast<BoomListType>(0x358);

struct HookItem : public CCObject {
    Hook* m_pHook;

    HookItem(Hook* h) : m_pHook(h) {
        this->autorelease();
    }
};

class HookCell : public TableViewCell {
    protected:
        Mod* m_pMod;
        Hook* m_pHook;

		HookCell(const char* name, CCSize size);

        void draw() override;

        void onEnable(CCObject*);
	
	public:
        void loadFromHook(Hook*, Mod*);

		static HookCell* create(const char* key, CCSize size);
};

class HookListView : public CustomListView {
    protected:
        Mod* m_pMod;

        void setupList() override;
        TableViewCell* getListCell(const char* key) override;
        void loadCell(TableViewCell* cell, unsigned int index) override;
    
    public:
        static HookListView* create(
            CCArray* hooks, Mod* Mod, float width, float height
        );
};
