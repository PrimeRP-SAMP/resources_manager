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

#include "rm_tree.h"
#include "deferred_function.hpp"
#include <common.hpp>

rm_tree::rm_tree(std::filesystem::path base_path)
    : base_path(std::move(base_path)) {
  static bool once = false;
  if (!once) {
    curl_global_init(CURL_GLOBAL_ALL);
    once = true;
  }
  worker.current_cdn = cdns.cend();
}

rm_tree::rm_tree(const rm_tree &tree)
    : base_path(tree.base_path), cdns(tree.cdns) {
  worker.current_cdn = cdns.cend();
  // Only root project can have dependencies, so don't copy them
  // This is a copy constructor which gets called only within add_dependency function
  // After any task has started, an app can't add new dependencies for safety reasons
}

rm_tree::~rm_tree() {
  if (worker.self != nullptr) {
    if (worker.self->joinable()) {
      worker.process_data.force_stop = true;
      worker.self->join();
    }
    delete worker.self;
    worker.self = nullptr;
  }
}

// Common

error_code_t rm_tree::add_cdn(const rm_cdn &cdn) {
  if (is_working())
    return kCannotWhenWorking;
  cdns.emplace_back(cdn);
  worker.current_cdn = cdns.cend();
  return kNoError;
}

error_code_t rm_tree::add_dependency(const rm_tree &dependency) {
  if (is_working())
    return kCannotWhenWorking;
  if (worker.last_state != worker_mode_t::kNone)
    return kCannotAfterStarted;
  dependencies.emplace_back(dependency).base_path = base_path;
  return kNoError;
}

// Fetcher

error_code_t rm_tree::fetch_updates() {
  if (is_working())
    return kCannotWhenWorking;
  items.clear();
  worker.pending_download_items.clear();
  worker.worker_error.reset();
  worker.current_state = worker_mode_t::kFetching;
  summon_worker(updates_fetcher_worker);
  return kNoError;
}

bool rm_tree::fetching() const {
  return worker.current_state == worker_mode_t::kFetching;
}

bool rm_tree::fetched() const {
  return !fetching() && worker.last_state == worker_mode_t::kFetching;
}

error_code_t rm_tree::stop_fetching() {
  if (!fetching())
    return kCannotWhenNotWorking;

  worker.process_data.force_stop = true;
//  while (fetching()); // spinlock
  return kNoError;
}

// Downloader

error_code_t rm_tree::download() {
  if (is_working())
    return kCannotWhenWorking;

  if (get_pending_download_files_count() <= 0)
    return kCannotWhenNothingToDownload;

  worker.worker_error.reset();
  worker.current_state = worker_mode_t::kDownloading;
  summon_worker(download_worker);
  return kNoError;
}

bool rm_tree::downloading() const {
  return worker.current_state == worker_mode_t::kDownloading;
}

bool rm_tree::downloaded() const {
  return !downloading() && worker.last_state == worker_mode_t::kDownloading;
}

error_code_t rm_tree::stop_download() {
  if (!downloading())
    return kCannotWhenNotWorking;

  worker.process_data.force_stop = true;
//  while (downloading()); // spinlock
  return kNoError;
}

// Checker

error_code_t rm_tree::check() {
  if (is_working())
    return kCannotWhenWorking;

  worker.worker_error.reset();
  worker.current_state = worker_mode_t::kChecking;
  summon_worker(check_worker);
  return kNoError;
}

bool rm_tree::checking() const {
  return worker.current_state == worker_mode_t::kChecking;
}

bool rm_tree::checked() const {
  return !checking() && worker.last_state == worker_mode_t::kChecking;
}

error_code_t rm_tree::stop_check() {
  if (!checking())
    return kCannotWhenNotWorking;

  worker.process_data.force_stop = true;
//  while (checking()); // spinlock
  return kNoError;
}

// Remover modifications

error_code_t rm_tree::remove_modifications() {
  if (is_working())
    return kCannotWhenWorking;

  worker.worker_error.reset();
  worker.current_state = worker_mode_t::kRemovingModifications;
  summon_worker(remove_modifications_worker);
  return kNoError;
}

