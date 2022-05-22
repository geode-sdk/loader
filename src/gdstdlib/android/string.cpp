#include <gdstdlib.hpp>
#include <utils/platform.hpp>

#ifdef GEODE_IS_ANDROID

#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <dlfcn.h>

// FIXME: all this code is so stinky i hate gd::string

using AllocatorFn = void*(*)(size_t);
using DeallocatorFn = void(*)(void*);

AllocatorFn get_allocator() {
	auto handle = dlopen("libcocos2dcpp.so", RTLD_LAZY | RTLD_NOLOAD);
	return reinterpret_cast<AllocatorFn>(dlsym(handle, "_Znwj"));
}

DeallocatorFn get_deallocator() {
	auto handle = dlopen("libcocos2dcpp.so", RTLD_LAZY | RTLD_NOLOAD);
	return reinterpret_cast<DeallocatorFn>(dlsym(handle, "_ZdlPv"));
}

void* gdstd_allocate(const size_t size) {
	static const auto allocator = get_allocator();
	return allocator(size);
}

void gdstd_free(void* p) {
	static const auto deallocator = get_deallocator();
	return deallocator(p);
}

namespace gd {
	string::string() : m_data(nullptr) {}

	void string::set_data(const char* c_str, const size_t size) {
		m_data = static_cast<_internal_string*>(gdstd_allocate(sizeof(_internal_string) + size + 1)) + 1;
		m_data[-1] = {
			size,
			size + 1,
			0u
		};
		memcpy(m_data, c_str, size + 1);
	}
	void string::clear_data() {
		if (m_data) {
			if (--(m_data[-1].m_refcount) <= 0)
				gdstd_free(&m_data[-1]);
			
			m_data = nullptr;
		}
	}
	void string::assign(const string& other) {
		if (this == &other) return;
		clear_data();
		if (other.m_data) {
			m_data = other.m_data;
			m_data[-1].m_refcount++;
		}
	}
	void string::assign(const char* other) {
		clear_data();
		set_data(other, strlen(other));
	}

	string::string(char const* c_str) {
		set_data(c_str, strlen(c_str));
	}
	string::string(const string& other) : string(other.m_data == nullptr ? "" : other.c_str()) {
	}
	string& string::operator=(char const* c_str) {
		assign(c_str);
		return *this;
	}
	string& string::operator=(const string& other) {
		assign(other);
		return *this;
	}
	string::~string() {
		clear_data();
	}

}

#endif