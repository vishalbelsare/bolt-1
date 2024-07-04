// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _H_PLATFORM
#define _H_PLATFORM

#include <string>
#include "file.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    VENDOR_ARM = 0,
    VENDOR_QUALCOMM = 1,
    VENDOR_INTEL = 2,
    VENDOR_NVIDIA = 3,
} PlatformVendor;

inline EE get_platform_info(Platform platform, cl_platform_info info, void **value, U32 *len)
{
    if (NULL == value) {
        return NULL_POINTER;
    }
    size_t size;
    I32 ret = clGetPlatformInfo(platform, info, 0, NULL, &size);
    if (CL_SUCCESS == ret) {
        if (len != NULL) {
            *len = size;
        }
        char *data = (char *)malloc(size + 1);
        if (NULL == data) {
            return ALLOC_FAILED;
        }
        data[size] = '\0';
        ret = clGetPlatformInfo(platform, info, size + 1, data, NULL);
        if (CL_SUCCESS != ret) {
            free(data);
        } else {
            *value = data;
        }
    }
    map_cl_error_2_ee(ret);
}

inline EE get_platforms(U32 *num, Platform **platforms)
{
    if (NULL == platforms || NULL == num) {
        return NULL_POINTER;
    }
    I32 ret = clGetPlatformIDs(0, NULL, num);
    UNI_DETAIL_LOG("platform num:%d.\n", *num);
    if (CL_SUCCESS == ret) {
        Platform *p = (Platform *)malloc((*num) * sizeof(Platform));
        if (NULL == p) {
            return ALLOC_FAILED;
        }
        ret = clGetPlatformIDs(*num, p, NULL);
        if (CL_SUCCESS != ret) {
            free(p);
        } else {
            *platforms = p;
#if _DEBUG
            for (U32 i = 0; i < *num; i++) {
                char *name = NULL;
                CHECK_STATUS(get_platform_info(p[i], CL_PLATFORM_NAME, (void **)&name, NULL));
                UNI_DETAIL_LOG("platform %d is %s.\n", i, name);
                if (name != NULL) {
                    free(name);
                }
            }
#endif
        }
    }
    map_cl_error_2_ee(ret);
}

inline EE select_platform(PlatformVendor vendor, Platform *platform)
{
    if (NULL == platform) {
        return NULL_POINTER;
    }
    const static char *key[] = {"arm", "qualcomm", "intel"};
    U32 num_platforms = 0;
    Platform *platforms = NULL;
    EE ret = get_platforms(&num_platforms, &platforms);
    if (SUCCESS == ret) {
        const char *platform_vendor = key[vendor];
        for (U32 i = 0; i < num_platforms; i++) {
            Platform p = platforms[i];
            char *name = NULL;
            ret = get_platform_info(p, CL_PLATFORM_NAME, (void **)&name, NULL);
            UNI_DETAIL_LOG("platform %d is %s.\n", i, name);
            if (SUCCESS == ret) {
                if (contains(name, platform_vendor)) {
                    UNI_DETAIL_LOG("use platform %s.\n", name);
                    *platform = p;
                }
                if (name != NULL) {
                    free(name);
                }
            }
        }
    }
    if (platforms != NULL) {
        free(platforms);
    }
    map_cl_error_2_ee(ret);
}

#define CHAR_PLATFORM_INFO(info, str)                      \
    {                                                      \
        EE ret = get_platform_info(p, info, &value, &len); \
        if (SUCCESS == ret) {                              \
            char *tmp = (char *)value;                     \
            tmp[len] = '\0';                               \
            printf(str ": %s\n", tmp);                     \
            free(value);                                   \
        } else {                                           \
            map_cl_error_2_ee(ret);                        \
        }                                                  \
    }

/**
 * @brief list information about platform
 *
 */
inline EE list_platform_info(Platform p)
{
    void *value;
    U32 len;

    CHAR_PLATFORM_INFO(CL_PLATFORM_PROFILE, "\t Profile");
    CHAR_PLATFORM_INFO(CL_PLATFORM_VERSION, "\t Version ");
    CHAR_PLATFORM_INFO(CL_PLATFORM_NAME, "\t Name ");
    CHAR_PLATFORM_INFO(CL_PLATFORM_VENDOR, "\t Vendor ");
    CHAR_PLATFORM_INFO(CL_PLATFORM_EXTENSIONS, "\t Extensions ");

    return SUCCESS;
}

