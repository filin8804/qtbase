

#### Inputs



#### Libraries

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES TYPE OPTIONAL)
find_package(Libudev)
set_package_properties(Libudev PROPERTIES TYPE OPTIONAL)


#### Tests

# reduce_relocations
qt_config_compile_test(reduce_relocations
    LABEL "-Bsymbolic-functions support"
"#if !(defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__) || defined(__amd64))
#  error Symbolic function binding on this architecture may be broken, disabling it (see QTBUG-36129).
#endif


int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */

    /* END TEST: */
    return 0;
}
"# FIXME: qmake: ['TEMPLATE = lib', 'CONFIG += dll bsymbolic_functions', 'isEmpty(QMAKE_LFLAGS_BSYMBOLIC_FUNC): error("Nope")']
)

# sse2
qt_config_compile_test_x86simd(sse2 "SSE2 instructions")

# sse3
qt_config_compile_test_x86simd(sse3 "SSE3 instructions")

# ssse3
qt_config_compile_test_x86simd(ssse3 "SSSE3 instructions")

# sse4_1
qt_config_compile_test_x86simd(sse4_1 "SSE4.1 instructions")

# sse4_2
qt_config_compile_test_x86simd(sse4_2 "SSE4.2 instructions")

# aesni
qt_config_compile_test_x86simd(aesni "AES new instructions")

# f16c
qt_config_compile_test_x86simd(f16c "F16C instructions")

# rdrnd
qt_config_compile_test_x86simd(rdrnd "RDRAND instruction")

# shani
qt_config_compile_test_x86simd(shani "SHA new instructions")

# avx
qt_config_compile_test_x86simd(avx "AVX instructions")

# avx2
qt_config_compile_test_x86simd(avx2 "AVX2 instructions")

# avx512f
qt_config_compile_test_x86simd(avx512f "AVX512 F instructions")

# avx512er
qt_config_compile_test_x86simd(avx512er "AVX512 ER instructions")

# avx512cd
qt_config_compile_test_x86simd(avx512cd "AVX512 CD instructions")

# avx512pf
qt_config_compile_test_x86simd(avx512pf "AVX512 PF instructions")

# avx512dq
qt_config_compile_test_x86simd(avx512dq "AVX512 DQ instructions")

# avx512bw
qt_config_compile_test_x86simd(avx512bw "AVX512 BW instructions")

# avx512vl
qt_config_compile_test_x86simd(avx512vl "AVX512 VL instructions")

# avx512ifma
qt_config_compile_test_x86simd(avx512ifma "AVX512 IFMA instructions")

# avx512vbmi
qt_config_compile_test_x86simd(avx512vbmi "AVX512 VBMI instructions")

# posix_fallocate
qt_config_compile_test(posix_fallocate
    LABEL "POSIX fallocate()"
"
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
(void) posix_fallocate(0, 0, 0);
    /* END TEST: */
    return 0;
}
")

# alloca_stdlib_h
qt_config_compile_test(alloca_stdlib_h
    LABEL "alloca() in stdlib.h"
"
#include <stdlib.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
alloca(1);
    /* END TEST: */
    return 0;
}
")

# alloca_h
qt_config_compile_test(alloca_h
    LABEL "alloca() in alloca.h"
"
#include <alloca.h>
#ifdef __QNXNTO__
// extra include needed in QNX7 to define NULL for the alloca() macro
#  include <stddef.h>
#endif
int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
alloca(1);
    /* END TEST: */
    return 0;
}
")

# alloca_malloc_h
qt_config_compile_test(alloca_malloc_h
    LABEL "alloca() in malloc.h"
"
#include <malloc.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */
alloca(1);
    /* END TEST: */
    return 0;
}
")

# stack_protector
qt_config_compile_test(stack_protector
    LABEL "stack protection"
"#ifdef __QNXNTO__
#  include <sys/neutrino.h>
#  if _NTO_VERSION < 700
#    error stack-protector not used (by default) before QNX 7.0.0.
#  endif
#endif


int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* BEGIN TEST: */

    /* END TEST: */
    return 0;
}
"# FIXME: qmake: QMAKE_CXXFLAGS += -fstack-protector-strong
)



