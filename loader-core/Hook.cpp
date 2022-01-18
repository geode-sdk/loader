#include <interface/Hook.hpp>
#include <vector>
#include <interface/Mod.hpp>
#include <interface/Loader.hpp>
#include <helpers/casts.hpp>
#include <helpers/vector.hpp>
#include <core/include/lilac/core/hook/hook.hpp>
#include "Internal.hpp"
#include "InternalMod.hpp"

USE_LILAC_NAMESPACE();

struct hook_info {
    Hook* hook;
    Mod* mod;
};

// for some reason this doesn't work as 
// a normal static global. the vector just 
// gets cleared for no reason somewhere 
// between `addHook` and `loadHooks`
static std::vector<hook_info>* g_hooks = nullptr;
static bool g_readyToHook = false;

Result<Hook*> Mod::addHookBase(void* addr, void* detour, Hook* hook) {
    if (!hook) {
        hook = new Hook();
        hook->m_address = addr;
    }
    if ((hook->m_handle = const_cast<void*>(lilac::core::hook::add(addr, detour)))) {
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
            if ((hook->m_handle = const_cast<void*>(lilac::core::hook::add(hook->m_address, hook->m_detour)))) {
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
            if (lilac::core::hook::remove(hook->m_handle)) {
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
    if (g_readyToHook) {
        return this->addHookBase(addr, detour);
    } else {
        auto hook = new Hook();
        hook->m_address = addr;
        hook->m_detour = detour;
        if (!g_hooks) g_hooks = new std::vector<hook_info>;
        g_hooks->push_back({ hook, this });
        return Ok<Hook*>(hook);
    }
}

Result<Hook*> Mod::addHook(void* addr, void* detour, void** trampoline) {
    *trampoline = addr;
    return this->addHook(addr, detour);
}

bool Lilac::loadHooks() {
    g_readyToHook = true;
    auto thereWereErrors = false;
    for (auto const& hook : *g_hooks) {
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
    g_hooks->clear();
    delete g_hooks;
    g_hooks = nullptr;
    return !thereWereErrors;
}