bool rm_tree::removing_modifications() const {
  return worker.current_state == worker_mode_t::kRemovingModifications;
}

bool rm_tree::removed_modifications() const {
  return !removed_modifications() && worker.last_state == worker_mode_t::kRemovingModifications;
}

error_code_t rm_tree::stop_remove_modifications() {
  if (!removing_modifications())
    return kCannotWhenNotWorking;

  worker.process_data.force_stop = true;
//  while (removing_modifications()); // spinlock
  return kNoError;
}

// Workers data helpers

bool rm_tree::has_worker_error() const {
  return worker.worker_error.has_value();
}

bool rm_tree::is_system_error() const {
  if (!has_worker_error())
    return false;
  return std::get_if<std::system_error>(&worker.worker_error.value()) != nullptr;
}

bool rm_tree::is_runtime_error() const {
  if (!has_worker_error())
    return false;
  return std::get_if<std::runtime_error>(&worker.worker_error.value()) != nullptr;
}

int rm_tree::get_worker_sys_error_code() const {
  if (!has_worker_error())
    return 0;
  if (!is_system_error())
    return -1;
  return std::get<std::system_error>(worker.worker_error.value()).code().value();
}

std::string rm_tree::get_worker_error_str() const {
  if (!has_worker_error())
    return "";
  return std::visit([](auto &&v) { return v.what(); }, worker.worker_error.value());
}

uint64_t rm_tree::get_total_work_amount() const {
  return worker.process_data.total_work_amount;
}

uint64_t rm_tree::get_completed_work_amount() {
  size_t processed = worker.process_data.processed_work_amount;
  if (downloading()) {
    std::scoped_lock dl_workers_lock(worker.download_workers_mtx);
    for (auto &worker : worker.download_workers) {
      processed += worker->downloaded_size;
    }
  }
  for (auto &dependency : dependencies) {
    processed += dependency.get_completed_work_amount();
  }
  return processed;
}

// Helpers

bool rm_tree::is_working() const {
  return worker.current_state != worker_mode_t::kNone;
}

std::filesystem::path rm_tree::get_entry_full_path(const rm_entry &entry) const {
  return base_path / entry.relative_path;
}

void rm_tree::summon_worker(rm_tree::worker_t worker_fn) {
  if (worker_fn == nullptr)
    return;
  worker_release();
  worker.self = new std::thread(worker_fn, std::ref(*this), nullptr);
}

void rm_tree::worker_release() {
  if (is_working() || worker.self == nullptr)
    return;
  if (worker.self->joinable())
    worker.self->join();
  delete worker.self;
  worker.self = nullptr;
}

rm_tree::cdns_const_iterator rm_tree::get_current_cdn(uint64_t offset) {
  static std::mutex mtx;
  std::scoped_lock lock(mtx);

  auto &current_cdn_errors_count = worker.current_cdn_errors_count;
  auto &current_cdn = worker.current_cdn;
  auto cdns_count = cdns.size();
  auto cdns_begin = cdns.cbegin();
  auto cdns_end = cdns.cend();

  if ((current_cdn_errors_count >= kMaxCdnErrorsCount && cdns_count > 1) || current_cdn == cdns_end) {
    if (cdns_count == 1) {
      current_cdn = cdns_begin;
    } else {
      if (offset == 0)
        offset =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
      std::default_random_engine def_engine(offset);
      std::uniform_int_distribution<size_t> distro(0, cdns_count - 1);
      auto posn = distro(def_engine);
      auto picked_cdn = cdns_begin + posn;

      if (current_cdn == cdns_end) {
        current_cdn = picked_cdn;
      } else if (cdns_count == 2) {
        current_cdn = cdns_begin + (current_cdn == cdns_begin ? 1 : 0);
      } else if (picked_cdn == current_cdn) {
        current_cdn = get_current_cdn(offset + 1);
      }
    }
    current_cdn_errors_count = 0;
  }
  return current_cdn;
}

