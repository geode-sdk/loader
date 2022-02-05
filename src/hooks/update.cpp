#include <Geode.hpp>
#include <Internal.hpp>

USE_GEODE_NAMESPACE();

class $modify(CCScheduler) {
    void update(float dt) {
        Geode::get()->executeGDThreadQueue();
        return $CCScheduler::update(dt);
    }
};
