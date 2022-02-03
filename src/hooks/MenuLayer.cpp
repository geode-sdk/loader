#include "hook.hpp"
#include <mods/list/ModListLayer.hpp>

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

		auto ng = getChild<>(bottomMenu, -1);
		ng->retain();
		ng->removeFromParent();

		auto spr = CCSprite::create("geode-button-gold.png");
		if (!spr) {
			spr = ButtonSprite::create("!!");
		} else {
			Interface::mod()->log() << spr->getTextureRect().size << geode::endl;
			CCRect rect = spr->getTextureRect();
			rect.size = rect.size * 4;
			auto frame = CCSpriteFrame::createWithTexture(
				spr->getTexture(),
				rect,
				spr->isTextureRectRotated(),
				CCPointMake(3, -6),
				rect.size
			);
			spr->setDisplayFrame(frame);
			frame->release();
		}
		
		auto btn = CCMenuItemSpriteExtra::create(
			spr, this, menu_selector(CustomMenuLayer::onGeode)
		);
		bottomMenu->addChild(btn);

		bottomMenu->addChild(ng);
		ng->release();

		bottomMenu->alignItemsHorizontallyWithPadding(3.f);

		bottomMenu->addChild(chest);
		chest->release();

		return true;
	}

	void onGeode(CCObject*) {
		ModListLayer::scene();
	}
};

