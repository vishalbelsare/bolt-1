find_path(JNI_INCLUDE_DIR NAMES jni.h HINTS $ENV{JNI_ROOT}/include ${JNI_ROOT}/include $ENV{JNI_ROOT}/include/linux ${JNI_ROOT}/include/linux ${JNI_ROOT}/include/win32)
find_path(JNI_MD_INCLUDE_DIR NAMES jni_md.h HINTS $ENV{JNI_ROOT}/include ${JNI_ROOT}/include $ENV{JNI_ROOT}/include/linux ${JNI_ROOT}/include/linux ${JNI_ROOT}/include/win32)

if (JNI_INCLUDE_DIR AND JNI_MD_INCLUDE_DIR)
    set(JNI_FOUND true)
else (JNI_INCLUDE_DIR AND JNI_MD_INCLUDE_DIR)
    set(JNI_FOUND false)
endif (JNI_INCLUDE_DIR AND JNI_MD_INCLUDE_DIR)

if (JNI_FOUND)
    message(STATUS "Found jni.h: ${JNI_INCLUDE_DIR}")
else (JNI_FOUND)
    if (NOT JNI_ROOT)
        set(JNI_ROOT $ENV{JNI_ROOT})
    endif()
    message(WARNING 
    " JNI_ROOT: ${JNI_ROOT}\n"
    " Can not find jni.h/jni_md.h in <JNI_ROOT>/include, <JNI_ROOT>/include/linux or <JNI_ROOT>/include/win32 directory, so can not use Java API.\n"
    " If you want to use Java API, please set shell or cmake environment variable JNI_ROOT.\n"
    " Currently Java package name is com.huawei.noah. If you want to change it, you can modify BOLT_JNI_PREFIX_(x) macro in inference/engine/include/jni_header.h.")
endif (JNI_FOUND)
