function(s2_add_upstream_test_modules target source_dir)
  target_sources(${target} PRIVATE
    "${source_dir}/diagnostics/upstream/AITests.cpp"
    "${source_dir}/diagnostics/upstream/BuildingTests.cpp"
    "${source_dir}/diagnostics/upstream/DatabaseTests.cpp"
    "${source_dir}/diagnostics/upstream/LegacyTests.cpp"
    "${source_dir}/diagnostics/upstream/NavigationTests.cpp"
    "${source_dir}/diagnostics/upstream/ProjectileTests.cpp"
    "${source_dir}/diagnostics/upstream/RenderTests.cpp"
    "${source_dir}/diagnostics/upstream/TestSupport.cpp"
    "${source_dir}/diagnostics/upstream/UnitTests.cpp"
  )
endfunction()
