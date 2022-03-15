#include <cmath>  // std::abs
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>

// #include <string.h>

#include <cstring>
#include <cstdlib>

#include "jxl/codestream_header.h"
#include "jxl/decode.h"
#include "jxl/decode_cxx.h"
#include "jxl/encode.h"
#include "jxl/encode_cxx.h"
#include "jxl/jpg2jxl.h"
#include "jxl/types.h"
// #include "lib/extras/codec.h"  // 由于接口里没用到SetFromBytes 接口，所以这个依赖想注释掉，如果加上，Cmake配置那里要加上lib/extras的依赖
#include "lib/jxl/base/file_io.h"
#include "lib/jxl/dec_external_image.h"
#include "lib/jxl/enc_butteraugli_comparator.h"
#include "lib/jxl/enc_comparator.h"
#include "lib/jxl/enc_external_image.h"
#include "lib/jxl/encode_internal.h"

void EncodeWithEncoder(JxlEncoder *enc, std::vector<uint8_t> *compressed) {
  compressed->resize(64);
  uint8_t *next_out = compressed->data();
  size_t avail_out = compressed->size() - (next_out - compressed->data());
  JxlEncoderStatus process_result = JXL_ENC_NEED_MORE_OUTPUT;
  while (process_result == JXL_ENC_NEED_MORE_OUTPUT) {
    process_result = JxlEncoderProcessOutput(enc, &next_out, &avail_out);
    if (process_result == JXL_ENC_NEED_MORE_OUTPUT) {
      size_t offset = next_out - compressed->data();
      compressed->resize(compressed->size() * 2);
      next_out = compressed->data() + offset;
      avail_out = compressed->size() - offset;
    }
  }
  compressed->resize(next_out - compressed->data());
  if (JXL_ENC_SUCCESS != process_result) {
    printf("JxlEncoderProcessOutput error.\n");
    exit(1);
  }
}

// API for demo test
bool encodeJpegToJxlFile(char *in_file, char *out_file) {
  std::string filename = in_file;
  std::string out_filename = out_file;
  printf("Input file: %s\n", filename.c_str());
  printf("Output file name: %s\n", out_filename.c_str());
  jxl::PaddedBytes orig;
  bool ok = ReadFile(filename, &orig);
  printf("Read file size: %zu\n", orig.size());
  JXL_CHECK(ok);
  jxl::CodecInOut orig_io;
  // if (!SetFromBytes(jxl::Span<const uint8_t>(orig), &orig_io, nullptr)) {
  //   printf("SetFromBytes error...\n");
  // }
  JxlEncoderPtr enc = JxlEncoderMake(nullptr);
  JxlEncoderFrameSettings *frame_settings =
      JxlEncoderFrameSettingsCreate(enc.get(), NULL);
  if (JXL_ENC_SUCCESS != JxlEncoderUseContainer(enc.get(), JXL_TRUE)) {
    printf("JxlEncoderUseContainer error...\n");
    exit(1);
  }
  if (JXL_ENC_SUCCESS != JxlEncoderStoreJPEGMetadata(enc.get(), JXL_TRUE)) {
    printf("JxlEncoderStoreJPEGMetadata error...\n");
    exit(1);
  }
  if (JXL_ENC_SUCCESS !=
      JxlEncoderAddJPEGFrame(frame_settings, orig.data(), orig.size())) {
    printf("JxlEncoderAddJPEGFrame error...\n");
    exit(1);
  }
  JxlEncoderCloseInput(enc.get());
  std::vector<uint8_t> compressed;
  EncodeWithEncoder(enc.get(), &compressed);
  // 编码完成，写入文件
  bool write_ok = jxl::WriteFile(compressed, out_filename);
  if (!write_ok) {
    printf("WriteFile error...\n");
    exit(1);
  }
  return true;
}

bool decodeJxlToJpegFile(char *in_file, char *out_file) {
  std::string filename = in_file;
  std::string out_filename = out_file;
  printf("Input file: %s\n", filename.c_str());
  printf("Output file name: %s\n", out_filename.c_str());
  // test
  std::vector<uint8_t> orig;
  bool ok = jxl::ReadFile(filename, &orig);
  printf("Read file size: %zu\n", orig.size());
  JXL_CHECK(ok);
  jxl::CodecInOut orig_io;

  JxlDecoderPtr dec = JxlDecoderMake(nullptr);
  if (JXL_DEC_SUCCESS !=
      JxlDecoderSubscribeEvents(
          dec.get(), JXL_DEC_JPEG_RECONSTRUCTION | JXL_DEC_FULL_IMAGE)) {
    printf("JxlDecoderSubscribeEvents error...\n");
    exit(1);
  }
  JxlDecoderSetInput(dec.get(), orig.data(), orig.size());
  if (JXL_DEC_JPEG_RECONSTRUCTION != JxlDecoderProcessInput(dec.get())) {
    printf("JxlDecoderProcessInput error...\n");
    exit(1);
  }
  std::vector<uint8_t> reconstructed_buffer(128);
  if (JXL_DEC_SUCCESS != JxlDecoderSetJPEGBuffer(dec.get(),
                                                 reconstructed_buffer.data(),
                                                 reconstructed_buffer.size())) {
    printf("JxlDecoderSetJPEGBuffer error...\n");
    exit(1);
  }
  size_t used = 0;
  JxlDecoderStatus dec_process_result = JXL_DEC_JPEG_NEED_MORE_OUTPUT;
  while (dec_process_result == JXL_DEC_JPEG_NEED_MORE_OUTPUT) {
    used = reconstructed_buffer.size() - JxlDecoderReleaseJPEGBuffer(dec.get());
    reconstructed_buffer.resize(reconstructed_buffer.size() * 2);
    if (JXL_DEC_SUCCESS !=
        JxlDecoderSetJPEGBuffer(dec.get(), reconstructed_buffer.data() + used,
                                reconstructed_buffer.size() - used)) {
      printf("JxlDecoderSetJPEGBuffer error...\n");
      exit(1);
    }
    dec_process_result = JxlDecoderProcessInput(dec.get());
  }
  if (JXL_DEC_FULL_IMAGE != dec_process_result) {
    printf("JxlDecoderProcessInput error...\n");
    exit(1);
  }
  used = reconstructed_buffer.size() - JxlDecoderReleaseJPEGBuffer(dec.get());
  printf("used: %zu\n", used);
  // 编码完成，写入文件
  bool write_ok = jxl::WriteFile(reconstructed_buffer, out_filename);
  if (!write_ok) {
    printf("WriteFile error...\n");
    exit(1);
  }
  return true;
}

