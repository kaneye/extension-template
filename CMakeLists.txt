cmake_minimum_required(VERSION 3.5)

# Set extension name here
set(TARGET_NAME quack)
#set(TARGET_NAME kdb)

# DuckDB's extension distribution supports vcpkg. As such, dependencies can be added in ./vcpkg.json and then
# used in cmake with find_package. Feel free to remove or replace with other dependencies.
# Note that it should also be removed from vcpkg.json to prevent needlessly installing it..
find_package(OpenSSL REQUIRED)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})
include_directories(src/include)

#set(EXTENSION_SOURCES src/quack_extension.cpp)
set(EXTENSION_SOURCES 
  src/kdb_extension.cpp
  src/kdb_reader/kdbreader.cpp
  src/kdb_reader/inputstream.cpp
  src/kdb_reader/qfile.cpp
  src/kdb_reader/utils.cpp
  src/kdb_reader/readstring.cpp
)


build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

# Link OpenSSL in both the static library as the loadable extension
#target_link_libraries(${EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)
#target_link_libraries(${LOADABLE_EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto z)
target_link_libraries(${LOADABLE_EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto z)

install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
