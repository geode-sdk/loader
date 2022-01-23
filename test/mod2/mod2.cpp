#include "mod2.hpp"

using namespace geode::core::meta;
using namespace geode::core::meta::x86;

bool GJGarageLayer_init(GJGarageLayer* self) {
    if (!self->init())
        return false;

    if (Loader::get()->isModLoaded("com.geode.test_one")) {
        logMessage("Hi from TestMod2!");
    } else {
        Interface::mod()->log() << "TestMod1 is not loaded :(" << geode::endl;
    }

    auto label = CCLabelBMFont::create("Holy hell omg gosh", "bigFont.fnt");
    label->setPosition(100, 100);
    label->setZOrder(99999);
    self->addChild(label);
    
    return true;
}

GEODE_API bool GEODE_CALL geode_load(Mod* mod) {
    Interface::get()->init(mod);

    mod->addHook(
        reinterpret_cast<void*>(address_of<&GJGarageLayer::init>),
        reinterpret_cast<void*>(Thiscall<bool, GJGarageLayer*>::get_wrapper<&GJGarageLayer_init>())
    );

    return true;
}

GEODE_API void GEODE_CALL geode_unload() {}