inline EE get_device_info(Device device, cl_device_info info, void **value, U32 *len)
{
    if (NULL == value) {
        return NULL_POINTER;
    }

    size_t size;
    I32 ret = clGetDeviceInfo(device, info, 0, NULL, &size);
    if (CL_SUCCESS == ret) {
        if (NULL != len) {
            *len = size;
        }
        char *data = (char *)malloc(size + 1);
        if (NULL == data) {
            return ALLOC_FAILED;
        }
        data[size] = '\0';
        ret = clGetDeviceInfo(device, info, size + 1, data, NULL);
        if (CL_SUCCESS != ret) {
            free(data);
        } else {
            *value = data;
        }
    }
    map_cl_error_2_ee(ret);
}

inline EE get_devices(Platform platform, cl_device_type type, U32 *num, Device **devices)
{
    if (NULL == devices || NULL == num) {
        return NULL_POINTER;
    }
    I32 ret = clGetDeviceIDs(platform, type, 0, NULL, num);
    UNI_DETAIL_LOG("devices num:%d.\n", *num);
    if (CL_SUCCESS == ret) {
        Device *p = (Device *)malloc((*num) * sizeof(Device));
        if (NULL == p) {
            return ALLOC_FAILED;
        }
        ret = clGetDeviceIDs(platform, type, (*num), p, NULL);
        if (CL_SUCCESS != ret) {
            free(p);
        } else {
            *devices = p;
#if _DEBUG
            for (U32 i = 0; i < *num; i++) {
                char *name = NULL;
                CHECK_STATUS(get_device_info(p[i], CL_DEVICE_NAME, (void **)&name, NULL));
                UNI_DETAIL_LOG("device %d is %s.\n", i, name);
                if (name != NULL) {
                    free(name);
                }
            }
#endif
        }
    }
    map_cl_error_2_ee(ret);
}

inline EE create_sub_device(
    Device device, const cl_device_partition_property *properties, U32 *num, Device **devices)
{
    if (num == NULL || devices == NULL) {
        return NULL_POINTER;
    }
    I32 ret = clCreateSubDevices(device, properties, 0, NULL, num);
    if (CL_SUCCESS == ret) {
        Device *d = (Device *)malloc(sizeof(Device) * (*num));
        if (NULL == d) {
            return ALLOC_FAILED;
        }
        ret = clCreateSubDevices(device, properties, *num, d, NULL);
        if (CL_SUCCESS == ret) {
            *devices = d;
        } else {
            free(d);
        }
    }
    map_cl_error_2_ee(ret);
}

inline EE retain_device(Device device)
{
    I32 ret = clRetainDevice(device);
    map_cl_error_2_ee(ret);
}

inline EE release_device(Device device)
{
    I32 ret = clReleaseDevice(device);
    map_cl_error_2_ee(ret);
}

inline EE get_device_max_image3d_sizes(Device device, U32 *size)
{
    I32 ret;
    size_t width, height, depth;
    ret = clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &width, NULL);
    if (CL_SUCCESS != ret) {
        map_cl_error_2_ee(ret);
    }
    ret = clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &height, NULL);
    if (CL_SUCCESS != ret) {
        map_cl_error_2_ee(ret);
    }
    ret = clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &depth, NULL);
    size[0] = width;
    size[1] = height;
    size[2] = depth;
    map_cl_error_2_ee(ret);
}

inline EE get_device_max_work_item_sizes(Device device, U32 *size)
{
    size_t data[3];
    I32 ret =
        clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 3, &data, NULL);
    for (U32 i = 0; i < 3; i++) {
        size[i] = (U32)data[i];
    }
    map_cl_error_2_ee(ret);
}

inline EE get_device_max_compute_units(Device device, U32 *size)
{
    size_t val;
    I32 ret = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(size_t), &val, NULL);
    *size = val;
    map_cl_error_2_ee(ret);
}

inline EE get_device_max_work_group_size(Device device, U32 *size)
{
    size_t val;
    I32 ret = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &val, NULL);
    *size = val;
    map_cl_error_2_ee(ret);
}

#define V_Q_Info(device, info, type, str)                                \
    {                                                                    \
        type v;                                                          \
        I32 ret = clGetDeviceInfo(device, info, sizeof(type), &v, NULL); \
        if (CL_SUCCESS != ret) {                                         \
            map_cl_error_2_ee(ret);                                      \
        }                                                                \
                                                                         \
        printf(str "%lld\n", (long long)v);                              \
    }

