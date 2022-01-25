#include <HeaderBase.hpp>
#include <helper/AlertPopup.hpp>
#undef max
#undef min

/**
 * 			IDK man
 * 
 * just haven't been feeling it
 * 
 * https://youtu.be/fiP5H4QpFn8
 */

USE_GEODE_NAMESPACE();

AlertPopup* AlertPopup::create(std::string const& title, std::string const& caption, 
	std::string const& button1, std::string const& button2) {

	auto ret = new AlertPopup(); 
	if (ret && ret->init(title, caption, button1, button2)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

AlertPopup* AlertPopup::withDelegate(AlertPopupDelegate* delegate) {
	m_delegate = delegate;
	return this;
}

AlertPopup* AlertPopup::show() {
	auto runningScene = CCDirector::sharedDirector()->getRunningScene();
	auto zOrder = std::max(105, runningScene->getHighestChildZ() + 1);
	runningScene->addChild(this, zOrder);
	setVisible(true);
	return this;
}

bool AlertPopup::init(std::string const& title, std::string const& caption, 
	std::string const& button1, std::string const& button2) {

	if (!CCLayerColor::initWithColor(ccc4(0,0,0,150))) return false;

	CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);

	setTouchEnabled(true);
	setKeypadEnabled(true);
	setKeyboardEnabled(true);

	auto windowSize = CCDirector::sharedDirector()->getWinSize();

	// auto textArea = TextArea::create(caption, "chatFont.fnt", 1.0, width - 60.0, CCPointMake(0.5, 0.5), 20.0, false);
	// addChild(textArea, 3);
	// height = textArea->m_obRect.size.height + 120;
	// height = std::max(height, 140.0f);

	m_buttonMenu = CCMenu::create();
	addChild(m_buttonMenu, 2);

	auto button1Sprite = ButtonSprite::create(button1.c_str());
	auto button1Menu = CCMenuItemSpriteExtra::create(button1Sprite, nullptr, this, SEL_MenuHandler(&AlertPopup::onButton1));
	m_buttonMenu->addChild(button1Menu);

	auto button2Sprite = ButtonSprite::create(button2.c_str());
	auto button2Menu = CCMenuItemSpriteExtra::create(button2Sprite, nullptr, this, SEL_MenuHandler(&AlertPopup::onButton2));
	m_buttonMenu->addChild(button2Menu);

	float menuLength = button1Sprite->getContentSize().width + button2Sprite->getContentSize().width;
	float padding = 15.0f;
	m_buttonMenu->alignItemsHorizontallyWithPadding(padding);

	m_buttonMenu->setPosition(CCPointMake(windowSize.width * 0.5, windowSize.height * 0.5 + 30.0));

	return true;
}

void AlertPopup::close() {
	setTouchEnabled(false);
	setKeypadEnabled(false);
	setKeyboardEnabled(false);
	removeFromParentAndCleanup(true);
}

void AlertPopup::keyBackClicked() {
	if (m_delegate) m_delegate->onDismiss();
	close();
}

void AlertPopup::onButton1(CCObject* ob) {
	if (m_delegate) m_delegate->onButton1();
	close();
}

void AlertPopup::onButton2(CCObject* ob) {
	if (m_delegate) m_delegate->onButton2();
	close();
}

AlertPopup::~AlertPopup() {

}

// void AlertPopup::keyDown(enumKeyCodes keyCode) {
// 	if (keyCode == 0x3ec) sendAndCleanup(true);
// 	else if (keyCode == KEY_Space && !m_noAction) return;
// 	else CCLayer::keyDown(keyCode);
// }
