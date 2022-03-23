#pragma once
#include "Platform.hpp"

/*
	Internal use functions
*/
namespace geode::core {
	namespace impl {
		void* generateRawTrampoline(void* address);

		void addJump(void* at, void* to);

		void handleContext(void* context, const void* current);
	}

	namespace hook {
		inline bool initialize() {
			return TargetPlatform::initialize();
		}
	}
}
