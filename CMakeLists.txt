cmake_minimum_required(VERSION 3.10)
project(folly-example VERSION 0.1.0 LANGUAGES CXX)

# 设置导出编译命令
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# 引入PkgConfig模块
find_package(PkgConfig REQUIRED)

# 查找libfolly库
pkg_check_modules(FOLLY REQUIRED libfolly)

# 查找glog库
pkg_check_modules(GLOG REQUIRED libglog)

# 查找gflags库
pkg_check_modules(GFLAGS REQUIRED gflags)

# 查找libunwind库
pkg_check_modules(UNWIND REQUIRED libunwind)

# 查找libevnt库
pkg_check_modules(EVENT REQUIRED libevent)

# 查找Boost库
find_package(Boost REQUIRED COMPONENTS context)

# 查找double-conversion库
find_package(double-conversion REQUIRED)

# 查找fmt库
find_package(fmt REQUIRED)

# 包含头文件目录
include_directories(
  ${FOLLY_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置库目录
link_directories(
  ${FOLLY_LIBRARY_DIRS}
  ${GLOG_LIBRARY_DIRS}
  ${GFLAGS_LIBRARY_DIRS}
  ${UNWIND_LIBRARY_DIRS}
  ${EVENT_LIBRARY_DIRS}
)

# 查找所有源文件
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/source/*.cpp)

# 添加可执行文件目标
add_executable(folly-example ${SOURCES})

# 链接libfolly、glog、gflags、libunwind、boost和double-conversion库
target_link_libraries(folly-example
  ${FOLLY_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${UNWIND_LIBRARIES}
  ${EVENT_LIBRARIES}
  Boost::context
  double-conversion
  fmt::fmt
  iberty
)