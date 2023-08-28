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

#include <curl/curl.h>
#include <utility>
#include <string>
#include <unordered_map>

class rm_cdn {
  std::string base_url;
  std::unordered_map<std::string, std::string> headers;
  bool is_http_;
  std::string custom_cacert_filepath;
public:
  struct easy_init_t {
    CURLM *linked_curlm = nullptr;
    CURL *ch = nullptr;
    curl_slist *headers = nullptr;

    void link_to_curlm(CURLM *link_to);
    void unlink_from_curlm();

    easy_init_t() = default;

    easy_init_t(easy_init_t &) = delete;
    easy_init_t(easy_init_t &&old_init);

    easy_init_t &operator=(const easy_init_t &) = delete;
    easy_init_t &operator=(easy_init_t &&old_init);

    ~easy_init_t();
  };
  rm_cdn() = default;
  explicit rm_cdn(std::string url);
  void add_header(const std::string &key, const std::string &value);
  void remove_header(const std::string &key);
  std::string build_url(std::string path) const;
  easy_init_t easy_init(const std::string &path) const;
  bool is_http() const;

  void set_custom_cacert_filepath(const std::string &path);
};
