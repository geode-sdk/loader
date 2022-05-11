#include <Geode.hpp>

USE_GEODE_NAMESPACE();

#ifdef GEODE_IS_MACOS
class $modify(MenuLayer) {
	void FLAlert_Clicked(FLAlertLayer* layer, bool btn1) {
		if (btn1 == true && layer->getTag() == 0) {
			Loader::getInternalMod()->log() << "Saving...";
	        auto r = Loader::get()->saveSettings();
	        if (!r) Loader::getInternalMod()->logInfo(r.error(), Severity::Error);
	        Loader::getInternalMod()->log() << "Saved";
		} 
		MenuLayer::FLAlert_Clicked(layer, btn1);
	} 
};
#else

class $modify(AppDelegate) {
    void trySaveGame() {
        Loader::getInternalMod()->log() << "Saving...";
        auto r = Loader::get()->saveSettings();
        if (!r) Loader::getInternalMod()->logInfo(r.error(), Severity::Error);
        Loader::getInternalMod()->log() << "Saved";
        return AppDelegate::trySaveGame();
    }
};

#endif