std::string rm_tree::fetch_url_path_content(const std::string &path) {
  auto error_code = CURL_LAST;
  std::string ret;
  auto write_fn = +[](void *contents, size_t size, size_t nmemb, void *userp) -> size_t {
    auto downloaded_size = size * nmemb;
    reinterpret_cast<decltype(&ret)>(userp)->append(reinterpret_cast<char *>(contents), downloaded_size);
    L_VERBOSE(1, "Fetch URL path content process: {}", downloaded_size);
    return downloaded_size;
  };
  auto max_fails_count = kMaxCdnErrorsCount * cdns.size();
  auto fails_count = 0;
  while (error_code != CURLE_OK) {
    auto cdn = get_current_cdn();
    auto init = cdn->easy_init(path);
    auto is_http = cdn->is_http();
    auto &ch = init.ch;
    if (ch == nullptr)
      throw std::runtime_error("Could not initialize CURL channel");

    ret.clear();
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, write_fn);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, &ret);
    error_code = curl_easy_perform(ch);
    curl_easy_cleanup(ch);

    std::string error_str;
    deferred_function def_error([&]() {
      if (!error_str.empty()) {
        L_ERROR("Error during fetching url path content: {}", error_str);
      }
    });
    long response_code = 0;
    switch (error_code) {
    case CURLE_OK:curl_easy_getinfo(ch, CURLINFO_RESPONSE_CODE, &response_code);
      break;
    case CURLE_COULDNT_RESOLVE_PROXY:error_str = "Couldn't resolve proxy.";
      break;
    case CURLE_COULDNT_RESOLVE_HOST:error_str = "Couldn't resolve host.";
      break;
    case CURLE_COULDNT_CONNECT:error_str = "Couldn't connect to host.";
      break;
    case CURLE_REMOTE_ACCESS_DENIED:error_str = "Couldn't connect to host: remote access denied.";
      break;
    default:error_str = "Unknown CURL error: " + std::to_string(error_code) + ".";
      break;
    }

    const char *url = nullptr;
    curl_easy_getinfo(ch, CURLINFO_EFFECTIVE_URL, &url);

    if (error_code == CURLE_OK && is_http && response_code != 200) {
      error_str = "The request was proceeded correctly, but host returned an unknown HTTP code: "
          + std::to_string(response_code) + ".";
      error_code = CURL_LAST;
    }

    auto has_error = !error_str.empty() || (is_http && response_code != 200);
    if (has_error) {
      ++fails_count;
      error_str += " Problematic URL path was: ";
      error_str += url != nullptr ? url : "Unknown url";
    }

    if (fails_count >= max_fails_count) {
      error_str += " Could't receive data " + std::to_string(fails_count) + " times.";
      throw std::runtime_error(error_str);
    }
  }
  return ret;
}

// Download helpers

auto rm_tree::find_download_worker(CURL *easy_handler) {
  for (auto item = worker.download_workers.begin(); item != worker.download_workers.end(); ++item) {
    if (*item && (*item)->init.ch == easy_handler) {
      return item;
    }
  }
  return worker.download_workers.end();
}

auto rm_tree::find_pending_download_item(const rm_entry &entry) {
  for (auto item = worker.pending_download_items.begin(); item != worker.pending_download_items.end(); ++item) {
    if (item->value == entry) {
      return item;
    }
  }
  return worker.pending_download_items.end();
}

size_t rm_tree::get_pending_download_files_count(bool include_dependencies) const {
  size_t ret = worker.pending_download_files_count;
  if (include_dependencies) {
    for (auto &dependency : dependencies) {
      ret += dependency.get_pending_download_files_count();
    }
  }
  return ret;
}

// Checker helpers

bool rm_tree::is_entry_valid(const rm_entry &entry) const {
  auto full_path = get_entry_full_path(entry);
  if (!exists(full_path) || !is_regular_file(full_path))
    return false;
  auto file_sz = file_size(full_path);
  if (file_sz != entry.size)
    return false;
  return common::get_file_hash(full_path) == entry.fnv_hash;
}

size_t rm_tree::get_entries_count(bool include_dependencies) const {
  auto ret = items.size();
  if (include_dependencies) {
    for (auto &dependency : dependencies) {
      ret += dependency.get_entries_count();
    }
  }
  return ret;
}

