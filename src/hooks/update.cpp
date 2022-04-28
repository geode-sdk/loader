#include <Geode.hpp>
#include <InternalLoader.hpp>

USE_GEODE_NAMESPACE();

class $modify(CCScheduler) {
    void update(float dt) {
        InternalLoader::get()->executeGDThreadQueue();
        geode::log << "hmm";
        return CCScheduler::update(dt);
    }
};
