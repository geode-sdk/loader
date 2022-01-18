#include "InternalMod.hpp"

ModInfo getInternalModInfo() {
    ModInfo info;
    
    info.m_id          = "com.lilac.lilac";
    info.m_name        = "Lilac";
    info.m_developer   = "Lilac Team";
    info.m_description = "Internal representation";
    info.m_details     = "Internal representation of Lilac.";
    info.m_credits     = "";
    info.m_version     = { 1, 0, 0 };

    return info;
}

InternalMod::InternalMod() : Mod(getInternalModInfo()) {
}

InternalMod* InternalMod::get() {
    static auto g_mod = new InternalMod;
    return g_mod;
}
