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

#include "resources_manager.h"
#include "rm_tree.h"
#include "rm_cdn.h"

rm_tree *rm_tree_create(const char *path) {
  return new rm_tree(path);
}

void rm_tree_destroy(rm_tree *tree) {
  if (tree == nullptr)
    return;
  delete tree;
}

error_code_t rm_tree_add_cdn(rm_tree *tree, rm_cdn *cdn) {
  auto ret = tree->add_cdn(*cdn);
  delete cdn;
  return ret;
}

error_code_t rm_tree_add_dependency(rm_tree *tree, rm_tree *dependency) {
  auto ret = tree->add_dependency(*dependency);
  delete dependency;
  return ret;
}

error_code_t rm_tree_fetch_updates(rm_tree *tree) {
  return tree->fetch_updates();
}

bool rm_tree_fetching_updates(rm_tree *tree) {
  return tree->fetching();
}

bool rm_tree_fetched_updates(rm_tree *tree) {
  return tree->fetched();
}

error_code_t rm_tree_stop_fetching_updates(rm_tree *tree) {
  return tree->stop_fetching();
}

error_code_t rm_tree_download(rm_tree *tree) {
  return tree->download();
}

bool rm_tree_downloading(rm_tree *tree) {
  return tree->downloading();
}

bool rm_tree_downloaded(rm_tree *tree) {
  return tree->downloaded();
}

error_code_t rm_tree_stop_downloading(rm_tree *tree) {
  return tree->stop_download();
}

error_code_t rm_tree_check(rm_tree *tree) {
  return tree->check();
}

bool rm_tree_checking(rm_tree *tree) {
  return tree->checking();
}

bool rm_tree_checked(rm_tree *tree) {
  return tree->checked();
}

error_code_t rm_tree_stop_checking(rm_tree *tree) {
  return tree->stop_check();
}

error_code_t rm_tree_remove_modifications(rm_tree *tree) {
  return tree->remove_modifications();
}

bool rm_tree_removing_modifications(rm_tree *tree) {
  return tree->removing_modifications();
}

bool rm_tree_removed_modifications(rm_tree *tree) {
  return tree->removed_modifications();
}

error_code_t rm_tree_stop_removing_modifications(rm_tree *tree) {
  return tree->stop_remove_modifications();
}

bool rm_tree_has_worker_errors(rm_tree *tree) {
  return tree->has_worker_error();
}

bool rm_tree_is_system_error(rm_tree *tree) {
  return tree->is_system_error();
}

bool rm_tree_is_indexed_error(rm_tree *tree) {
  return tree->is_indexed_error();
}

bool rm_tree_is_runtime_error(rm_tree *tree) {
  return tree->is_runtime_error();
}

error_code_t rm_tree_get_worker_indexed_error_code(rm_tree *tree) {
  return tree->get_worker_indexed_error_code();
}

int rm_tree_get_worker_sys_error_code(rm_tree *tree) {
  return tree->get_worker_sys_error_code();
}

bool rm_tree_get_worker_error_str(rm_tree *tree, char *str, int str_capacity) {
  auto stdstr = tree->get_worker_error_str();
  return strcpy_s(str, str_capacity, stdstr.c_str()) != 0;
}

uint64_t rm_tree_get_total_work_amount(rm_tree *tree) {
  return tree->get_total_work_amount();
}

uint64_t rm_tree_get_completed_work_amount(rm_tree *tree) {
  return tree->get_completed_work_amount();
}

size_t rm_tree_get_pending_download_files_count(rm_tree *tree) {
  return tree->get_pending_download_files_count();
}

rm_cdn *rm_cdn_create(const char *url) {
  return new rm_cdn(url);
}

void rm_cdn_destroy(rm_cdn *cdn) {
  if (cdn == nullptr)
    return;
  delete cdn;
}

void rm_cdn_add_header(rm_cdn *cdn, const char *key, const char *value) {
  cdn->add_header(key, value);
}

void rm_cdn_remove_header(rm_cdn *cdn, const char *key) {
  cdn->remove_header(key);
}
