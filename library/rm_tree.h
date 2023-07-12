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

#include <utility>

#include "rm_cdn.h"
#include "rm_entry.h"
#include "resources_manager.h"

class rm_tree {
  std::vector<rm_cdn> cdns; // all cdns related to this project
  std::vector<rm_entry> items; // all items of this tree. ACHTUNG! do not add items with same names
  std::vector<rm_tree> dependencies; // dependant trees, like moonloader, cleo and etc. only root project can have dependencies
  std::filesystem::path base_path; // absolute path to download. only root knows this property

  using items_const_iterator = decltype(items)::const_iterator;
  using items_iterator = decltype(items)::iterator;
  using cdns_const_iterator = decltype(cdns)::const_iterator;
  using cdns_iterator = decltype(cdns)::iterator;

  enum class worker_mode_t {
    kNone,
    kFetching,
    kDownloading,
    kChecking,
    kRemovingModifications,
    kMaxMode
  };

  struct worker_process_data_t {
    std::atomic_uint64_t total_work_amount = 0;
    std::atomic_uint64_t processed_work_amount = 0;
    std::atomic_bool force_stop = false;
  };

  struct download_worker_job_t {
    bool is_http;
    rm_entry item;
    std::ofstream stream;
    rm_cdn::easy_init_t init;
    std::atomic_uint64_t downloaded_size;
    bool abort = false;
  };

  struct pending_download_item_t {
    rm_entry value;
    size_t errors_count = 0;

    inline explicit pending_download_item_t(rm_entry entry) : value(std::move(entry)) {};
  };

  struct {
    std::atomic<worker_mode_t> last_state = worker_mode_t::kNone;
    std::atomic<worker_mode_t> current_state = worker_mode_t::kNone;

    size_t current_cdn_errors_count = 0;
    cdns_const_iterator current_cdn;

    std::optional<std::variant<
        std::system_error,
        std::runtime_error,
        std::exception
      >> worker_error;
    std::thread *self = nullptr;

    worker_process_data_t process_data;

    std::vector<pending_download_item_t> pending_download_items;
    std::atomic_size_t pending_download_files_count;

    std::mutex download_workers_mtx;
    std::vector<std::unique_ptr<download_worker_job_t>> download_workers;
  } worker;
public:
  explicit rm_tree(std::filesystem::path base_path);
  rm_tree(const rm_tree &tree);
  ~rm_tree();

  // Common
  error_code_t add_cdn(const rm_cdn &cdn);
  error_code_t add_dependency(const rm_tree &dependency);

  // Fetchers
  error_code_t fetch_updates();
  bool fetching() const;
  bool fetched() const;
  error_code_t stop_fetching();

  // Downloader
  error_code_t download();
  bool downloading() const;
  bool downloaded() const;
  error_code_t stop_download();

  // Checker
  error_code_t check();
  bool checking() const;
  bool checked() const;
  error_code_t stop_check();

  // Modifications remover
  error_code_t remove_modifications();
  bool removing_modifications() const;
  bool removed_modifications() const;
  error_code_t stop_remove_modifications();

  // Workers data helpers
  bool has_worker_error() const;
  bool is_system_error() const;
  bool is_runtime_error() const;
  int get_worker_sys_error_code() const;
  std::string get_worker_error_str() const;

  uint64_t get_total_work_amount() const;
  uint64_t get_completed_work_amount();

  size_t get_pending_download_files_count(bool include_dependencies = true) const;
private:
  using worker_t = void(rm_tree &tree, worker_process_data_t *process_data);

  // Helpers
  bool is_working() const;
  std::filesystem::path get_entry_full_path(const rm_entry &entry) const;
  void summon_worker(worker_t worker_fn);
  void worker_release();

  cdns_const_iterator get_current_cdn(uint64_t offset = 0);
  std::string fetch_url_path_content(const std::string &path);

  // Download helpers
  auto find_download_worker(CURL *easy_handler);
  auto find_pending_download_item(const rm_entry &entry);

  // Checker helpers
  bool is_entry_valid(const rm_entry &entry) const;
  size_t get_entries_count(bool include_dependencies = true) const;
  uint64_t get_pending_items_download_size(bool include_dependencies = true) const;

  // Modifications remover helpers
  std::vector<rm_entry> get_all_entries(bool include_dependencies = true) const;

  // Workers
  static void download_worker(rm_tree &tree, worker_process_data_t *process_data = nullptr);
  static void updates_fetcher_worker(rm_tree &tree, worker_process_data_t *process_data = nullptr);
  static void check_worker(rm_tree &tree, worker_process_data_t *process_data = nullptr);
  static void remove_modifications_worker(rm_tree &tree, worker_process_data_t *process_data = nullptr);

  // Check worker data
  static constexpr size_t kMaxCdnErrorsCount = 10;

  // Download worker data
  static constexpr size_t kParallelJobsCount = 5;
  static constexpr size_t kMaxDownloadWorkerErrorsCount = 15;
};
