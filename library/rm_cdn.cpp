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

#include "rm_cdn.h"
#include <common.hpp>

rm_cdn::easy_init_t::~easy_init_t() {
  if (ch != nullptr) {
    unlink_from_curlm();
    curl_easy_cleanup(ch);
    ch = nullptr;
  }
  if (headers != nullptr) {
    curl_slist_free_all(headers);
    headers = nullptr;
  }
}

void rm_cdn::easy_init_t::link_to_curlm(CURLM *link_to) {
  if (link_to == nullptr || ch == nullptr) return;
  unlink_from_curlm();
  curl_multi_add_handle(link_to, ch);
  linked_curlm = link_to;
}

void rm_cdn::easy_init_t::unlink_from_curlm() {
  if (linked_curlm == nullptr || ch == nullptr) return;
  curl_multi_remove_handle(linked_curlm, ch);
  linked_curlm = nullptr;
}

rm_cdn::easy_init_t::easy_init_t(rm_cdn::easy_init_t &&old_init) {
  this->~easy_init_t();
  ch = old_init.ch;
  headers = old_init.headers;
  old_init.ch = nullptr;
  old_init.headers = nullptr;
}

rm_cdn::easy_init_t &rm_cdn::easy_init_t::operator=(rm_cdn::easy_init_t &&old_init) {
  this->~easy_init_t();
  ch = old_init.ch;
  headers = old_init.headers;
  old_init.ch = nullptr;
  old_init.headers = nullptr;
  return *this;
}

rm_cdn::rm_cdn(std::string url) : base_url(std::move(url)) {
  if (base_url.back() != '/') {
    base_url += '/';
  }
  is_http_ = base_url.rfind("http", 0) == 0;
}

void rm_cdn::add_header(const std::string &key, const std::string &value) {
  headers[key] = value;
}

void rm_cdn::remove_header(const std::string &key) {
  if (headers.find(key) == headers.cend()) return;
  headers.erase(key);
}

std::string rm_cdn::build_url(std::string path) const {
  if (path[0] == '/') {
    path.erase(0, 1);
  }
  std::replace(path.begin(), path.end(), '\\', '/');
  auto splitted = common::str_split(path, '/');
  path.clear();
  for (auto ptr = splitted.begin(); ptr != splitted.end(); ++ptr) {
    auto escaped = curl_escape(ptr->c_str(), ptr->size());
    path += escaped;
    if (ptr + 1 != splitted.end())
      path += '/';
    curl_free(escaped);
  }
  return base_url + path;
}

rm_cdn::easy_init_t rm_cdn::easy_init(const std::string &path) const {
  easy_init_t ret;
  ret.ch = curl_easy_init();
  if (ret.ch == nullptr) {
    L_WARN("Could not initialuze CURL easy handler");
    return ret;
  }
  curl_easy_setopt(ret.ch, CURLOPT_URL, build_url(path).c_str());
  if (is_http() && !headers.empty()) {
    curl_slist *curl_headers = nullptr;
    for (auto &entry : headers) {
      std::string value = entry.first;
      value += ": ";
      value += entry.second;

      auto temp = curl_slist_append(curl_headers, value.c_str());
      if (temp == nullptr) {
        // something went wrong, but we have no idea really what
        L_WARN("Could not initialuze CURL easy handler headers list");
        if (curl_headers != nullptr)
          curl_slist_free_all(curl_headers);
        return ret;
      }
      curl_headers = temp;
    }
    curl_easy_setopt(ret.ch, CURLOPT_HTTPHEADER, curl_headers);
    ret.headers = curl_headers;
  }
  return ret;
}

bool rm_cdn::is_http() const {
  return is_http_;
}
