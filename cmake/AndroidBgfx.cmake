# bgfx.cmake enables annotations unconditionally in Debug. The Redmi's Vulkan
# loader does not resolve the debug object naming function used at startup.
foreach(property IN ITEMS COMPILE_DEFINITIONS INTERFACE_COMPILE_DEFINITIONS)
  get_target_property(definitions bgfx ${property})
  list(FILTER definitions EXCLUDE REGEX "^BGFX_CONFIG_DEBUG_ANNOTATION=")
  set_property(TARGET bgfx PROPERTY ${property} "${definitions}")
endforeach()
target_compile_definitions(bgfx PUBLIC BGFX_CONFIG_DEBUG_ANNOTATION=0
  PRIVATE BGFX_CONFIG_RENDERER_OPENGLES=0 BGFX_CONFIG_RENDERER_VULKAN=1)
target_link_libraries(bgfx PUBLIC android log)