uint64_t rm_tree::get_pending_items_download_size(bool include_dependencies) const {
  uint64_t ret = 0;
  for (auto &entry : worker.pending_download_items) {
    ret += entry.value.compressed ? entry.value.compressed_size : entry.value.size;
  }
  if (include_dependencies) {
    for (auto &dependency : dependencies) {
      ret += dependency.get_pending_items_download_size();
    }
  }
  return ret;
}

// Modifications remover helpers

std::vector<rm_entry> rm_tree::get_all_entries(bool include_dependencies) const {
  auto entries = items;
  if (include_dependencies) {
    for (auto &dependency : dependencies) {
      auto dep_entries = dependency.get_all_entries();
      entries.insert(entries.end(), dep_entries.begin(), dep_entries.end());
    }
  }
  return entries;
}

// Workers

void rm_tree::download_worker(rm_tree &tree, worker_process_data_t *process_data) {
  auto &worker = tree.worker;
  auto &downloader_data = process_data != nullptr ? *process_data : worker.process_data;
  auto &total_download_size = downloader_data.total_work_amount;
  auto &downloaded_size = downloader_data.processed_work_amount;
  auto &pending_download_items = worker.pending_download_items;
  auto &download_workers = worker.download_workers;

  if (process_data == nullptr) { // only root tree has to do this
    downloaded_size = 0;
    total_download_size = tree.get_pending_items_download_size();
  }

  auto write_fn = +[](void *contents, size_t size, size_t nmemb, void *userp) -> size_t {
    auto this_worker = reinterpret_cast<download_worker_job_t *>(userp);
    if (this_worker->abort)
      return CURL_READFUNC_ABORT;
    auto downloaded_size = size * nmemb;
    L_VERBOSE(1, "Downloaded worker process (bytes): {}, file: {}", downloaded_size, this_worker->item.relative_path.string());
    this_worker->downloaded_size += downloaded_size;
    this_worker->stream.write(reinterpret_cast<char *>(contents), downloaded_size);
    return downloaded_size;
  };

  try {
    auto curlm = curl_multi_init();
    if (curlm == nullptr)
      throw std::runtime_error("Could not initialize curl multi handle");

    deferred_function scoped_curlm([&]() {
      std::scoped_lock dl_workers_lock(worker.download_workers_mtx);
      download_workers.clear();
      if (curlm != nullptr) {
        curl_multi_cleanup(curlm);
        curlm = nullptr;
      }
    });

    while (!pending_download_items.empty()) {
      {
        std::scoped_lock dl_workers_lock(worker.download_workers_mtx);

        for (auto i = 0; i < kParallelJobsCount; ++i) {
          if (i >= pending_download_items.size() || download_workers.size() >= kParallelJobsCount)
            break;
          auto &pending_download_item = pending_download_items.at(i);
          auto &entry = pending_download_item.value;
          auto &job = download_workers.emplace_back();
          job = std::make_unique<download_worker_job_t>();
          job->item = entry;
          job->init = std::move(tree.get_current_cdn()->easy_init(entry.relative_path.string()));
          job->is_http = tree.get_current_cdn()->is_http();
          job->downloaded_size = 0;
          auto full_path = tree.get_entry_full_path(entry);
          if (entry.compressed)
            full_path += ".zst";
          if (exists(full_path))
            remove(full_path);
          if (!exists(full_path.parent_path()))
            create_directories(full_path.parent_path());
          job->stream.open(full_path, std::ios::out | std::ios::binary);
          curl_easy_setopt(job->init.ch, CURLOPT_WRITEFUNCTION, write_fn);
          curl_easy_setopt(job->init.ch, CURLOPT_WRITEDATA, job.get());
          job->init.link_to_curlm(curlm);
        }
      }
      int still_running = 0;
      do {
        auto mc = curl_multi_perform(curlm, &still_running);
        if (mc == CURLM_OK)
          mc = curl_multi_wait(curlm, nullptr, 0, 300, nullptr);
        if (mc != CURLM_OK) {
          std::string error_str = "Unknown CURLM error: ";
          error_str += std::to_string(mc);
          throw std::runtime_error(error_str);
        }
        if (downloader_data.force_stop) {
          for (auto &worker : worker.download_workers) {
            worker->abort = true;
          }
        }
      } while (still_running > 0);
      if (downloader_data.force_stop)
        throw std::runtime_error("Force stopped downloader process");
      int msgs_left = 0;
      while (auto msg = curl_multi_info_read(curlm, &msgs_left)) {
        if (msg->msg == CURLMSG_DONE) {
          auto ch = msg->easy_handle;
          auto error_code = msg->data.result;
          auto dl_worker = tree.find_download_worker(ch);
          if (dl_worker == download_workers.end()) {
            // How this even possible? Dunno what to do
            curl_multi_remove_handle(curlm, ch);
            curl_easy_cleanup(ch);
            continue;
          }
          auto dl_worker_content = dl_worker->get();
          auto pending_download_item = tree.find_pending_download_item(dl_worker_content->item);
          if (pending_download_item == pending_download_items.end()) {
            // How this even possible? Dunno what to do
            std::scoped_lock dl_workers_lock(worker.download_workers_mtx);
            curl_multi_remove_handle(curlm, ch);
            download_workers.erase(dl_worker);
            continue;
          }
          auto is_http = dl_worker_content->is_http;

          std::string error_str;
          deferred_function def_error([&]() {
            if (!error_str.empty()) {
              L_ERROR("Error during downloading files: {}", error_str);
            }
          });
          long response_code = 0;
          switch (error_code) {
          case CURLE_ABORTED_BY_CALLBACK:
            // code won't reach here, but anyway, just to be sure
            throw std::runtime_error("Force stopped downloader process");
            break;
          case CURLE_OK:curl_easy_getinfo(ch, CURLINFO_RESPONSE_CODE, &response_code);
            break;
          case CURLE_COULDNT_RESOLVE_PROXY:error_str = "Couldn't resolve proxy.";
            break;
          case CURLE_COULDNT_RESOLVE_HOST:error_str = "Couldn't resolve host.";
            break;
          case CURLE_COULDNT_CONNECT:error_str = "Couldn't connect to host.";
            break;
          case CURLE_REMOTE_ACCESS_DENIED:error_str = "Couldn't connect to host: remote access denied.";
            break;
          default:error_str = "Unknown CURL error: " + std::to_string(error_code) + ".";
            break;
          }

          const char *url = nullptr;
          curl_easy_getinfo(ch, CURLINFO_EFFECTIVE_URL, &url);

          if (error_code == CURLE_OK && is_http && response_code != 200) {
            error_str = "The request was proceeded correctly, but host returned an unknown HTTP code: "
                + std::to_string(response_code) + ".";
            error_code = CURL_LAST;
          }
          size_t worker_downloaded_size = dl_worker->get()->downloaded_size;
          {
            std::scoped_lock dl_workers_lock(worker.download_workers_mtx);
            download_workers.erase(dl_worker);
          }

          auto has_errors = !error_str.empty() || (is_http && response_code != 200);
          if (!has_errors) {
            downloaded_size += worker_downloaded_size;
            auto fullpath = tree.get_entry_full_path(pending_download_item->value);
            auto compressed = pending_download_item->value.compressed;
            if (compressed) {
              L_INFO("File {} is compressed, decompressing...", pending_download_item->value.relative_path.string());
              std::filesystem::path compressed_filename = fullpath.string() + ".zst";
              common::decompress_file(compressed_filename, fullpath);
              remove(compressed_filename);
              L_INFO("File {} is decompressed successfully", pending_download_item->value.relative_path.string());
            }
            L_INFO("File {} is downloaded successfully", pending_download_item->value.relative_path.string());
            pending_download_items.erase(pending_download_item);
            --worker.pending_download_files_count;
          } else {
            ++worker.current_cdn_errors_count;
            ++pending_download_item->errors_count;
            error_str += " Problematic URL path was: ";
            error_str += url != nullptr ? url : "Unknown url";
            error_str += " Problematic file: ";
            error_str += pending_download_item->value.relative_path.string();

            if (pending_download_item->errors_count >= kMaxDownloadWorkerErrorsCount) {
              throw std::runtime_error(error_str);
            }
          }
        }
      }
    }
    for (auto &dependency : tree.dependencies) {
      download_worker(dependency, &downloader_data);
    }
  } catch (const std::system_error &fail) {
    if (process_data != nullptr)
      throw fail; // let the parent to handle it
    tree.worker.worker_error = fail;
  } catch (const std::runtime_error &fail) {
    if (process_data != nullptr)
      throw fail; // let the parent to handle it
    tree.worker.worker_error = fail;
  } catch (const std::exception &exc) {
    if (process_data != nullptr)
      throw exc; // let the parent to handle it
    tree.worker.worker_error = exc;
  }

  if (tree.has_worker_error()) {
    L_ERROR("Exception during downloading files: {}", tree.get_worker_error_str());
  }
  L_INFO("Files downloader completed");

  worker.last_state = worker.current_state.load();
  worker.current_state = worker_mode_t::kNone;
}

