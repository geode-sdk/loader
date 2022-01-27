#include "InternalMod.hpp"

ModInfo getInternalModInfo() {
    ModInfo info;
    
    info.m_id          = "com.geode.geode";
    info.m_name        = "Geode";
    info.m_developer   = "Geode Team";
    info.m_description = "Internal representation";
    info.m_details     = "Internal representation of Geode.";
    info.m_credits     = "";
    info.m_version     = { 1, 0, 0 };

    return info;
}

InternalMod::InternalMod() : Mod(getInternalModInfo()) {
    this->m_supportsDisabling = false;
}

InternalMod::~InternalMod() {
}

InternalMod* InternalMod::get() {
    static auto g_mod = new InternalMod;
    return g_mod;
}