#define B_Q_Info(device, info, str)                                         \
    {                                                                       \
        cl_bool v;                                                          \
        I32 ret = clGetDeviceInfo(device, info, sizeof(cl_bool), &v, NULL); \
        if (CL_SUCCESS != ret) {                                            \
            map_cl_error_2_ee(ret);                                         \
        }                                                                   \
                                                                            \
        printf(str "%s\n", v ? "Yes" : "NO");                               \
    }

#define STR_Q_Info(device, info, str)                           \
    {                                                           \
        size_t len;                                             \
        I32 ret = clGetDeviceInfo(device, info, 0, NULL, &len); \
        if (CL_SUCCESS != ret) {                                \
            map_cl_error_2_ee(ret);                             \
        }                                                       \
                                                                \
        char *v = (char *)malloc(len + 1);                      \
        ret = clGetDeviceInfo(device, info, len, v, NULL);      \
        if (CL_SUCCESS != ret) {                                \
            map_cl_error_2_ee(ret);                             \
        }                                                       \
                                                                \
        v[len] = '\0';                                          \
        printf(str "%s\n", v);                                  \
        free(v);                                                \
    }

/**
 * @brief list all attributes of device
 *
 * @param device		input
 *
 * @return
 *      0 : success
 *      -1: error
 */
