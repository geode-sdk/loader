# Implementing Platform Functions

### `Mod::loadPlatformBinary` 

This needs to

 * Load the platform binary

 * Set the Mod's `m_loadFunc` and `m_unloadFunc`

 * Set `m_platformInfo`

If any of these fail, return an error. On success, return `Ok<>()`

### `Mod::unloadPlatformBinary`

This needs to

 * Free up `m_platformInfo`

 * Unload the platform binary

 * Set `m_loadFunc` and `m_unloadFunc` to null
