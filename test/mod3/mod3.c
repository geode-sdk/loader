#include "mod3.h"

bool GEODE_C_API geode_load(void* mod) {
    geode_mod_log(mod, "Hi from mod3.c!");
    
    return true;
}

void GEODE_C_API geode_unload() {}
