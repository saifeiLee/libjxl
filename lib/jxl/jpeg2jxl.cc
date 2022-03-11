#include <cstring>

#include "jxl/jpg2jxl.h"
#include "lib/jxl/jpeg/enc_jpeg_data.h"
#include "lib/extras/dec/jpg.h"
#include "lib/jxl/base/span.h"
#include "lib/jxl/codec_in_out.h"
#include "lib/jxl/dec_file.h"
#include "lib/jxl/enc_cache.h"
#include "lib/jxl/enc_color_management.h"
#include "lib/jxl/enc_file.h"

/* NOTA BENE: see file history to uncover how to decode HDR JPEGs to pixels. */

/** Result: uint32_t 'size' followed by compressed image (JXL). */
uint8_t* jxlCompress(const uint8_t* data, size_t size) {
  jxl::PaddedBytes compressed;
  jxl::CodecInOut io;
  jxl::extras::Codec input_codec;
  for (size_t i = 0; i < size; ++i) {
      compressed.push_back(data[i]);
  }
  if (size >= 2 && data[0] == 0xFF && data[1] == 0xD8) {
    input_codec = jxl::extras::Codec::kJPG;
  }
  // 第一步
  if (!jxl::jpeg::DecodeImageJPG(jxl::Span<const uint8_t>(compressed), &io)) {
    printf("Decode jpeg error\n");
    return nullptr;
  }
  printf("jpeg decode success???");
//   if (!jxl::SetFromBytes(jxl::Span<const uint8_t>(data, size), &io, nullptr,
//                          &input_codec)) {
//     printf("Set from bytes error.\n");
//     return nullptr;
//   }
  jxl::CompressParams params;
  jxl::PassesEncoderState passes_encoder_state;
  // 第二步
  if (!jxl::EncodeFile(params, &io, &passes_encoder_state, &compressed,
                       jxl::GetJxlCms(), nullptr, nullptr)) {
    printf("EncodeFile error.\n");
    return nullptr;
  }
  size_t compressed_size = compressed.size();
  uint8_t* result = reinterpret_cast<uint8_t*>(malloc(compressed_size + 4));
  uint32_t* meta = reinterpret_cast<uint32_t*>(result);
  meta[0] = compressed_size;
  memcpy(result + 4, compressed.data(), compressed_size);
  printf("jxlCompress result:%p, size: %zu\n", result, compressed_size);
  return result;
}

/** Result: uint32_t 'size' followed by decompressed image (JPG). */
uint8_t* jxlDecompress(const uint8_t* data, size_t size) {
  jxl::PaddedBytes decompressed;
  jxl::CodecInOut io;
  jxl::DecompressParams params;
  printf("Ready to DecodeFile.\n");
  if (!jxl::DecodeFile(params, jxl::Span<const uint8_t>(data, size), &io,
                       nullptr)) {
    printf("DecodeFile error.\n");
    return nullptr;
  }
  io.use_sjpeg = false;
  io.jpeg_quality = 100;
  // 改成EncodeJpegData
  printf("jpeg_data: %p\n", io.Main().jpeg_data.get());
  if (!io.Main().jpeg_data.get())
  {
      printf("jpeg_data is null\n");
      exit(1);
  }
  if (jxl::jpeg::EncodeJPEGData(*io.Main().jpeg_data.get(), &decompressed)) {
    printf("EncodeJPEGData 成功\n");
  } else {
    printf("EncodeJPEGData 失败\n");
  }
//   if (!jxl::Encode(io, jxl::extras::Codec::kJPG, io.Main().c_current(), 8,
//                    &decompressed, nullptr)) {
//     printf("Encode error.\n");
//     return nullptr;
//   }
  printf("jxlDecompress result:%p, size: %zu\n", decompressed.data(),
         decompressed.size());
  size_t decompressed_size = decompressed.size();
  uint8_t* result = reinterpret_cast<uint8_t*>(malloc(decompressed_size + 4));
  uint32_t* meta = reinterpret_cast<uint32_t*>(result);
  meta[0] = decompressed_size;
  memcpy(result + 4, decompressed.data(), decompressed_size);
  return result;
}
