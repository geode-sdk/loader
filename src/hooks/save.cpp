#include <Geode.hpp>

USE_GEODE_NAMESPACE();

class $modify(AppDelegate) {
    virtual void trySaveGame() {
        Loader::getInternalMod()->log() << "Saving...";
        auto r = Loader::get()->saveSettings();
        if (!r) Loader::getInternalMod()->logInfo(r.error(), Severity::Error);
        Loader::getInternalMod()->log() << "Saved";
        return AppDelegate::trySaveGame();
    }
};
