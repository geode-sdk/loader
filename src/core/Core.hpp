#pragma once
/*
	Internal use functions
*/
namespace geode::core {
	namespace impl {
		void* generateRawTrampoline(void* address);

		void addJump(void* at, void* to);
	}

	namespace hook {
		bool initialize();
	}
}