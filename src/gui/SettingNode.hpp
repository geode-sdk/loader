#pragma once

#pragma warning(disable: 4067)

#include <Geode>

USE_GEODE_NAMESPACE();

template<class SettingClass>
class GeodeSettingNode : public TableViewCell {
protected:
	SettingClass* m_setting;
	CCMenu* m_buttonMenu;
	CCLabelBMFont* m_nameLabel;

	bool init(SettingClass* setting) {
		if (!CCNode::init())
			return false;
		
		m_setting = setting;

		this->setContentSize({ m_width, m_height });

		auto text = setting->getName().size() ? setting->getName() : setting->getKey();
		m_nameLabel = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
		m_nameLabel->setAnchorPoint({ .0f, .5f });
		m_nameLabel->setPosition(m_height / 2, m_height / 2);
		m_nameLabel->setScale(.5f);
		this->addChild(m_nameLabel);

		m_buttonMenu = CCMenu::create();
		m_buttonMenu->setPosition(m_width - m_height / 2, m_height / 2);
		this->addChild(m_buttonMenu);

		CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);
		this->registerWithTouchDispatcher();

		return true;
	}

	~GeodeSettingNode() {
		CCDirector::sharedDirector()->getTouchDispatcher()->decrementForcePrio(2);
	}

	GeodeSettingNode(float width, float height) : TableViewCell("cum", width, height) {}
};

class BoolSettingNode : public GeodeSettingNode<BoolSetting> {
protected:
	bool init(BoolSetting* setting);

	void onToggle(CCObject*);

	BoolSettingNode(float width, float height) : GeodeSettingNode<BoolSetting>(width, height) {}

public:
	static BoolSettingNode* create(BoolSetting* setting, float width);
};

class IntSettingNode : public GeodeSettingNode<IntSetting> {
protected:
	CCLabelBMFont* m_valueLabel;

	bool init(IntSetting* setting);

	void updateValue();
	void onArrow(CCObject*);

	IntSettingNode(float width, float height) : GeodeSettingNode<IntSetting>(width, height) {}

public:
	static IntSettingNode* create(IntSetting* setting, float width);
};

class FloatSettingNode : public GeodeSettingNode<FloatSetting> {
protected:
	CCLabelBMFont* m_valueLabel;

	bool init(FloatSetting* setting);

	void updateValue();
	void onArrow(CCObject*);

	FloatSettingNode(float width, float height) : GeodeSettingNode<FloatSetting>(width, height) {}

public:
	static FloatSettingNode* create(FloatSetting* setting, float width);
};

class StringSettingNode : public GeodeSettingNode<StringSetting> {
protected:
	bool init(StringSetting* setting);

	StringSettingNode(float width, float height) : GeodeSettingNode<StringSetting>(width, height) {}

public:
	static StringSettingNode* create(StringSetting* setting, float width);
};

class ColorSettingNode : public GeodeSettingNode<ColorSetting> {
protected:
	bool init(ColorSetting* setting);

	ColorSettingNode(float width, float height) : GeodeSettingNode<ColorSetting>(width, height) {}

public:
	static ColorSettingNode* create(ColorSetting* setting, float width);
};

class ColorAlphaSettingNode : public GeodeSettingNode<ColorAlphaSetting> {
protected:
	bool init(ColorAlphaSetting* setting);

	ColorAlphaSettingNode(float width, float height) : GeodeSettingNode<ColorAlphaSetting>(width, height) {}

public:
	static ColorAlphaSettingNode* create(ColorAlphaSetting* setting, float width);
};

class PathSettingNode : public GeodeSettingNode<PathSetting> {
protected:
	bool init(PathSetting* setting);

	PathSettingNode(float width, float height) : GeodeSettingNode<PathSetting>(width, height) {}

public:
	static PathSettingNode* create(PathSetting* setting, float width);
};

class StringSelectSettingNode : public GeodeSettingNode<StringSelectSetting> {
protected:
	bool init(StringSelectSetting* setting);

	StringSelectSettingNode(float width, float height) : GeodeSettingNode<StringSelectSetting>(width, height) {}

public:
	static StringSelectSettingNode* create(StringSelectSetting* setting, float width);
};

class CustomSettingPlaceHolderNode : public TableViewCell {
protected:
	bool init(CustomSettingPlaceHolder* setting);

	CustomSettingPlaceHolderNode(float width, float height) : TableViewCell("custom", width, height) {}

public:
	static CustomSettingPlaceHolderNode* create(CustomSettingPlaceHolder* setting, float width);
};

#pragma warning(default: 4067)
