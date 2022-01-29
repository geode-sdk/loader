#include "hook.hpp"
#include <mods/list/ModListLayer.hpp>
#include <WackyGeodeMacros>

class $modify(GameManager) {
	void reloadAllStep2() {
		$GameManager::reloadAllStep2();
		Loader::get()->addResourceSearchPaths();
	}
};

class $modify(CustomMenuLayer, MenuLayer) {
	bool init() {
		if (!$MenuLayer::init())
			return false;
		
		auto bottomMenu = getChild<CCMenu*>(this, 3);

		auto chest = getChild<>(bottomMenu, -1);
		chest->retain();
		chest->removeFromParent();

		auto y = getChild<>(bottomMenu, 0)->getPositionY();

		auto spr = CCSprite::create("geode-button-color.png");
		if (!spr) {
			spr = ButtonSprite::create("!!");
		} else {
			auto frame = CCSpriteFrame::createWithTexture(
				spr->getTexture(),
				spr->getTextureRect(),
				spr->isTextureRectRotated(),
				{ 3, -6 },
				spr->getTextureRect().size * 3
			);
			spr->setDisplayFrame(frame);
			frame->release();
		}
		auto btn = CCMenuItemSpriteExtra::create(
			spr, this, menu_selector(CustomMenuLayer::onGeode)
		);
		bottomMenu->addChild(btn);

		bottomMenu->alignItemsHorizontallyWithPadding(3.f);

		CCARRAY_FOREACH_B_TYPE(bottomMenu->getChildren(), node, CCNode) {
			node->setPositionY(y);
		}

		bottomMenu->addChild(chest);
		chest->release();

		return true;
	}

	void onGeode(CCObject*) {
		ModListLayer::scene();
	}
};

