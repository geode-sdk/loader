#include <dobby.h>
#include <iostream>
#include "Core.hpp"

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
        // std::cout << "at: " << at << " to: " << to << std::endl;
        DobbyDestroy(at);
        DobbyHook(at, to, &trampolines()[at]);
	}
}

bool geode::core::hook::initialize() {
	return true;
}