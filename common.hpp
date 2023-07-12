// MIT License

// Copyright (c) 2023 Northn

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>
#include <zstd.h>
#include <sstream>

namespace common {
constexpr const char *kResourcesDataFilename = "rm_files_data.json";
constexpr size_t kMaxFullCheckSize = 5 * 1024 * 1024; // in bytes (default: 5MB)
constexpr const char *kForcedFullCheckExtensions[] = {
    ".exe",
    ".dll",
    ".sys",
    ".asi",
    ".sf",
    ".cleo",
    ".cs",
    ".lua",
    ".luac"
};
constexpr size_t kCheckEveryXBytes = 512 * 1024; // in bytes (default: 5KB)
constexpr size_t kCheckXBytes = 4;
static_assert(kCheckXBytes < kCheckEveryXBytes);

inline std::string str_tolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

template <typename Out>
void split(const std::string &s, char delim, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) {
    *result++ = item;
  }
}

inline std::vector<std::string> str_split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

inline bool is_extension_forced_to_fullcheck(const std::string &extension) {
  auto lower_case_extension = str_tolower(extension);
  for (auto &entry : kForcedFullCheckExtensions) {
    if (lower_case_extension == entry) {
      return true;
    }
  }
  return false;
}

inline uint32_t get_fnv_hash(const uint8_t *buffer, size_t buffer_size) {
  static constexpr uint32_t fnv_prime = 0x811C9DC5;
  uint32_t hash = 0;
  for (uint32_t i = 0; i < buffer_size; ++i) {
    hash *= fnv_prime;
    hash ^= (buffer[i]);
  }
  return hash;
}

inline uint32_t get_file_hash(const std::filesystem::path &path) {
  if (!exists(path) || !is_regular_file(path))
    return 0;
  auto file_sz = file_size(path);
  if (file_sz == 0)
    return 0;
  std::unique_ptr<char[]> buffer{nullptr};
  size_t buffer_size_to_hash = 0;
  std::ifstream file_stream;
  file_stream.open(path, std::ios::in | std::ios::binary);
  if (file_sz <= kMaxFullCheckSize || is_extension_forced_to_fullcheck(path.extension().string())) {
    buffer.reset(new char[file_sz + 1]);
    buffer_size_to_hash = file_sz;
    file_stream.read(buffer.get(), file_sz);
  } else {
    size_t alloc_size = file_sz / (kCheckEveryXBytes * (kCheckXBytes + 1));
    buffer.reset(new char[alloc_size]);
    for (auto i = 0, pos = 0; pos < file_sz; i += kCheckXBytes) {
      if (pos + kCheckXBytes > file_sz || (i + kCheckXBytes) >= alloc_size)
        break;
      file_stream.seekg(pos);
      file_stream.read(buffer.get() + i, kCheckXBytes);
      pos += kCheckEveryXBytes;
      buffer_size_to_hash = i + kCheckXBytes;
    }
  }
  return get_fnv_hash(reinterpret_cast<uint8_t *>(buffer.get()), buffer_size_to_hash);
}

inline bool decompress_file(const std::filesystem::path &in_filepath, const std::filesystem::path &out_filepath) {
  if (!exists(in_filepath) || !is_regular_file(in_filepath))
    throw std::runtime_error("Could not locate input path in decompressor");

  if (!exists(out_filepath.parent_path()))
    create_directories(out_filepath.parent_path());

  auto buff_in_size = ZSTD_DStreamInSize();
  auto buff_out_size = ZSTD_DStreamOutSize();
  auto buff_in = std::make_unique<char[]>(buff_in_size);
  auto buff_out = std::make_unique<char[]>(buff_out_size);
  auto dctx = ZSTD_createDCtx();

  if (dctx == nullptr)
    throw std::runtime_error("Could not allocate decompressor context for ZSTD decompressor");

  std::ifstream in_file;
  in_file.open(in_filepath, std::ios::in | std::ios::binary);

  std::ofstream out_file;
  out_file.open(out_filepath, std::ios::out | std::ios::binary);

  while (true) {
    in_file.read(buff_in.get(), buff_in_size);
    auto bytes_read = in_file.gcount();
    if (bytes_read <= 0)
      break;

    ZSTD_inBuffer input{buff_in.get(), static_cast<size_t>(bytes_read), 0};
    while (input.pos < input.size) {
      ZSTD_outBuffer output{buff_out.get(), buff_out_size, 0};
      auto ret = ZSTD_decompressStream(dctx, &output, &input);
      if (ZSTD_isError(ret)) {
        std::string error_str = "Unknown ZSTD error while decompressing: ";
        error_str += ZSTD_getErrorName(ret);
        throw std::runtime_error(error_str);
      }
      out_file.write(buff_out.get(), output.pos);
    }
  }
  out_file.flush();
  ZSTD_freeDCtx(dctx);
  return true;
}

inline bool compress_file(const std::filesystem::path &in_filepath, const std::filesystem::path &out_filepath, const int level) {
  if (!exists(in_filepath) || !is_regular_file(in_filepath))
    throw std::runtime_error("Could not locate input path in compressor");

  if (!exists(out_filepath.parent_path()))
    create_directories(out_filepath.parent_path());

  auto buff_in_size = ZSTD_CStreamInSize();
  auto buff_out_size = ZSTD_CStreamOutSize();
  auto buff_in = std::make_unique<char[]>(buff_in_size);
  auto buff_out = std::make_unique<char[]>(buff_out_size);
  auto cctx = ZSTD_createCCtx();

  std::ifstream in_file;
  in_file.open(in_filepath, std::ios::in | std::ios::binary);
  std::ofstream out_file;
  out_file.open(out_filepath, std::ios::out | std::ios::binary);

  if (cctx == nullptr)
    throw std::runtime_error("Could not allocate compressor context for ZSTD compressor");

  ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, level);
  ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1);
  if (auto threads_count = std::thread::hardware_concurrency(); threads_count > 2) {
    threads_count /= 2;
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, threads_count);
  }

  auto to_read = buff_in_size;
  while (true) {
    in_file.read(buff_in.get(), buff_in_size);
    auto bytes_read = in_file.gcount();

    auto last_chunk = bytes_read < to_read;
    ZSTD_EndDirective mode = last_chunk ? ZSTD_e_end : ZSTD_e_continue;

    auto finished = false;
    ZSTD_inBuffer input{reinterpret_cast<void *>(buff_in.get()), static_cast<size_t>(bytes_read), 0};
    do {
      ZSTD_outBuffer output{reinterpret_cast<void *>(buff_out.get()), buff_out_size, 0};
      auto remaining = ZSTD_compressStream2(cctx, &output, &input, mode);
      if (ZSTD_isError(remaining)) {
        std::string error_str = "Unknown ZSTD error while compressing: ";
        error_str += ZSTD_getErrorName(remaining);
        throw std::runtime_error(error_str);
      }
      out_file.write(buff_out.get(), output.pos);
      finished = last_chunk ? (remaining == 0) : (input.pos == input.size);
    } while (!finished);

    if (input.pos != input.size)
      throw std::runtime_error(
          "ZSTD compressor error: Impossible: zstd only returns 0 when the input is completely consumed!");

    if (last_chunk)
      break;
  }
  out_file.flush();
  ZSTD_freeCCtx(cctx);
  return true;
}

}
