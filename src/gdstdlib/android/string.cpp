#include <gdstdlib.hpp>
#include <utils/platform.hpp>

#ifdef GEODE_IS_ANDROID

#include <string.h>

namespace gd {
	string::string() : m_data(nullptr) {}

	string::string(char const* c_str) {
		const auto size = strlen(c_str);
		m_data = static_cast<decltype(m_data)>(operator new(sizeof(_internal_string) + size + 1));
		m_data[-1] = {
			size,
			size + 1,
			0u
		};
		memcpy(m_data, c_str, size + 1);
	}
	string::string(const string& other) : string(other.m_data == nullptr ? "" : other.c_str()) {
	}
	string& string::operator=(char const* c_str) {
		return *this;
	}
	string& string::operator=(const string& other) {
		return *this;
	}
	string::~string() {
		if (m_data == nullptr) return;
		if (m_data[-1].m_refcount-- <= 0) {
			delete m_data;
		}
	}

}

#endif