#ifndef JPEG_2_JXL_H_
#define JPEG_2_JXL_H_
#include <stddef.h>
#include <stdint.h>
#include "jxl/jxl_export.h"

JXL_EXPORT uint8_t* jxlCompress(const uint8_t* data, size_t size);
JXL_EXPORT uint8_t* jxlDecompress(const uint8_t* data, size_t size);

#endif