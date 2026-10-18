# Exposes compile_commands.json in the source root for the LSP (clangd).
#
# compile_commands.json is written at generate time, i.e. after all CMakeLists are processed,
# so it cannot be copied during configuration. A symlink is created instead: it is valid as soon
# as generation finishes and always points at the most recently configured build directory.

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(_psf_cc_source "${CMAKE_BINARY_DIR}/compile_commands.json")
set(_psf_cc_link "${CMAKE_SOURCE_DIR}/compile_commands.json")

if(NOT CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  file(REMOVE "${_psf_cc_link}")
  file(CREATE_LINK "${_psf_cc_source}" "${_psf_cc_link}" RESULT _psf_cc_result SYMBOLIC)
  if(NOT _psf_cc_result EQUAL 0)
    message(WARNING "Could not link compile_commands.json into the source root: ${_psf_cc_result}")
  endif()
endif()

unset(_psf_cc_source)
unset(_psf_cc_link)
unset(_psf_cc_result)
