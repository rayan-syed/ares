add_executable(n64-headless
  main.cpp
  n64-runtime.cpp
  n64-runtime.hpp
  platform-headless.cpp
  platform-headless.hpp
)

target_include_directories(n64-headless PRIVATE
  ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(n64-headless PRIVATE
  MIA_LIBRARY
)

target_link_libraries(n64-headless PRIVATE
  ares::ares
  ares::mia
  ares::nall
)

ares_configure_executable(n64-headless)