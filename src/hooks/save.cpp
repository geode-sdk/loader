#include <Geode.hpp>

USE_GEODE_NAMESPACE();

class $modify(AppDelegate) {
    void trySaveGame_() {
        Loader::getInternalMod()->log() << "Saving..." << geode::endl;
        for (auto mod : Loader::get()->getLoadedMods()) {
            auto r = mod->saveData();
            if (!r) Loader::getInternalMod()->logInfo(r.error(), Severity::Error);
        }
        return $AppDelegate::trySaveGame_();
    }
};
