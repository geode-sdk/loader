#include "hook.hpp"
#include <Internal.hpp>

class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down) {
        KeybindManager::get()->registerKeyPress(key, down);
        if (!KeybindManager::keyIsModifier(key)) {
            if (KeybindManager::get()->handleKeyEvent(
                KB_GLOBAL_CATEGORY,
                Keybind(key),
                CCDirector::sharedDirector()->getRunningScene(),
                down
            )) return true;
        }
        return $CCKeyboardDispatcher::dispatchKeyboardMSG(key, down);
    }
};

class $modify(CCScheduler) {
    void update(float dt) {
        KeybindManager::get()->handleRepeats(dt);
        Lilac::get()->executeGDThreadQueue();
        return $CCScheduler::update(dt);
    }
};

#ifdef GEODE_IS_DESKTOP
class $modify(CCEGLView) {
    void onGLFWMouseCallBack(GLFWwindow* wnd, int btn, int pressed, int z) {
        KeybindManager::get()->registerMousePress(
            static_cast<MouseButton>(btn), pressed
        );
        return $CCEGLView::onGLFWMouseCallBack(wnd, btn, pressed, z);
    }
};
#endif
