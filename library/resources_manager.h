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

#ifndef RM_EXPORT
#if defined RM_BUILDING_DLL && WIN32
#ifdef RM_EXPORT_SYMBOLS
#define RM_EXPORT __declspec(dllexport)
#else
#define RM_EXPORT __declspec(dllimport)
#endif
#else
#define RM_EXPORT
#endif
#endif

enum error_code_t {
  kUnknownError = -1,

  kNoError,
  kCannotWhenWorking,
  kCannotAfterStarted,
  kCannotWhenNothingToDownload,
  kCannotWhenNotWorking,

  kForceStoppedProcess,
  kCouldNotResolveHost,
  kCouldNotResolveProxy,
  kCouldNotConnectToHost,
  kCouldNotConnectToHostRemoteAccessDenied,
  kUnknownHttpCodeResponse,
  kUnknownCurlError,

  kMaxErrorCode
};

#ifdef __cplusplus
#include <cstdint>
class rm_tree;
class rm_cdn;
extern "C" {
#else
typedef void rm_tree;
typedef void rm_cdn;
#include <stddef.h>
#endif

// Resource manager trees

RM_EXPORT rm_tree *rm_tree_create(const char *path);
RM_EXPORT void rm_tree_destroy(rm_tree *tree);

RM_EXPORT error_code_t rm_tree_add_cdn(rm_tree *tree, rm_cdn *cdn);
RM_EXPORT error_code_t rm_tree_add_dependency(rm_tree *tree, rm_tree *dependency);

RM_EXPORT error_code_t rm_tree_fetch_updates(rm_tree *tree);
RM_EXPORT bool rm_tree_fetching_updates(rm_tree *tree);
RM_EXPORT bool rm_tree_fetched_updates(rm_tree *tree);
RM_EXPORT error_code_t rm_tree_stop_fetching_updates(rm_tree *tree);

RM_EXPORT error_code_t rm_tree_download(rm_tree *tree);
RM_EXPORT bool rm_tree_downloading(rm_tree *tree);
RM_EXPORT bool rm_tree_downloaded(rm_tree *tree);
RM_EXPORT error_code_t rm_tree_stop_downloading(rm_tree *tree);

RM_EXPORT error_code_t rm_tree_check(rm_tree *tree);
RM_EXPORT bool rm_tree_checking(rm_tree *tree);
RM_EXPORT bool rm_tree_checked(rm_tree *tree);
RM_EXPORT error_code_t rm_tree_stop_checking(rm_tree *tree);

RM_EXPORT error_code_t rm_tree_remove_modifications(rm_tree *tree);
RM_EXPORT bool rm_tree_removing_modifications(rm_tree *tree);
RM_EXPORT bool rm_tree_removed_modifications(rm_tree *tree);
RM_EXPORT error_code_t rm_tree_stop_removing_modifications(rm_tree *tree);

RM_EXPORT bool rm_tree_has_worker_errors(rm_tree *tree);
RM_EXPORT bool rm_tree_is_system_error(rm_tree *tree);
RM_EXPORT bool rm_tree_is_indexed_error(rm_tree *tree);
RM_EXPORT bool rm_tree_is_runtime_error(rm_tree *tree);
RM_EXPORT int rm_tree_get_worker_sys_error_code(rm_tree *tree);
RM_EXPORT error_code_t rm_tree_get_worker_indexed_error_code(rm_tree *tree);
RM_EXPORT bool rm_tree_get_worker_error_str(rm_tree *tree, char *str, int str_capacity);

RM_EXPORT uint64_t rm_tree_get_total_work_amount(rm_tree *tree);
RM_EXPORT uint64_t rm_tree_get_completed_work_amount(rm_tree *tree);

RM_EXPORT size_t rm_tree_get_pending_download_files_count(rm_tree *tree);

// Resource manager CDNs

RM_EXPORT rm_cdn *rm_cdn_create(const char *url);
RM_EXPORT void rm_cdn_destroy(rm_cdn *cdn);

RM_EXPORT void rm_cdn_add_header(rm_cdn *cdn, const char *key, const char *value);
RM_EXPORT void rm_cdn_remove_header(rm_cdn *cdn, const char *key);
RM_EXPORT void rm_cdn_set_custom_cacert_filepath(rm_cdn *cdn, const char *path);

#ifdef __cplusplus
}
#endif