inline EE list_device_info(Device device)
{
    printf("..........Device Info..............\n");
    STR_Q_Info(device, CL_DEVICE_NAME, "Device name : ");
    V_Q_Info(device, CL_DEVICE_ADDRESS_BITS, U32, "Address Bits : ");
    B_Q_Info(device, CL_DEVICE_AVAILABLE, "Device Available : ");
    B_Q_Info(device, CL_DEVICE_COMPILER_AVAILABLE, "Device Compiler Available : ");
    B_Q_Info(device, CL_DEVICE_ENDIAN_LITTLE, "Device is little Endian : ");
    B_Q_Info(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, "ECC Supported : ");
    STR_Q_Info(device, CL_DEVICE_EXTENSIONS, "Device Extensions : ");
    STR_Q_Info(device, CL_DEVICE_OPENCL_C_VERSION, "OpenCL C Version : ");
    STR_Q_Info(device, CL_DEVICE_PROFILE, "Device Profile : ");
    V_Q_Info(device, CL_DEVICE_PROFILING_TIMER_RESOLUTION, size_t, "Timer Resolution : ");
    {
        cl_device_fp_config v;
        I32 ret = clGetDeviceInfo(
            device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &v, NULL);
        if (CL_SUCCESS != ret) {
            map_cl_error_2_ee(ret);
        }

        if (v & CL_FP_DENORM) {
            printf("Device Support Denorm Single Float \n");
        }
        if (v & CL_FP_INF_NAN) {
            printf("Device Support Single Float INF NAN\n");
        }
        if (v & CL_FP_ROUND_TO_NEAREST) {
            printf("Device Support Single Float Round to Nearest\n");
        }
        if (v & CL_FP_ROUND_TO_ZERO) {
            printf("Device Support Single Float Round to Zero \n");
        }
        if (v & CL_FP_ROUND_TO_INF) {
            printf("Device Support Single Float Round to Inf\n");
        }
        if (v & CL_FP_FMA) {
            printf("Device Support Single Float FMA\n");
        }
        if (v & CL_FP_SOFT_FLOAT) {
            printf("Device does not Support Hardware Single Float\n");
        }
    }

    STR_Q_Info(device, CL_DEVICE_VENDOR, "Device Vendor : ");
    V_Q_Info(device, CL_DEVICE_VENDOR_ID, U32, "Device Vendor ID : ");
    STR_Q_Info(device, CL_DEVICE_VERSION, "Device Version : ");
    STR_Q_Info(device, CL_DRIVER_VERSION, "Driver Version : ");
    B_Q_Info(device, CL_DEVICE_HOST_UNIFIED_MEMORY, "Unified Memory Supported : ");
    V_Q_Info(device, CL_DEVICE_MAX_PARAMETER_SIZE, size_t, "Max Parameter Size : ");

    printf("..............Global Memory Configuration.............\n");
    V_Q_Info(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, cl_ulong, "Max Memory Allocate Size : ");
    V_Q_Info(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN, U32, "Max Base Address Align Size : ");
    V_Q_Info(device, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, U32, "Min Data Type align Size :");

    V_Q_Info(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, cl_ulong, "Global Memory Cache Size : ");
    {
        cl_device_mem_cache_type v;
        I32 ret = clGetDeviceInfo(
            device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cl_device_mem_cache_type), &v, NULL);
        if (CL_SUCCESS != ret) {
            map_cl_error_2_ee(ret);
        }
        switch (v) {
            case CL_NONE:
                printf("Global Memory does not have Cache \n");
                break;
            case CL_READ_ONLY_CACHE:
                printf("Global Memory has Readonly Cache \n");
                break;
            case CL_READ_WRITE_CACHE:
                printf("Global Memory has Read Write Cache \n");
                break;
            default:
                printf("Unknown Global Memory Cache type \n");
                break;
        }
    }

    V_Q_Info(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, U32, "Global Memory, Cacheline Size : ");
    V_Q_Info(device, CL_DEVICE_GLOBAL_MEM_SIZE, cl_ulong, "Global Memory Size : ");
    // CL_DEVICE_HALF_FP_CONFIG

    printf("..................Image Information...................\n");
    B_Q_Info(device, CL_DEVICE_IMAGE_SUPPORT, "Image Supported : ");
    V_Q_Info(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, size_t, "2D Image Max Height : ");
    V_Q_Info(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, size_t, "2D Image Max Width : ");
    V_Q_Info(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, size_t, "3D Image Max Depth : ");
    V_Q_Info(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, size_t, "3D Image Max Height : ");
    V_Q_Info(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, size_t, "3D Image Max Width : ");
    V_Q_Info(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, U32, "Max Read Image Args : ");
    V_Q_Info(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, U32, "Max Write Image Args : ");
    V_Q_Info(device, CL_DEVICE_MAX_SAMPLERS, U32, "Max Samples : ");

    printf(".................Local Memory...............................\n");
    V_Q_Info(device, CL_DEVICE_LOCAL_MEM_SIZE, cl_ulong, "Local Memory Size : ");
    {
        cl_device_local_mem_type v;
        I32 ret = clGetDeviceInfo(
            device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(cl_device_local_mem_type), &v, NULL);
        if (CL_SUCCESS != ret) {
            map_cl_error_2_ee(ret);
        }
        switch (v) {
            case CL_LOCAL:
                printf("Device has Dedicate Local Memory\n");
                break;
            case CL_GLOBAL:
                printf("Local Memory uses Global Memory\n");
                break;
            default:
                printf("%d\n", __LINE__);
        }
    }

    printf("...................CU Information...........................\n");
    V_Q_Info(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, U32, "Max Clock Frequency : ");
    V_Q_Info(device, CL_DEVICE_MAX_COMPUTE_UNITS, U32, "Max Compute Units : ");

    printf(".................Constant Memory Information.............\n");
    V_Q_Info(device, CL_DEVICE_MAX_CONSTANT_ARGS, U32, "Max Constant Args : ");
    V_Q_Info(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, cl_ulong, "Max Constant Buffer Size : ");

    printf("...................ND Range Information........................\n");
    V_Q_Info(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t, "Max Work Group Size : ");
    V_Q_Info(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, U32, "Work Item Dimensions : ");

    {
        size_t v[3];
        I32 ret =
            clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 3, &v, NULL);
        if (CL_SUCCESS != ret) {
            map_cl_error_2_ee(ret);
        }
        printf("Max Work Item size : %zu %zu %zu\n", v[0], v[1], v[2]);
    }

    printf(".....................Vector Information..................\n");
    V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, U32, "Native Vector Width Char : ");
    V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, U32, "Native Vector Width Short : ");
    V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, U32, "Native Vector Width Int : ");
    V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, U32, "Native Vector Width Long : ");
    V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, U32, "Native Vector Width Float : ");
    V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, U32, "Native Vector Width Double : ");
    V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, U32, "Native Vector Width Half : ");

    V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, U32, "Preferred Vector Width Char : ");
    V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, U32, "Preferred Vector Width Short : ");
    V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, U32, "Preferred Vector Width Int : ");
    V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, U32, "Preferred Vector Width Long : ");
    V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, U32, "Preferred Vector Width Float : ");
    V_Q_Info(
        device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, U32, "Preferred Vector Width Double : ");
    V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, U32, "Preferred Vector Width Half : ");

    return SUCCESS;
}

#if defined(__cplusplus)
}
#endif
#endif
