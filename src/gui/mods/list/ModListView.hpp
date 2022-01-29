#pragma once

#include <Geode>

USE_GEODE_NAMESPACE();

static constexpr const BoomListType kBoomListType_Mod
    = static_cast<BoomListType>(0x350);

// Wrapper so you can pass Mods in a CCArray
struct ModObject : public CCObject {
    Mod* m_mod;
    inline ModObject(Mod* mod) : m_mod(mod) {
        this->autorelease();
    };
};

class ModListView;

class ModCell : public TableViewCell {
    protected:
        ModListView* m_list;
        Mod* m_mod;
        CCMenuItemToggler* m_enableToggle = nullptr;
        CCMenuItemSpriteExtra* m_unresolvedExMark;

		ModCell(const char* name, CCSize size);

        void draw() override;
        void onInfo(CCObject*);
        void onEnable(CCObject*);
	    void onUnresolvedInfo(CCObject*);

        bool init(ModListView* list);

	public:
		void updateBGColor(int index);
		
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
