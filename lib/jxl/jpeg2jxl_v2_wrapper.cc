#include "jxl/jpeg2jxl_v2_wrapper.h"
#include "jxl/jpg2jxl.h"

#ifdef __cplusplus
extern "C" {
#endif
uint8_t* decodeJxlToJpegBytes_wrapper(uint8_t* data, size_t data_size, size_t* decompressed_size) {
  return decodeJxlToJpegBytes(data, data_size, decompressed_size);
}
#ifdef __cplusplus
}
#endif