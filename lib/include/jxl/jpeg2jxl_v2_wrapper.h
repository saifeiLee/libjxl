#include <stddef.h>
#include <stdint.h>

#include "jxl/jxl_export.h"
#ifdef __cplusplus
extern "C" {
#endif

JXL_EXPORT uint8_t* decodeJxlToJpegBytes_wrapper(uint8_t* data,
                                                 size_t data_size,
                                                 size_t* decompressed_size);
#ifdef __cplusplus
}
#endif