// API for Spice

uint8_t* encodeJpegToJxlBytes(uint8_t *data, size_t data_size, size_t* compressed_size) {
  std::vector<uint8_t> orig;
  orig.resize(data_size);
  memcpy(orig.data(), data, data_size);

  JxlEncoderPtr enc = JxlEncoderMake(nullptr);
  JxlEncoderFrameSettings *frame_settings =
      JxlEncoderFrameSettingsCreate(enc.get(), NULL);
  if (JXL_ENC_SUCCESS != JxlEncoderUseContainer(enc.get(), JXL_TRUE)) {
    printf("JxlEncoderUseContainer error...\n");
    exit(1);
  }
  if (JXL_ENC_SUCCESS != JxlEncoderStoreJPEGMetadata(enc.get(), JXL_TRUE)) {
    printf("JxlEncoderStoreJPEGMetadata error...\n");
    exit(1);
  }
  if (JXL_ENC_SUCCESS !=
      JxlEncoderAddJPEGFrame(frame_settings, orig.data(), orig.size())) {
    printf("JxlEncoderAddJPEGFrame error...\n");
    exit(1);
  }
  JxlEncoderCloseInput(enc.get());
  std::vector<uint8_t> compressed;
  EncodeWithEncoder(enc.get(), &compressed);
  *compressed_size = compressed.size();
  printf("[JXL log]compressed size: %zu\n", compressed.size());
  uint8_t* result = reinterpret_cast<uint8_t*>(malloc(*compressed_size));
  memcpy(result, compressed.data(), *compressed_size);
  return result;
}

uint8_t* decodeJxlToJpegBytes(uint8_t *data, size_t data_size, size_t* decompressed_size) {
  std::vector<uint8_t> orig;
  orig.resize(data_size);
  memcpy(orig.data(), data, data_size);
  printf("Read file size: %zu\n", orig.size());

  JxlDecoderPtr dec = JxlDecoderMake(nullptr);
  if (JXL_DEC_SUCCESS !=
      JxlDecoderSubscribeEvents(
          dec.get(), JXL_DEC_JPEG_RECONSTRUCTION | JXL_DEC_FULL_IMAGE)) {
    printf("JxlDecoderSubscribeEvents error...\n");
    exit(1);
  }
  JxlDecoderSetInput(dec.get(), orig.data(), orig.size());
  if (JXL_DEC_JPEG_RECONSTRUCTION != JxlDecoderProcessInput(dec.get())) {
    printf("JxlDecoderProcessInput error...\n");
    exit(1);
  }
  std::vector<uint8_t> reconstructed_buffer(128);
  if (JXL_DEC_SUCCESS != JxlDecoderSetJPEGBuffer(dec.get(),
                                                 reconstructed_buffer.data(),
                                                 reconstructed_buffer.size())) {
    printf("JxlDecoderSetJPEGBuffer error...\n");
    exit(1);
  }
  size_t used = 0;
  JxlDecoderStatus dec_process_result = JXL_DEC_JPEG_NEED_MORE_OUTPUT;
  while (dec_process_result == JXL_DEC_JPEG_NEED_MORE_OUTPUT) {
    used = reconstructed_buffer.size() - JxlDecoderReleaseJPEGBuffer(dec.get());
    reconstructed_buffer.resize(reconstructed_buffer.size() * 2);
    if (JXL_DEC_SUCCESS !=
        JxlDecoderSetJPEGBuffer(dec.get(), reconstructed_buffer.data() + used,
                                reconstructed_buffer.size() - used)) {
      printf("JxlDecoderSetJPEGBuffer error...\n");
      exit(1);
    }
    dec_process_result = JxlDecoderProcessInput(dec.get());
  }
  if (JXL_DEC_FULL_IMAGE != dec_process_result) {
    printf("JxlDecoderProcessInput error...\n");
    exit(1);
  }
  used = reconstructed_buffer.size() - JxlDecoderReleaseJPEGBuffer(dec.get());
  printf("used: %zu\n", used);
  *decompressed_size = used;
  uint8_t* result = reinterpret_cast<uint8_t*>(malloc(used));
  memcpy(result, reconstructed_buffer.data(), used);
  return result;
}