#include <Geode>

USE_GEODE_NAMESPACE();

class $modify(GameManager) {
	void reloadAllStep2() {
		$GameManager::reloadAllStep2();
		Loader::get()->updateResources();
	}
};
