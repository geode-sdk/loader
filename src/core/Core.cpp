#include <dobby.h>
#include <iostream>
#include "Core.hpp"
#include <Log.hpp>
#include <platform.hpp>
#include <map>

#ifdef GEODE_IS_MACOS
#warning Bandaid patch!
#include <GDML.hpp>

namespace geode::core::impl {
	namespace {
		auto& trampolines() {
			static std::map<void*, HookContainer> ret;
			return ret;
		}
	}

	void* generateRawTrampoline(void* address) {
        // std::cout << "address: " << address << std::endl;
        // std::cout << "trampoline address is : " << trampolines()[address] << std::endl;
        return (void*)trampolines()[address].getOriginal();
	}

	void addJump(void* at, void* to) {
		trampolines()[at] = HookContainer((uintptr_t)at, (func_t)to);
		trampolines()[at].enable();
	}
}

bool geode::core::hook::initialize() {
	return true;
}
#else
namespace geode::core::impl {
	namespace {
		auto& trampolines() {
			static std::map<void*, void*> ret;
			return ret;
		}
	}

	void* generateRawTrampoline(void* address) {
        // std::cout << "address: " << address << std::endl;
        // std::cout << "trampoline address is : " << trampolines()[address] << std::endl;
        return trampolines()[address];
	}

	void addJump(void* at, void* to) {
        Log() << "Add jump";
        DobbyDestroy(at);
        Log() << "middle";
        DobbyHook(at, to, &trampolines()[at]);
        Log() << "Added jump";
	}
}

bool geode::core::hook::initialize() {
	return true;
}
#endif
