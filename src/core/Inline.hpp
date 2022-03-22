#pragma once

namespace geode::core::impl {
	void* generateRawTrampoline(void* address);

	void addJump(void* at, void* to);
}
