option(USE_DYNAMIC_LIBRARY "set use dynamic library or not" OFF)
option(USE_MINSIZEREL ".so lib will be 300KB smaller but performance will be affected" OFF)

option(USE_ANDROID_LOG "set use Android log or not" OFF)
option(USE_DEBUG "set use debug information or not" OFF)
option(USE_DETAIL "set use more debug information or not" OFF)
option(USE_PROFILE "set use profile information or not" OFF)
option(USE_PROFILE_STATISTICS "set use profile statistics information or not" OFF)
option(USE_THREAD_SAFE "set use thread safe or not" OFF)
option(USE_LITE "set use lite version or not" OFF)

# model_tools variable
option(USE_CAFFE "set use caffe model as input or not" OFF)
option(USE_ONNX "set use onnx model as input or not" OFF)
option(USE_TFLITE "set use tflite model as input or not" OFF)
option(USE_TENSORFLOW "set use tensorflow model as input or not" OFF)
option(USE_MINDSPORE "set use mindspore model as input or not" OFF)

# blas_enhance tensor
option(USE_GENERAL "set use CPU serial code or not" OFF)
option(USE_X86 "set use X86 CPU SIMD instruction or not" OFF)
option(USE_NEON "set use ARM NEON instruction or not" OFF)
option(USE_GPU "set use GPU for parallel or not" OFF)
option(USE_MATRIX "set use matrix instruction or not" OFF)
option(USE_FP32 "set use CPU FP32 SIMD instruction or not" OFF)
option(USE_FP16 "set use CPU FP16 SIMD instruction or not" OFF)
option(USE_F16_MIX_PRECISION "set use CPU mix precision f16/f32 SIMD instruction or not" ON)
option(USE_INT8 "set use CPU INT8 SIMD instruction or not" OFF)
option(USE_INT8_WINOGRAD "set use INT8 winograd" ON)
option(USE_INT32 "set use CPU INT32 instruction or not" ON)
option(USE_OPENMP "set use openmp to run test(tinybert) or not" OFF)
option(USE_X86_ARM_CONSISTENCY "set use the same computation logic on ARM and x86" OFF)

option(USE_LIBRARY_TUNING "set use algorithm tuning or not" OFF)
option(USE_MEM_CHECK "set to use memory check or not" OFF)
option(USE_MODEL_PRINT "set to use model print or not" ON)
option(USE_SECURE_C "set to use Huawei Secure C or not" OFF)

option(USE_TRAINING "set whether to use training or not" OFF)
option(USE_FLOW "set whether to use flow or not" OFF)

option(USE_API_JAVA "set whether to use Java API or not" OFF)
option(USE_API_CPP "set whether to use C++ API or not" OFF)
option(USE_API_PYTHON "set whether to use Python API or not" OFF)

option(BUILD_TEST "set to build unit test or not" ON)
option(BUILD_EXAMPLE "set to build example or not" OFF)

include(CheckCXXCompilerFlag)

if (USE_TRAINING)
    set(ANDROID_TOOLCHAIN_PREFIX "aarch64-linux-android-")
endif(USE_TRAINING)

function (set_policy)
    if (POLICY CMP0074)
        cmake_policy(SET CMP0074 NEW)
    endif()
endfunction(set_policy)

