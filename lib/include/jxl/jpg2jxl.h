#ifndef JPEG_2_JXL_H_
#define JPEG_2_JXL_H_
#include <stddef.h>
#include <stdint.h>

#include "jxl/jxl_export.h"

JXL_EXPORT uint8_t* jxlCompress(const uint8_t* data, size_t size);
JXL_EXPORT uint8_t* jxlDecompress(const uint8_t* data, size_t size);
JXL_EXPORT bool encodeJpegToJxlFile(char* filename, char* out_filename);
JXL_EXPORT bool decodeJxlToJpegFile(char *in_file, char *out_file);
JXL_EXPORT uint8_t* encodeJpegToJxlBytes(uint8_t *data, size_t data_size, size_t* compressed_size);
JXL_EXPORT uint8_t* decodeJxlToJpegBytes(uint8_t *data, size_t data_size, size_t *decompressed_size);
#endif