#### Features

# This belongs into gui, but the license check needs it here already.
qt_feature("android_style_assets" PRIVATE
    LABEL "Android Style Assets"
    CONDITION ANDROID
)
qt_feature("developer_build"
    LABEL "Developer build"
    AUTODETECT OFF
)
qt_feature("private_tests" PRIVATE
    LABEL "Developer build: private_tests"
    AUTODETECT OFF
)
qt_feature_definition("developer_build" "QT_BUILD_INTERNAL")
qt_feature("appstore_compliant" PUBLIC
    LABEL "App store compliance"
    PURPOSE "Disables code that is not allowed in platform app stores"
    AUTODETECT APPLE_UIKIT OR ANDROID OR WINRT
)
qt_feature("simulator_and_device" PUBLIC
    LABEL "Build for both simulator and device"
    CONDITION APPLE_UIKIT AND INPUT_sdk STREQUAL ''
)
qt_feature("force_asserts" PUBLIC
    LABEL "Force assertions"
    AUTODETECT OFF
)
qt_feature("framework" PUBLIC
    LABEL "Build Apple Frameworks"
    CONDITION APPLE AND BUILD_SHARED_LIBS
)
qt_feature_definition("framework" "QT_MAC_FRAMEWORK_BUILD")
qt_feature("largefile"
    LABEL "Large file support"
    CONDITION NOT ANDROID AND NOT INTEGRITY AND NOT WINRT
)
qt_feature_definition("largefile" "QT_LARGEFILE_SUPPORT" VALUE "64")
qt_feature("reduce_exports" PRIVATE
    LABEL "Reduce amount of exported symbols"
    CONDITION NOT WIN32 AND CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY
)
qt_feature_definition("reduce_exports" "QT_VISIBILITY_AVAILABLE")
qt_feature("sse2" PRIVATE
    LABEL "SSE2"
    CONDITION ( ( TEST_architecture_arch STREQUAL i386 ) OR ( TEST_architecture_arch STREQUAL x86_64 ) ) AND TEST_subarch_sse2
)
qt_feature_definition("sse2" "QT_COMPILER_SUPPORTS_SSE2" VALUE "1")
qt_feature("sse3"
    LABEL "SSE3"
    CONDITION QT_FEATURE_sse2 AND TEST_subarch_sse3
)
qt_feature_definition("sse3" "QT_COMPILER_SUPPORTS_SSE3" VALUE "1")
qt_feature("ssse3"
    LABEL "SSSE3"
    CONDITION QT_FEATURE_sse3 AND TEST_subarch_ssse3
)
qt_feature_definition("ssse3" "QT_COMPILER_SUPPORTS_SSSE3" VALUE "1")
qt_feature("sse4_1"
    LABEL "SSE4.1"
    CONDITION QT_FEATURE_ssse3 AND TEST_subarch_sse4_1
)
qt_feature_definition("sse4_1" "QT_COMPILER_SUPPORTS_SSE4_1" VALUE "1")
qt_feature("sse4_2"
    LABEL "SSE4.2"
    CONDITION QT_FEATURE_sse4_1 AND TEST_subarch_sse4_2
)
qt_feature_definition("sse4_2" "QT_COMPILER_SUPPORTS_SSE4_2" VALUE "1")
qt_feature("avx"
    LABEL "AVX"
    CONDITION QT_FEATURE_sse4_2 AND TEST_subarch_avx
)
qt_feature_definition("avx" "QT_COMPILER_SUPPORTS_AVX" VALUE "1")
qt_feature("f16c"
    LABEL "F16C"
    CONDITION QT_FEATURE_avx AND TEST_subarch_f16c
)
qt_feature_definition("f16c" "QT_COMPILER_SUPPORTS_F16C" VALUE "1")
qt_feature("avx2" PRIVATE
    LABEL "AVX2"
    CONDITION QT_FEATURE_avx AND TEST_subarch_avx2
)
qt_feature_definition("avx2" "QT_COMPILER_SUPPORTS_AVX2" VALUE "1")
qt_feature("avx512f"
    LABEL "F"
    CONDITION QT_FEATURE_avx2 AND TEST_subarch_avx512f
)
qt_feature_definition("avx512f" "QT_COMPILER_SUPPORTS_AVX512F" VALUE "1")
qt_feature("avx512er"
    LABEL "ER"
    CONDITION QT_FEATURE_avx512f AND TEST_subarch_avx512er
)
qt_feature_definition("avx512er" "QT_COMPILER_SUPPORTS_AVX512ER" VALUE "1")
qt_feature("avx512cd"
    LABEL "CD"
    CONDITION QT_FEATURE_avx512f AND TEST_subarch_avx512cd
)
qt_feature_definition("avx512cd" "QT_COMPILER_SUPPORTS_AVX512CD" VALUE "1")
qt_feature("avx512pf"
    LABEL "PF"
    CONDITION QT_FEATURE_avx512f AND TEST_subarch_avx512pf
)
qt_feature_definition("avx512pf" "QT_COMPILER_SUPPORTS_AVX512PF" VALUE "1")
qt_feature("avx512dq"
    LABEL "DQ"
    CONDITION QT_FEATURE_avx512f AND TEST_subarch_avx512dq
)
qt_feature_definition("avx512dq" "QT_COMPILER_SUPPORTS_AVX512DQ" VALUE "1")
qt_feature("avx512bw"
    LABEL "BW"
    CONDITION QT_FEATURE_avx512f AND TEST_subarch_avx512bw
)
qt_feature_definition("avx512bw" "QT_COMPILER_SUPPORTS_AVX512BW" VALUE "1")
qt_feature("avx512vl"
    LABEL "VL"
    CONDITION QT_FEATURE_avx512f AND TEST_subarch_avx512vl
)
qt_feature_definition("avx512vl" "QT_COMPILER_SUPPORTS_AVX512VL" VALUE "1")
qt_feature("avx512ifma"
    LABEL "IFMA"
    CONDITION QT_FEATURE_avx512f AND TEST_subarch_avx512ifma
)
qt_feature_definition("avx512ifma" "QT_COMPILER_SUPPORTS_AVX512IFMA" VALUE "1")
qt_feature("avx512vbmi"
    LABEL "VBMI"
    CONDITION QT_FEATURE_avx512f AND TEST_subarch_avx512vbmi
)
qt_feature_definition("avx512vbmi" "QT_COMPILER_SUPPORTS_AVX512VBMI" VALUE "1")
qt_feature("aesni"
    LABEL "AES"
    CONDITION QT_FEATURE_sse2 AND TEST_subarch_aes
)
qt_feature_definition("aesni" "QT_COMPILER_SUPPORTS_AES" VALUE "1")
qt_feature("rdrnd"
    LABEL "RDRAND"
    CONDITION TEST_subarch_rdseed
)
qt_feature_definition("rdrnd" "QT_COMPILER_SUPPORTS_RDRND" VALUE "1")
qt_feature("shani"
    LABEL "SHA"
    CONDITION QT_FEATURE_sse2 AND TEST_subarch_sha
)
qt_feature_definition("shani" "QT_COMPILER_SUPPORTS_SHA" VALUE "1")
qt_feature("x86SimdAlways"
    LABEL "Intrinsics without -mXXX option"
    CONDITION ( ( TEST_architecture_arch STREQUAL i386 ) OR ( TEST_architecture_arch STREQUAL x86_64 ) ) AND ON
)
qt_feature_definition("x86SimdAlways" "QT_COMPILER_SUPPORTS_SIMD_ALWAYS" VALUE "1")
qt_feature("mips_dsp"
    LABEL "DSP"
    CONDITION ( TEST_architecture_arch STREQUAL mips ) AND TEST_subarch_dsp
)
qt_feature_definition("mips_dsp" "QT_COMPILER_SUPPORTS_MIPS_DSP" VALUE "1")
qt_feature("mips_dspr2"
    LABEL "DSPr2"
    CONDITION ( TEST_architecture_arch STREQUAL mips ) AND TEST_subarch_dspr2
)
qt_feature_definition("mips_dspr2" "QT_COMPILER_SUPPORTS_MIPS_DSPR2" VALUE "1")
qt_feature("neon"
    LABEL "NEON"
    CONDITION ( ( TEST_architecture_arch STREQUAL arm ) OR ( TEST_architecture_arch STREQUAL arm64 ) ) AND TEST_subarch_neon
)
qt_feature_definition("neon" "QT_COMPILER_SUPPORTS_NEON" VALUE "1")
qt_feature("alloca_h" PRIVATE
    LABEL "alloca.h"
    CONDITION TEST_alloca_h
)
qt_feature("alloca_malloc_h" PRIVATE
    LABEL "alloca() in malloc.h"
    CONDITION NOT QT_FEATURE_alloca_h AND TEST_alloca_malloc_h
)
qt_feature("alloca" PRIVATE
    LABEL "alloca()"
    CONDITION QT_FEATURE_alloca_h OR QT_FEATURE_alloca_malloc_h OR TEST_alloca_stdlib_h
)
qt_feature("thread" PUBLIC
    SECTION "Kernel"
    LABEL "Thread support"
    PURPOSE "Provides QThread and related classes."
)
qt_feature("future" PUBLIC
    SECTION "Kernel"
    LABEL "QFuture"
    PURPOSE "Provides QFuture and related classes."
    CONDITION QT_FEATURE_thread
)
qt_feature("concurrent" PUBLIC
    SECTION "Kernel"
    LABEL "Qt Concurrent"
    PURPOSE "Provides a high-level multi-threading API."
    CONDITION QT_FEATURE_future
)
qt_feature_definition("concurrent" "QT_NO_CONCURRENT" NEGATE VALUE "1")
qt_feature("dbus" PUBLIC PRIVATE
    LABEL "Qt D-Bus"
    AUTODETECT NOT APPLE_UIKIT AND NOT ANDROID AND NOT WINRT
    CONDITION QT_FEATURE_thread
)
qt_feature_definition("dbus" "QT_NO_DBUS" NEGATE VALUE "1")
qt_feature("dbus_linked" PRIVATE
    LABEL "Qt D-Bus directly linked to libdbus"
    CONDITION QT_FEATURE_dbus AND libs.dbus OR FIXME
    ENABLE INPUT_dbus STREQUAL 'linked'
    DISABLE INPUT_dbus STREQUAL 'runtime'
)
qt_feature("gui" PRIVATE
    LABEL "Qt Gui"
)
qt_feature("network" PRIVATE
    LABEL "Qt Network"
)
qt_feature("sql" PRIVATE
    LABEL "Qt Sql"
    CONDITION QT_FEATURE_thread
)
qt_feature("testlib" PRIVATE
    LABEL "Qt Testlib"
)
qt_feature("widgets" PRIVATE
    LABEL "Qt Widgets"
    AUTODETECT NOT APPLE_TVOS AND NOT APPLE_WATCHOS
    CONDITION QT_FEATURE_gui
)
qt_feature_definition("widgets" "QT_NO_WIDGETS" NEGATE)
qt_feature("xml" PRIVATE
    LABEL "Qt Xml"
)
qt_feature("libudev" PRIVATE
    LABEL "udev"
    CONDITION Libudev_FOUND
)

qt_extra_definition("QT_VERSION_STR" "\"${PROJECT_VERSION}\"" PUBLIC)
qt_extra_definition("QT_VERSION_MAJOR" ${PROJECT_VERSION_MAJOR} PUBLIC)
qt_extra_definition("QT_VERSION_MINOR" ${PROJECT_VERSION_MINOR} PUBLIC)
qt_extra_definition("QT_VERSION_PATCH" ${PROJECT_VERSION_PATCH} PUBLIC)
