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

#include "semantic_version.h"

semantic_version::semantic_version(uint8_t major, uint8_t minor, uint8_t patch)
    : major(major), minor(minor), patch(patch) {
  /* Nothing to do */
}

int semantic_version::compare(const semantic_version &other) const {
  if (major != other.major)
  {
    return major - other.major;
  }

  if (minor != other.minor)
  {
    return minor - other.minor;
  }

  if (patch != other.patch)
  {
    return patch - other.patch;
  }

  return 0;
}

bool semantic_version::operator==(const semantic_version &other) const {
  return compare(other) == 0;
}

bool semantic_version::operator!=(const semantic_version &other) const {
  return !(*this == other);
}

bool semantic_version::operator>(const semantic_version &other) const {
  return compare(other) > 0;
}

bool semantic_version::operator<(const semantic_version &other) const {
  return compare(other) < 0;
}

bool semantic_version::operator>=(const semantic_version &other) const {
  return compare(other) >= 0;
}

bool semantic_version::operator<=(const semantic_version &other) const {
  return compare(other) <= 0;
}