macro (set_c_cxx_flags)
    set(COMMON_FLAGS "-O3 -fPIC")
    # warning flag can be remove in release version
    set(COMMON_FLAGS "${COMMON_FLAGS} -W -Wextra")
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wno-unused-command-line-argument -Wno-unused-parameter")
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wno-unused-result -Wno-deprecated-declarations -Wno-unused-variable")
    if ((NOT CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 8.1.0) OR (NOT WIN32))
        set(COMMON_FLAGS "${COMMON_FLAGS} -fstack-protector-all")
    endif()

    if (USE_OPENMP)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_OPENMP -fopenmp")
        CHECK_CXX_COMPILER_FLAG("-static-openmp" COMPILER_SUPPORTS_STATIC_OPENMP)
        if (COMPILER_SUPPORTS_STATIC_OPENMP)
            set(COMMON_FLAGS "${COMMON_FLAGS} -static-openmp")
        endif ()
    endif(USE_OPENMP)

    if (USE_THREAD_SAFE OR USE_CAFFE OR USE_ONNX OR USE_FLOW OR WIN32)
        set(COMMON_FLAGS "${COMMON_FLAGS} -pthread")
    endif ()

    if (USE_LIBRARY_TUNING)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_LIBRARY_TUNING")
    endif(USE_LIBRARY_TUNING)

    if (BUILD_TEST)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_BUILD_TEST")
    endif ()

    if (BUILD_EXAMPLE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_BUILD_EXAMPLE")
    endif ()

    if (USE_DEBUG)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_DEBUG")
    endif()

    if (USE_LITE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_LITE")
    endif()

    if (USE_DETAIL)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_DETAIL")
    endif()

    if (USE_API_JAVA)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_API_JAVA")
    endif()

    if (USE_ANDROID_LOG)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_ANDROID_LOG -llog")
    endif(USE_ANDROID_LOG)

    if (USE_PROFILE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_PROFILE")
    endif(USE_PROFILE)

    if (USE_PROFILE_STATISTICS)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_PROFILE_STATISTICS")
    endif(USE_PROFILE_STATISTICS)

    if (USE_THREAD_SAFE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_THREAD_SAFE")
    endif(USE_THREAD_SAFE)

    if (USE_GENERAL)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_GENERAL")
    endif(USE_GENERAL)

    if (USE_GPU)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_GPU")
    endif(USE_GPU)

    if (USE_X86)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_X86 -mavx2 -mfma")
        if (cpuinfo_f16c)
            set(COMMON_FLAGS "${COMMON_FLAGS} -mf16c")
        endif ()
        if (USE_AVX512_VNNI)
            set(COMMON_FLAGS "${COMMON_FLAGS} -mavx512f")
            set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_AVX512_VNNI")
        endif(USE_AVX512_VNNI)
    endif(USE_X86)

    if (USE_INT32)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_INT32")
    endif ()

    if (USE_FP32)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_FP32")
    endif (USE_FP32)

    if (USE_FP16)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_FP16")
        if (USE_F16_MIX_PRECISION)
            set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_F16_MIX_PRECISION")
        endif (USE_F16_MIX_PRECISION)
    endif ()

    if (USE_INT8)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_INT8")
    endif (USE_INT8)

    if (USE_NEON)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_NEON")
        if (USE_FP16)
            set(arch "-march=arm")
            if (USE_MATRIX)
                set (arch "${arch}v8.2-a+sve2+bf16")
            else ()
                if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
                    set(arch "${arch}v8-a")
                else ()
                    set(arch "${arch}v8.2-a")
                endif ()
                set (arch "${arch}+fp16")
            endif ()
            if (USE_INT8)
                set(arch "${arch}+dotprod")
                if (USE_INT8_WINOGRAD AND (NOT USE_X86_ARM_CONSISTENCY))
                    set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_INT8_WINOGRAD")
                endif ()
            endif (USE_INT8)
            set(COMMON_FLAGS "${COMMON_FLAGS} ${arch}")
        else ()
            if (NOT ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
                if (NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin"))
                    set(COMMON_FLAGS "${COMMON_FLAGS} -mfp16-format=ieee")
                endif()
            endif ()
        endif (USE_FP16)
    endif(USE_NEON)

    if (USE_X86_ARM_CONSISTENCY)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_X86_ARM_CONSISTENCY")
    endif (USE_X86_ARM_CONSISTENCY)

    if (USE_MATRIX)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_MATRIX")
    endif (USE_MATRIX)

    if (USE_CAFFE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_CAFFE")
    endif()
    if (USE_ONNX)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_ONNX")
    endif()
    if (USE_TFLITE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_TFLITE")
    endif()
    if (USE_TENSORFLOW)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_TENSORFLOW")
    endif()
    if (USE_MINDSPORE)
	    set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_MINDSPORE")
    endif()    

    if (USE_MEM_CHECK)
	    set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_MEM_CHECK")
    endif()

    if (USE_MODEL_PRINT)
	    set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_MODEL_PRINT")
    endif()

    if (USE_SECURE_C)
	    set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_SECURE_C")
    endif()

    if (NOT USE_DYNAMIC_LIBRARY)
        set(COMMON_FLAGS "${COMMON_FLAGS} -static-libstdc++")
    endif()

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -std=gnu99")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=c++11")
    
    if (USE_DEBUG)
        set(CMAKE_BUILD_TYPE "Debug")
    elseif (USE_MINSIZEREL)
        set(CMAKE_BUILD_TYPE "MinSizeRel")
    endif (USE_DEBUG)
endmacro(set_c_cxx_flags)

macro (set_test_c_cxx_flags)
    if (NOT USE_DYNAMIC_LIBRARY)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(COMMON_FLAGS "${COMMON_FLAGS} -static-libgcc")
            if (NOT "${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "${CMAKE_SYSTEM_PROCESSOR}")
                set(COMMON_FLAGS "${COMMON_FLAGS} -static")
            endif()
            if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
                set(COMMON_FLAGS "${COMMON_FLAGS} -static")
            endif()
        endif()
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND NOT APPLE AND NOT WIN32)
        set(COMMON_FLAGS "${COMMON_FLAGS} -Wl,-allow-shlib-undefined")
    endif()

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
endmacro (set_test_c_cxx_flags)

macro (set_project_install_directory)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
endmacro (set_project_install_directory)

if(USE_DYNAMIC_LIBRARY)
    set(uni_library uni)
    set(model_spec_library model_spec)
    set(gcl_library gcl)
    set(kernelsource_library kernelsource)
    set(blas_enhance_library blas_enhance)
    set(tensor_library tensor)
    set(image_library image)
    set(model_tools_caffe_library model_tools_caffe)
    set(model_tools_onnx_library model_tools_onnx)
    set(model_tools_tflite_library model_tools_tflite)
    set(model_tools_tensorflow_library model_tools_tensorflow)
    set(model_tools_mindspore_library model_tools_mindspore)
    set(model_tools_library model_tools)
    set(engine_library engine)
    set(flow_library flow)
else()
    set(uni_library uni_static)
    set(model_spec_library model_spec_static)
    set(gcl_library gcl_static)
    set(kernelsource_library kernelsource_static)
    set(blas_enhance_library blas_enhance_static)
    set(tensor_library tensor_static)
    set(image_library image_static)
    set(model_tools_caffe_library model_tools_caffe_static)
    set(model_tools_onnx_library model_tools_onnx_static)
    set(model_tools_tflite_library model_tools_tflite_static)
    set(model_tools_tensorflow_library model_tools_tensorflow_static)
    set(model_tools_mindspore_library model_tools_mindspore_static)
    set(model_tools_library model_tools_static)
    set(engine_library engine_static)
    set(flow_library flow_static)
endif()

macro(link_system_static name)
    if (NOT USE_DYNAMIC_LIBRARY)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            target_link_libraries(${name} -static-libgcc)
            if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
                target_link_libraries(${name} -static)
            endif()
        endif()
    endif()
endmacro()

macro(include_securec)
    if (USE_SECURE_C)
        include_directories(${SecureC_ROOT}/include)
    endif ()
endmacro()
macro(link_securec name)
    if (USE_SECURE_C)
        target_link_libraries(${name} ${SecureC_LIBRARY})
    endif ()
endmacro()

macro(include_uni)
    include_directories(${BOLT_ROOT}/common/uni/include)
    include_securec()
endmacro()
macro(link_uni name)
    target_link_libraries(${name} ${uni_library})
    link_securec(${name})
endmacro()
macro(link_uni_shared name)
    target_link_libraries(${name} uni)
    link_securec(${name})
endmacro()

macro(include_model_spec)
    include_directories(${BOLT_ROOT}/common/model_spec/include)
    include_memory()
    include_uni()
endmacro()
macro(link_model_spec name)
    target_link_libraries(${name} ${model_spec_library})
    link_uni(${name})
endmacro()

macro(include_gcl)
    include_directories(${BOLT_ROOT}/compute/tensor/src/gpu/mali/cl/kernel_option)
    include_directories(${BOLT_ROOT}/common/memory/include)
    include_directories(${BOLT_ROOT}/common/gcl/include)
    include_directories(${BOLT_ROOT}/common/gcl/tools/kernel_source_compile/include)
    include_directories(${OPENCL_INCLUDE_DIR})
    include_uni()
endmacro()

macro(link_opencl name)
    if (USE_GPU)
        target_link_libraries(${name} ${OPENCL_LIBRARIES})
    endif(USE_GPU)
endmacro()

macro(link_gcl name)
    if (USE_GPU)
        target_link_libraries(${name} ${gcl_library} ${kernelsource_library})
        if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
            target_link_libraries(${name} dl)
        endif()
        link_opencl(${name})
    endif (USE_GPU)
endmacro()

macro(include_memory)
    include_directories(${BOLT_ROOT}/common/memory/include)
    include_uni()
    include_gcl()
endmacro()

macro(include_blas_enhance)
    include_directories(${BOLT_ROOT}/compute/blas_enhance/include)
    include_uni()
    include_memory()
endmacro()
macro(link_blas_enhance name)
    target_link_libraries(${name} ${blas_enhance_library})
    link_uni(${name})
endmacro()

macro(include_tensor)
    include_directories(${BOLT_ROOT}/compute/tensor/include)
    include_blas_enhance()
    include_gcl()
    include_memory()
endmacro()
macro(link_tensor name)
    target_link_libraries(${name} ${tensor_library} ${blas_enhance_library})
    link_blas_enhance(${name})
    link_gcl(${name})
endmacro()

macro(include_image)
    include_directories(${BOLT_ROOT}/compute/image/include)
    include_tensor()
endmacro()
macro(link_image name)
    target_link_libraries(${name} ${image_library})
    link_tensor(${name})
endmacro()

macro(include_protobuf)
    include_directories(${Protobuf_INCLUDE_DIR})
endmacro()
macro(link_protobuf name)
    target_link_libraries(${name} ${Protobuf_LIBRARY})
    if (ANDROID)
        target_link_libraries(${name} -llog)
    endif()
endmacro()
macro(include_model_tools)
    include_directories(${BOLT_ROOT}/model_tools/include)
    include_model_spec()
    include_uni()
endmacro()
macro(link_model_tools_shared name)
    target_link_libraries(${name} model_tools)
    if(USE_CAFFE)
        target_link_libraries(${name} model_tools_caffe)
    endif()
    if(USE_ONNX)
        target_link_libraries(${name} model_tools_onnx)
    endif()
    if(USE_TFLITE)
        target_link_libraries(${name} model_tools_tflite)
    endif()
    if(USE_TENSORFLOW)
        target_link_libraries(${name} model_tools_tensorflow)
        target_link_libraries(${name} ${JSONCPP_SHARED_LIBRARY})
    endif()
    if(USE_MINDSPORE)
        target_link_libraries(${name} model_tools_mindspore)
    endif()
    if(USE_CAFFE OR USE_ONNX OR USE_MINDSPORE)
        target_link_libraries(${name} ${Protobuf_SHARED_LIBRARY})
    endif()
    target_link_libraries(${name} model_spec uni)
endmacro()
macro(link_model_tools name)
    target_link_libraries(${name} ${model_tools_library})
    if(USE_CAFFE)
        target_link_libraries(${name} ${model_tools_caffe_library})
    endif()
    if(USE_ONNX)
        target_link_libraries(${name} ${model_tools_onnx_library})
    endif()
    if(USE_TFLITE)
        target_link_libraries(${name} ${model_tools_tflite_library})
    endif()
    if(USE_TENSORFLOW)
        target_link_libraries(${name} ${model_tools_tensorflow_library})
        target_link_libraries(${name} ${JSONCPP_LIBRARY})
    endif()
    if(USE_MINDSPORE)
        target_link_libraries(${name} ${model_tools_mindspore_library})
    endif()
    if(USE_CAFFE OR USE_ONNX OR USE_MINDSPORE)
        link_protobuf(${name})
    endif()
    link_model_spec(${name})
    link_uni(${name})
endmacro()
macro(model_tools_test name src_name)
    include_directories(${BOLT_ROOT}/model_tools/include)
    add_executable(${name} ${src_name})
    link_model_tools(${name})
endmacro()

macro(include_engine)
    if (BUILD_EXAMPLE)
        include_directories(${JPEG_INCLUDE_DIR})
        include_directories(${OpenCV_INCLUDE_DIRS})
    endif ()
    include_directories(${BOLT_ROOT}/inference/engine/include)
    if (USE_API_JAVA)
        include_directories(${JNI_INCLUDE_DIR})
        include_directories(${JNI_MD_INCLUDE_DIR})
    endif ()
    include_model_tools()
    include_tensor()
    include_image()
endmacro()
macro(link_engine name)
    target_link_libraries(${name} ${engine_library})
    if (BUILD_EXAMPLE)
        target_link_libraries(${name} ${JPEG_LIBRARY})
    endif ()
    if (BUILD_EXAMPLE AND (${name} STREQUAL "ultra_face" OR ${name} STREQUAL "u2net"))
        target_link_libraries(${name} ${OpenCV_LIBS})
    endif ()
    link_model_spec(${name})
    target_link_libraries(${name} ${image_library} ${tensor_library} ${blas_enhance_library})
    link_gcl(${name})
    link_uni(${name})
endmacro()
macro(engine_test name src_name)
    include_engine()
    add_executable(${name} ${src_name})
    link_engine(${name})
endmacro()

macro(include_flow)
    include_directories(${BOLT_ROOT}/inference/flow/include)
    include_engine()
endmacro()
macro(flow_test name src_name)
    include_protobuf()
    include_directories(${BOLT_ROOT}/flow/include)
    if ("${name}" STREQUAL "flow_asr")
        set_policy()
        find_package(FFTS)
        add_executable(${name} ${src_name})
        target_link_libraries(${name} ${FFTS_LIBRARIES})
    else ()
        add_executable(${name} ${src_name})
    endif()
    target_link_libraries(${name} ${flow_library})
    link_engine(${name})
    link_protobuf(${name})
    add_dependencies(${name} flow.pb.h)
endmacro()

macro(include_train)
    include_model_tools()
    include_tensor()
    include_image()
endmacro()
macro(link_train name)
    target_link_libraries(${name} RaulLib)
    link_model_tools(${name})
    target_link_libraries(${name} ${image_library} ${tensor_library} ${blas_enhance_library})
    link_gcl(${name})
    link_uni(${name})
endmacro()
macro(train_test name src_name)
    include_directories(${BOLT_ROOT}/training/include)
    include_directories(${BOLT_ROOT}/training/src)
    add_executable(${name} ${src_name})
    link_train(${name})
endmacro()
