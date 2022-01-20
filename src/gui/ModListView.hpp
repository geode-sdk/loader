#pragma once

#include <Geode>

USE_GEODE_NAMESPACE();

static constexpr const BoomListType kBoomListType_Mod
    = static_cast<BoomListType>(0x350);

// Wrapper so you can pass Mods in a CCArray
struct ModObject : public CCObject {
    Mod* m_pMod;
    inline ModObject(Mod* mod) : m_pMod(mod) {
        this->autorelease();
    };
};

class ModListView;

class ModCell : public TableViewCell {
    protected:
        ModListView* m_pList;
        Mod* m_pMod;
        CCMenuItemToggler* m_pEnableToggle;
        CCMenuItemSpriteExtra* m_pUnresolvedExMark;

		ModCell(const char* name, CCSize size);

        void draw() override;
        void onInfo(CCObject*);
        void onEnable(CCObject*);
	    void onUnresolvedInfo(CCObject*);

        bool init(ModListView* list);

	public:
        void loadFromMod(ModObject*);

        void updateState(bool invert = false);

		static ModCell* create(ModListView* list, const char* key, CCSize size);
};

class ModListView : public CustomListView {
    protected:
        void setupList() override;
        TableViewCell* getListCell(const char* key) override;
        void loadCell(TableViewCell* cell, unsigned int index) override;
    
    public:
        static ModListView* create(CCArray* mods);

        void updateAllStates(ModCell* toggled = nullptr);
};
