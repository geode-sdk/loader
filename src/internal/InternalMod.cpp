#include "InternalMod.hpp"
#include "about.hpp"

ModInfo getInternalModInfo() {
    ModInfo info;
    
    info.m_id          = "com.geode.loader";
    info.m_name        = "Geode";
    info.m_developer   = "Geode Team";
    info.m_description = "Internal representation";
    info.m_details     = loader_about_md;
    info.m_creditsString = "";
    info.m_version     = { 1, 0, 0 };
    info.m_supportsDisabling = false;

    return info;
}

InternalMod::InternalMod() : Mod(getInternalModInfo()) {}

InternalMod::~InternalMod() {}

InternalMod* InternalMod::get() {
    static auto g_mod = new InternalMod;
    return g_mod;
}
