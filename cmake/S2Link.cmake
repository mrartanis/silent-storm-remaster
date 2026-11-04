# The historical CMake targets break static-library reference cycles with the
# GNU ld options --start-group and --whole-archive. Apple's ld64 rejects those
# spellings but resolves archives in repeated passes anyway; only whole-archive
# modules need explicit -force_load entries. All game targets therefore call
# s2_target_link_libraries(): GNU flags pass through unchanged on every other
# linker and are rewritten here for ld64.
function(s2_target_link_libraries target)
  if(NOT APPLE)
    target_link_libraries(${target} ${ARGN})
    return()
  endif()
  set(_s2_args "")
  set(_s2_whole FALSE)
  foreach(_s2_arg IN LISTS ARGN)
    if(_s2_arg STREQUAL "-Wl,--whole-archive")
      set(_s2_whole TRUE)
      continue()
    endif()
    if(_s2_arg STREQUAL "-Wl,--no-whole-archive")
      set(_s2_whole FALSE)
      continue()
    endif()
    if(_s2_arg STREQUAL "-Wl,--start-group" OR _s2_arg STREQUAL "-Wl,--end-group")
      continue()
    endif()
    if(_s2_whole)
      # Keep the target itself on the line: dropping it would also drop the
      # dependency edges and usage requirements CMake derives from it.
      if(TARGET "${_s2_arg}")
        list(APPEND _s2_args "-Wl,-force_load,$<TARGET_FILE:${_s2_arg}>" "${_s2_arg}")
      else()
        list(APPEND _s2_args "-Wl,-force_load,${_s2_arg}" "${_s2_arg}")
      endif()
    else()
      list(APPEND _s2_args "${_s2_arg}")
    endif()
  endforeach()
  target_link_libraries(${target} ${_s2_args})
endfunction()

# --gc-sections is a GNU ld spelling; Apple's ld64 strips dead code by the
# same intent under -dead_strip. Mach-O prefixes every symbol with an extra
# underscore, so mangled-name -u pins need one more leading underscore there.
function(s2_target_link_options target)
  set(_s2_args "")
  foreach(_s2_arg IN LISTS ARGN)
    if(APPLE AND _s2_arg STREQUAL "-Wl,--gc-sections")
      list(APPEND _s2_args "-Wl,-dead_strip")
    elseif(APPLE AND _s2_arg MATCHES "^-Wl,-u,_")
      string(SUBSTRING "${_s2_arg}" 7 -1 _s2_symbol)
      list(APPEND _s2_args "-Wl,-u,_${_s2_symbol}")
    else()
      list(APPEND _s2_args "${_s2_arg}")
    endif()
  endforeach()
  target_link_options(${target} ${_s2_args})
endfunction()

if(APPLE)
  # glibc exposes iconv() from libc; the macOS SDK ships it as a separate library.
  set(CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} -liconv")
  set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -liconv")
endif()
