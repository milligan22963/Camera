set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_SYSTEM_PROCESSOR amd64)

set(TOOL_CHAIN_PATH /usr/bin/)

set(TOOL_CHAIN_PREFIX )

add_definitions(-D_GNU_SOURCE)

# Global include paths
include_directories(host_applications/framework)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(interface/vcos/pthreads)
include_directories(interface/vmcs_host/linux)
include_directories(interface/vmcs_host)
include_directories(interface/vmcs_host/khronos)
include_directories(interface/khronos/include)
include_directories(${PROJECT_BINARY_DIR})
include_directories(interface/vchiq_arm)
#include_directories(tools/inet_transport)
include_directories(host_support/include)
