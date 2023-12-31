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

#include "rm_entry.h"

rm_entry::rm_entry() {
  /* Nothing to do */
}

rm_entry::rm_entry(const nlohmann::json &json) {
  std::string path = json["p"];
#ifndef WIN32
  std::replace(path.begin(), path.end(), '\\', '/');
#endif
  relative_path = path;
  size = static_cast<uint64_t>(json["s"]);
  fnv_hash = json["h"];

  compressed = json["c"];
  compressed_size = compressed ? static_cast<uint64_t>(json["cs"]) : 0;
  compressed_fnv_hash = compressed ? static_cast<uint32_t>(json["ch"]) : 0;
}

bool rm_entry::operator==(const rm_entry &other) const {
  return relative_path == other.relative_path;
}
