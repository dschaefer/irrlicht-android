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

set(ANDROID_C_FLAGS_COMMON "-fpic -mthumb-interwork -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -DANDROID -Wno-psabi")
set(ANDROID_C_FLAGS_ARM_COMMON "-g -fomit-frame-pointer -fstrict-aliasing -funswitch-loops -finline-limit=300")
set(ANDROID_C_FLAGS_ARM_V7A "-march=armv7-a -mfloat-abi=softfp -mfpu=vfp")
set(ANDROID_C_FLAGS_ARM_V5 "-march=armv5te -mtune=xscale -msoft-float")
set(ANDROID_C_FLAGS_ARM_RELEASE "-O2 ${ANDROID_C_FLAGS_ARM_COMMON}")
set(ANDROID_C_FLAGS_ARM_DEBUG "-DNDEBUG ${ANDROID_C_FLAGS_ARM_COMMON} -fno-omit-frame-pointer -fno-strict-aliasing")
set(ANDROID_C_FLAGS_THUMB_COMMON "-mthumb -g -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64")
set(ANDROID_C_FLAGS_THUMB_RELEASE "-Os ${ANDROID_C_FLAGS_THUMB_COMMON}")
set(ANDROID_C_FLAGS_THUMB_DEBUG "-DNDEBUG ${ANDROID_C_FLAGS_THUMB_COMMON} -marm -fno-omit-frame-pointer")

set(ANDROID_C_FLAGS_CODESOURCERY "-fno-builtin-sin -fno-builtin-sinf -fno-builtin-cos -fno-builtin-cosf")
set(ANDROID_C_FLAGS "${ANDROID_C_FLAGS_COMMON} ${ANDROID_C_FLAGS_ARM_RELEASE} ${ANDROID_C_FLAGS_ARM_V7A} ${ANDROID_C_FLAGS_CODESOURCERY}")
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
