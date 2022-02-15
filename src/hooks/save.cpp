#include <Geode.hpp>

USE_GEODE_NAMESPACE();

class $modify(AppDelegate) {
    virtual void trySaveGame() {
        Loader::getInternalMod()->log() << "Saving..." << geode::endl;
        Loader::get()->saveSettings();
        for (auto mod : Loader::get()->getLoadedMods()) {
            auto r = mod->saveData();
            if (!r) Loader::getInternalMod()->logInfo(r.error(), Severity::Error);
        }
        Loader::getInternalMod()->log() << "Saved" << geode::endl;
        return AppDelegate::trySaveGame();
    }
};
