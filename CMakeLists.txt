# cmake file

# build tool configuration

cmake_minimum_required(VERSION 3.2.0)

enable_testing()

# add project cmake module path
set(PRJ_CMAKE_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH ${PRJ_CMAKE_MODULE_DIR})

include(CMakePackageConfigHelpers)
include(Util)
include(HandleLLVMDependency)

option(PRJ_USE_LLVM_INTERNAL_MODULE TRUE)
option(PRJ_SKIP_TESTS TRUE)

get_version(VERSION PRJ_VERSION)

set(PRJ_NAME PropagateAttributes)

# part of LLVM or standalone
set(PRJ_STANDALONE_BUILD FALSE)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(${PRJ_NAME} C CXX)

  set(PRJ_STANDALONE_BUILD TRUE)

  message(STATUS "configuring as external LLVM project")
endif()


# CMP0056 is required for any libc++abi dependency of the try_compile() command
# this bumps required version to 3.2
# otherwise LD_LIBRARY_DIR needs to be set
set_policies(CMP0056 NEW)


# requirements

handle_llvm_dependency(${PRJ_STANDALONE_BUILD})


# project configuration

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

message(STATUS "RTTI support: ${LLVM_ENABLE_RTTI}")

if(NOT LLVM_ENABLE_EH)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

message(STATUS "EH support: ${LLVM_ENABLE_EH}")


# when we are building the project as standalone, we'd like to leave the install
# RPATH empty so that potentially we can dynamically link against other
# installed LLVM toolchains

if(PRJ_STANDALONE_BUILD)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  #set(CMAKE_INSTALL_RPATH ${LLVM_LIBRARY_DIRS})

  if(CMAKE_VERSION VERSION_EQUAL "3.8.0" OR CMAKE_VERSION VERSION_GREATER "3.8.0")
    set(CMAKE_BUILD_RPATH ${LLVM_LIBRARY_DIRS})
  else()
    # unfortunately this also sets the install RPATH for older cmake versions
    #set(PRJ_LINKER_FLAGS "${PRJ_LINKER_FLAGS} -Wl,-rpath ${LLVM_LIBRARY_DIRS}")
  endif()

  set(PRJ_LINKER_FLAGS "${PRJ_LINKER_FLAGS} -Wl,--disable-new-dtags")

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS 
    "${CMAKE_SHARED_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS 
    "${CMAKE_MODULE_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")
endif()

#

set(LIB_NAME "LLVM${PRJ_NAME}Pass")
set(LIB_SOURCES 
  "lib/PropagateAttributes.cpp"
  "lib/PropagateAttributesPass.cpp")

if(NOT PRJ_USE_LLVM_INTERNAL_MODULE)
  add_library(${LIB_NAME} MODULE ${LIB_SOURCES})
else()
  # this follows the suggestion documented in
  # http://llvm.org/docs/CMake.html#cmake-out-of-source-pass
  # for allowing future inclusion of the plugin in the LLVM source tree
  # by using LLVM's internal cmake functions and utilities
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib)

  add_llvm_loadable_module(${LIB_NAME} ${LIB_SOURCES})
endif()

# according to http://llvm.org/docs/CMake.html
# we do not need the below library dependencies since the plugin will be loaded
# via opt or clang which already have those libs in their dependencies
# comment out the following two statements to test against static lib LLVM build

#llvm_map_components_to_libnames(llvm_libs core support analysis ipo passes)
#target_link_libraries(${LIB_NAME} PUBLIC ${llvm_libs})

target_compile_definitions(${LIB_NAME} PUBLIC ${LLVM_DEFINITIONS})
target_compile_definitions(${LIB_NAME} PUBLIC VERSION_STRING=${PRJ_VERSION})

target_include_directories(${LIB_NAME} PUBLIC ${LLVM_INCLUDE_DIRS})
target_include_directories(${LIB_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
target_include_directories(${LIB_NAME} PUBLIC
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)

#

set(TESTEE_LIB ${LIB_NAME})
get_property(TESTEE_PREFIX TARGET ${TESTEE_LIB} PROPERTY PREFIX)
get_property(TESTEE_SUFFIX TARGET ${TESTEE_LIB} PROPERTY SUFFIX)

if(NOT TESTEE_PREFIX)
  set(TESTEE_PREFIX ${CMAKE_SHARED_MODULE_PREFIX})
endif()

if(NOT TESTEE_SUFFIX)
  set(TESTEE_SUFFIX ${CMAKE_SHARED_MODULE_SUFFIX})
endif()

add_subdirectory(unittests)
add_subdirectory(tests)
add_subdirectory(doc)


attach_compilation_db_command(${LIB_NAME})


# installation

set(PRJ_INSTALL_INCLUDE_DIR "include")
set(PRJ_INSTALL_SHARE_DIR "share")
set(PRJ_CMAKE_CONFIG_FILE "config.cmake")

if(NOT PRJ_STANDALONE_BUILD)
  set(PRJ_INSTALL_INCLUDE_DIR ${PRJ_NAME}${PRJ_INSTALL_INCLUDE_DIR})
  set(PRJ_INSTALL_SHARE_DIR ${PRJ_NAME}${PRJ_INSTALL_SHARE_DIR})
endif()

if(PRJ_STANDALONE_BUILD)
  install(TARGETS ${LIB_NAME} EXPORT ${PRJ_NAME} LIBRARY DESTINATION "lib")
endif()

string(TOLOWER ${PRJ_NAME} PRJ_NAME_LOWER)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/share/cmake/${PRJ_CMAKE_CONFIG_FILE}.in"
  "${CMAKE_CURRENT_BINARY_DIR}/share/cmake/${PRJ_NAME_LOWER}-${PRJ_CMAKE_CONFIG_FILE}"
  @ONLY)

get_version(SHORT VERSION PRJ_VERSION_SHORT)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/share/cmake/${PRJ_NAME_LOWER}-config-version.cmake"
  VERSION ${PRJ_VERSION_SHORT} COMPATIBILITY SameMajorVersion)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" 
  DESTINATION ${PRJ_INSTALL_INCLUDE_DIR})
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/share/"
  DESTINATION ${PRJ_INSTALL_SHARE_DIR})

install(EXPORT ${PRJ_NAME} DESTINATION "${PRJ_INSTALL_SHARE_DIR}/cmake")

