#include <Hook.hpp>
#include <vector>
#include <Mod.hpp>
#include <Loader.hpp>
#include <utils/casts.hpp>
#include <utils/vector.hpp>
#include <hook/hook.hpp>
#include "Internal.hpp"
#include "InternalMod.hpp"

USE_GEODE_NAMESPACE();

struct hook_info {
    Hook* hook;
    Mod* mod;
};

// for some reason this doesn't work as 
// a normal static global. the vector just 
// gets cleared for no reason somewhere 
// between `addHook` and `loadHooks`


GEODE_STATIC_VAR(std::vector<hook_info>, internalHooks);
GEODE_STATIC_VAR(bool, readyToHook);

Result<Hook*> Mod::addHookBase(void* addr, void* detour, Hook* hook) {
    if (!hook) {
        hook = new Hook();
        hook->m_address = addr;
    }
    if ((hook->m_handle = const_cast<void*>(geode::core::hook::add(addr, detour)))) {
        this->m_hooks.push_back(hook);
        hook->m_enabled = true;
        return Ok<Hook*>(hook);
    } else {
        delete hook;
        return Err<>(
            "Unable to create hook at " + std::to_string(as<uintptr_t>(addr))
        );
    }
}

Result<Hook*> Mod::addHookBase(Hook* hook) {
    return this->addHookBase(
        hook->m_address,
        hook->m_detour,
        hook
    );
}

Result<> Mod::enableHook(Hook* hook) {
    if (!hook->isEnabled()) {
        if (!hook->m_handle) {
            if ((hook->m_handle = const_cast<void*>(geode::core::hook::add(hook->m_address, hook->m_detour)))) {
                hook->m_enabled = true;
                return Ok<>();
            }
            return Err<>("Unable to create hook");
        }
        return Err<>("Hook already has a handle");
    }
    return Ok<>();
}

Result<> Mod::disableHook(Hook* hook) {
    if (hook->isEnabled()) {
        if (hook->m_handle) {
            if (geode::core::hook::remove(hook->m_handle)) {
                hook->m_enabled = false;
                hook->m_handle = nullptr;
                return Ok<>();
            }
            return Err<>("Unable to remove hook");
        }
        return Err<>("Hook lacks a handle");
    }
    return Ok<>();
}

Result<> Mod::removeHook(Hook* hook) {
    auto res = this->disableHook(hook);
    if (res) {
        vector_utils::erase<Hook*>(this->m_hooks, hook);
        delete hook;
    }
    return res;
}

Result<Hook*> Mod::addHook(void* addr, void* detour) {
    if (readyToHook()) {
        return this->addHookBase(addr, detour);
    } else {
        auto hook = new Hook();
        hook->m_address = addr;
        hook->m_detour = detour;
        internalHooks().push_back({ hook, this });
        return Ok<Hook*>(hook);
    }
}

Result<Hook*> Mod::addHook(void* addr, void* detour, void** trampoline) {
    *trampoline = addr;
    return this->addHook(addr, detour);
}

bool Geode::loadHooks() {
    readyToHook() = true;
    auto thereWereErrors = false;
    for (auto const& hook : internalHooks()) {
        auto res = hook.mod->addHookBase(hook.hook);
        if (!res) {
            hook.mod->throwError(
                res.error(),
                Severity::Error
            );
            thereWereErrors = true;
        }
    }
    // free up memory
    internalHooks().clear();
    return !thereWereErrors;
}
