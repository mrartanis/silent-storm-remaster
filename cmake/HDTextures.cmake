# Prepare the frozen HD release on the host, including cross-compiled games.
function(s2_build_hd_textures target)
  option(S2_BUILD_HD_TEXTURES "Install the prebuilt HD texture pack beside Game" ON)
  if(NOT S2_BUILD_HD_TEXTURES)
    return()
  endif()
  find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter)
  add_custom_target(S2HDTextures
    COMMAND "${Python3_EXECUTABLE}" "${root}/packaging/prepare_hd.py"
      --cache "${root}/data/res-hd" --output "$<TARGET_FILE_DIR:${target}>/res-hd"
    COMMENT "Preparing prebuilt HD textures (cached in data/res-hd)"
    VERBATIM)
  add_dependencies(${target} S2HDTextures)
endfunction()
