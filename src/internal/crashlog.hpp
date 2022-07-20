#pragma once

#include <string>

/**
 * Platform-specific crashlog functions. Used by the loader
 */
namespace crashlog {
    /**
     * Setup platform-specific crashlog handler
     * @returns True if the handler was succesfully installed, false otherwise
     */
    bool setupPlatformHandler();
    /**
     * Check if previous launch of GD crashed unexpectedly
     * @returns True if the launch crashed, false otherwise or if indeterminate
     */
    bool didLastLaunchCrash();
    /**
     * Get the directory where crashlogs are saved on the current platform
     * @returns Path to the directory, or an empty string if the platform does 
     * not support crash logs
     */
    std::string const& getCrashLogDirectory();
}
