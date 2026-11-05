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

# With landscape handled by the Android compositor, the identity transform
# can return SUBOPTIMAL on every present. The acquired images remain usable.
# Size changes and OUT_OF_DATE still recreate the swapchain.
set(vulkan_source "${BGFX_DIR}/src/renderer_vk.cpp")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${vulkan_source}")
file(READ "${vulkan_source}" vulkan_code)
set(acquire_end "\t, &m_backBufferColorIdx\n\t\t\t\t\t);\n\t\t\t}")
set(present_end "result = vkQueuePresentKHR(m_queue, &pi);\n\t\t\t}")
foreach(call_end IN ITEMS "${acquire_end}" "${present_end}")
  string(FIND "${vulkan_code}" "${call_end}" call_offset)
  if(call_offset EQUAL -1)
    message(FATAL_ERROR "The Android swapchain workaround needs updating for this bgfx source")
  endif()
  string(REPLACE "${call_end}" "${call_end}\n\t\t\tif (result == VK_SUBOPTIMAL_KHR) result = VK_SUCCESS;"
    vulkan_code "${vulkan_code}")
endforeach()
# The pinned Vulkan backend copies the old window into the framebuffer on
# reset. The new Android SurfaceView needs to be used on this very frame.
set(old_window "m_backBuffer.m_nwh != m_mainSwapChain.nwh")
string(FIND "${vulkan_code}" "${old_window}" old_window_offset)
if(old_window_offset EQUAL -1)
  message(FATAL_ERROR "The Android window replacement needs updating for this bgfx source")
endif()
string(REPLACE "${old_window}" "m_backBuffer.m_nwh != _swapChain.nwh" vulkan_code "${vulkan_code}")
string(REPLACE "m_backBuffer.m_nwh = m_mainSwapChain.nwh;" "m_backBuffer.m_nwh = _swapChain.nwh;" vulkan_code "${vulkan_code}")
set(android_vulkan_source "${CMAKE_CURRENT_BINARY_DIR}/android-bgfx/renderer_vk.cpp")
file(CONFIGURE OUTPUT "${android_vulkan_source}" CONTENT "@vulkan_code@" @ONLY)
get_target_property(bgfx_sources bgfx SOURCES)
list(REMOVE_ITEM bgfx_sources "${vulkan_source}")
set_property(TARGET bgfx PROPERTY SOURCES "${bgfx_sources};${android_vulkan_source}")
target_include_directories(bgfx PRIVATE "${BGFX_DIR}/src")
