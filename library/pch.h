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
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <utility>
#include <random>
#include <variant>
#include <optional>
#include <format>

#include <curl/curl.h>
#include <zstd.h>
#include <json.hpp>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <glog/log_severity.h>

#if 1
#define L_WARN(text, ...) LOG(WARNING) << std::format(text "\n" __VA_OPT__(,) __VA_ARGS__)
#define L_INFO(text, ...) LOG(INFO) << std::format(text "\n" __VA_OPT__(,) __VA_ARGS__)
#define L_ERROR(text, ...) LOG(ERROR) << std::format(text "\n" __VA_OPT__(,) __VA_ARGS__)
#define L_VERBOSE(level, text, ...) VLOG(level) << std::format(text "\n" __VA_OPT__(,) __VA_ARGS__)
#else
#define L_WARN(text, ...)
#define L_INFO(text, ...)
#define L_ERROR(text, ...)
#define L_VERBOSE(level, text, ...)
#endif
