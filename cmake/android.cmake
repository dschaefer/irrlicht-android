include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm-eabi)

set(CMAKE_CROSSCOMPILING 1)

set(ANDROID 1)

set(SDKROOT "$ENV{ANDROID_NDK_ROOT}/build/platforms/android-8/arch-arm")
#set(TOOLSROOT "$ENV{ANDROID_NDK_ROOT}/build/prebuilt/windows/arm-eabi-4.4.0")
set(TOOLSROOT "$ENV{ANDROID_NDK_ROOT}/build/prebuilt/windows/arm-2010q1")
set(PREFIX "arm-none-linux-gnueabi-")

CMAKE_FORCE_C_COMPILER("${TOOLSROOT}/bin/${PREFIX}gcc.exe" GNU)
CMAKE_FORCE_CXX_COMPILER("${TOOLSROOT}/bin/${PREFIX}g++.exe" GNU)

set(ANDROID_C_FLAGS "-fno-builtin-sin -fno-builtin-cos -fno-builtin-sinf -fno-builtin-cosf -fpic -mthumb-interwork -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -Wno-psabi -march=armv5te -mtune=xscale -msoft-float -mthumb -Os -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -DANDROID -Wa,--noexecstack -O2 -DNDEBUG -g")
set(ANDROID_CXX_FLAGS "${ANDROID_C_FLAGS} -fno-exceptions -fno-rtti")

set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib -Wl,--no-undefined")

set(CMAKE_C_LINK_FLAGS "-nostdlib")
set(CMAKE_CXX_LINK_FLAGS "-nostdlib")

include_directories("${SDKROOT}/usr/include")

link_directories("${SDKROOT}/usr/lib")

set(CMAKE_FIND_ROOT_PATH "${SDKROOT}" "${TOOLSROOT}" )
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
