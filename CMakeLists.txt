cmake_minimum_required(VERSION 3.6)

project(AfmCamera)

# Debug specifics
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

# Release specifics
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

# Configuration specific defines
include(configuration/${AFM_CONFIG}.cmake)

set (CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

set (USERLAND_ROOT /opt/vc/include)
set (INTERFACE_ROOT ${USERLAND_ROOT}/interface)
set (MMAL_INCLUDE ${USERLAND_ROOT}/interface/mmal)
set (EGL_ROOT ${USERLAND_ROOT}/host_applications/linux/apps/raspicam)

# Output for install
set (LIB_DIR ${CMAKE_BINARY_DIR}/staging/lib)
set (BIN_DIR ${CMAKE_BINARY_DIR}/staging/bin)

# Set --no-as-needed to stop the linker discarding mmal_vc_client
# as it can't see that the constructor registers a load of functionality
# with the MMAL core.
SET( CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed" )

include_directories (
    .
    internal
    ${GTK3_INCLUDE_DIRS}
    ${USERLAND_ROOT}
    ${INTERFACE_ROOT}
    ${USERLAND_ROOT}/host_applications/linux/libs/bcm_host/include
    ${USERLAND_ROOT}/host_applications/linux/libs/sm
    ${MMAL_INCLUDE}
    ${EGL_ROOT}
)

set (MMAL_LIBS
    mmal_core
    mmal_util
    mmal_vc_client
)

set (EGL_LIBS
    brcmGLESv2
    brcmEGL
)

set(SRC_FILES
    src/Camera.cpp
    src/handlers.cpp
    src/main.cpp
    src/Port.cpp
    src/PortFactory.cpp
    src/I2C.cpp
    src/SPI.cpp
    src/GPIO.cpp
    src/Table.cpp
    src/ImageSettings.cpp
    src/Database.cpp
    src/Persistence.cpp
    src/App.cpp
    src/AfmWindow.cpp
    src/AboutDialog.cpp
    src/PreferencesDialog.cpp
    src/MainWindow.cpp
    src/sdl_renderer.c
    src/util.cpp
)

add_executable (AfmCamera ${SRC_FILES} ${COMMON_SOURCES} ${EGL_SOURCES} ${GL_SCENE_SOURCES})

target_compile_options(
    AfmCamera
    PUBLIC ${GTK3_CFLAGS_OTHER}
)

target_link_directories(
    AfmCamera
    PUBLIC
    /opt/vc/lib/
)

target_link_libraries(
    AfmCamera
    -export-dynamic
    ${GTK3_LIBRARIES}
    ${EGL_LIBS}
    ${MMAL_LIBS}
    vcos
    bcm_host
    sqlite3
    X11
    m
    dl
    SDL2
    pthread
)

install (TARGETS AfmCamera DESTINATION ${BIN_DIR})