void rm_tree::updates_fetcher_worker(rm_tree &tree, worker_process_data_t *process_data) {
  auto &worker = tree.worker;
  auto &fetcher_data = process_data != nullptr ? *process_data : worker.process_data;

  L_INFO("Updates fetcher started");
  try {
    auto data = tree.fetch_url_path_content(common::kResourcesDataFilename);
    if (fetcher_data.force_stop)
      throw std::runtime_error("Force stopped check worker");
    auto data_json = nlohmann::json::parse(data);
    tree.items.clear();
    for (auto &entry : data_json) {
      tree.items.emplace_back(entry);
    }
    for (auto &dependency : tree.dependencies) {
      updates_fetcher_worker(dependency, &fetcher_data);
    }
  } catch (const std::system_error &fail) {
    if (process_data != nullptr)
      throw fail; // let the parent to handle it
    tree.worker.worker_error = fail;
  } catch (const std::runtime_error &fail) {
    if (process_data != nullptr)
      throw fail; // let the parent to handle it
    tree.worker.worker_error = fail;
  } catch (const std::exception &exc) {
    if (process_data != nullptr)
      throw exc; // let the parent to handle it
    tree.worker.worker_error = exc;
  }

  if (tree.has_worker_error()) {
    L_ERROR("Exception during fetching files update: {}", tree.get_worker_error_str());
  }
  L_INFO("Updates fetcher completed");

  worker.last_state = worker.current_state.load();
  worker.current_state = worker_mode_t::kNone;
}

