cmake_minimum_required (VERSION 3.8)

option(PRINT_DIRS "Print key CMake dir variables" ON)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("RtspServer")

add_subdirectory ("RtspServer")


set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/resources)

file(GLOB RESOURCE_FILES CONFIGURE_DEPENDS "${RESOURCE_DIR}/*")

file(MAKE_DIRECTORY "${OUTPUT_DIR}")
foreach(src IN LISTS RESOURCE_FILES)
  get_filename_component(name "${src}" NAME)
  configure_file("${src}" "${OUTPUT_DIR}/${name}" COPYONLY)
endforeach()


if(PRINT_DIRS)
  include(CMakePrintHelpers)  # 自带模块
  message(STATUS "==== CMake directory variables ====")
  cmake_print_variables(
    CMAKE_SOURCE_DIR         # 顶层源码根
    CMAKE_BINARY_DIR         # 顶层构建根(你的 build/ )
    PROJECT_SOURCE_DIR       # 当前 project() 的源码根
    PROJECT_BINARY_DIR       # 当前 project() 的构建根
    CMAKE_CURRENT_SOURCE_DIR # 当前正在处理的 CMakeLists.txt 所在目录
    CMAKE_CURRENT_BINARY_DIR # 与上面对应的构建目录
    CMAKE_CURRENT_LIST_DIR   # 正在执行的这段 CMake 代码所在目录(含被 include 的 .cmake)
    RESOURCE_DIR
    OUTPUT_DIR
  )
  message(STATUS "===================================")
endif()