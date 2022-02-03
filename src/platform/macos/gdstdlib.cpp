#include <gdstdlib.hpp>

#if defined(GEODE_IS_MAC) || defined(GEODE_IS_ANDROID)

using namespace gd;

namespace {
	static inline auto emptyInternalString() {
		return reinterpret_cast<_internal_string*>(*reinterpret_cast<uintptr_t*>(geode::base::get() + 0x6030d0) + sizeof(_internal_string));
	}
}
string::string() : m_data(nullptr) {
	m_data = emptyInternalString();
}
string::string(char const* ok) : m_data(nullptr) {
	reinterpret_cast<void(*)(string*, char const*)>(geode::base::get() + 0x48b0b4)(this, ok);
}
string::string(string const& ok) : m_data(nullptr) {
	reinterpret_cast<void(*)(string*, string const&)>(geode::base::get() + 0x489fcc)(this, ok);
}
string& string::operator=(char const* ok) {
	auto ret = reinterpret_cast<string*(*)(string*, char const*, size_t)>(geode::base::get() + 0x489f96)(this, ok, strlen(ok));
	return *ret;
}
string& string::operator=(string const& ok) {
	auto ret = reinterpret_cast<string*(*)(string*, string const&)>(geode::base::get() + 0x489f9c)(this, ok);
	return *ret;
}
string::~string() {
	if (m_data == nullptr) return;
	if (m_data == emptyInternalString()) return;
	if (m_data[-1].m_refcount-- <= 0) {
		reinterpret_cast<void(*)(_internal_string*)>(geode::base::get() + 0x48b03c)(&m_data[-1]);
	}
}

#endif