void rm_tree::check_worker(rm_tree &tree, worker_process_data_t *process_data) {
  auto &worker = tree.worker;
  auto &checker_data = process_data != nullptr ? *process_data : worker.process_data;
  auto &total_check_files_count = checker_data.total_work_amount;
  auto &checked_files_count = checker_data.processed_work_amount;
  auto &pending_download_items = worker.pending_download_items;
  auto &items = tree.items;

  pending_download_items.clear();
  if (process_data == nullptr) { // only root tree has to do this
    total_check_files_count = tree.get_entries_count();
    checked_files_count = 0;
  }

  L_INFO("Files checker started");
  try {
    for (auto &item : items) {
      if (checker_data.force_stop)
        throw std::runtime_error("Force stopped check worker");

      if (!tree.is_entry_valid(item))
        pending_download_items.emplace_back(item);
      ++checked_files_count;
    }
    for (auto &dependency : tree.dependencies) {
      check_worker(dependency, &checker_data);
    }
  } catch (const std::system_error &fail) {
    if (process_data != nullptr)
      throw fail; // let the parent to handle it
    tree.worker.worker_error = fail;
  } catch (const std::runtime_error &fail) {
    if (process_data != nullptr)
      throw fail; // let the parent to handle it
    tree.worker.worker_error = fail;
  } catch (const std::exception &exc) {
    if (process_data != nullptr)
      throw exc; // let the parent to handle it
    tree.worker.worker_error = exc;
  }

  if (tree.has_worker_error()) {
    L_ERROR("Exception during checking files: {}", tree.get_worker_error_str());
  }
  worker.pending_download_files_count = pending_download_items.size();
  L_INFO("Files checker completed");

  worker.last_state = worker.current_state.load();
  worker.current_state = worker_mode_t::kNone;
}

void rm_tree::remove_modifications_worker(rm_tree &tree, worker_process_data_t *process_data) {
  throw std::runtime_error("Not implemented");
}
