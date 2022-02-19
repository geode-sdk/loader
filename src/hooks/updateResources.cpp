#include <Geode.hpp>

USE_GEODE_NAMESPACE();

class $modify(GameManager) {
	void reloadAllStep2() {
		GameManager::reloadAllStep2();
		Loader::get()->updateResourcePaths();
	}
};

class $modify(LoadingLayer) {
	void loadAssets() {
		LoadingLayer::loadAssets();
		if (this->m_loadStep == 5) {
			Loader::get()->updateResources();
		}
	}
};

class $modify(CustomListView) {
	static CustomListView* create(cocos2d::CCArray* items, float height, float width, int page, BoomListType type) {
		Interface::mod()->log() << "CustomListView::create -> height: " << height << " width: " << width << " type: " << (int)type;
		return CustomListView::create(items, height, width, page, type);
	}
};
