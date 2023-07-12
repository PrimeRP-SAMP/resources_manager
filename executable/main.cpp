#include <iostream>
#include <filesystem>

#include <json.hpp>
#include <common.hpp>

constexpr size_t kMaxUncompressedFilesize = 16 * 1024 * 1024;
constexpr int kCompressionLevel = 3;

auto json_container = nlohmann::json::array();

std::filesystem::path in_path(const std::filesystem::path &relative_path) {
  return "./in" / relative_path;
}

std::filesystem::path out_path(const std::filesystem::path &relative_path) {
  return "./out" / relative_path;
}

void process_file(const std::filesystem::path &relative_path) {
  std::cout << "Processing a file: " << relative_path << std::endl;
  auto in_path_ = in_path(relative_path);
  auto out_path_ = out_path(relative_path);
  auto compress = file_size(in_path_) >= kMaxUncompressedFilesize;
  if (!is_directory(out_path_.parent_path()))
    create_directories(out_path_.parent_path());
  if (compress) {
    std::cout << "File is big, compressing..." << std::endl;
    common::compress_file(in_path_, out_path_, kCompressionLevel);
  } else {
    std::cout << "File is small, just copying instead..." << std::endl;
    copy(in_path_, out_path_);
  }
  auto obj = nlohmann::json::object();
  obj["p"] = relative_path.string();
  obj["s"] = static_cast<uint64_t>(file_size(in_path_));
  obj["h"] = common::get_file_hash(in_path_);
  obj["c"] = compress;
  if (compress) {
    obj["cs"] = static_cast<uint64_t>(file_size(out_path_));
    obj["ch"] = common::get_file_hash(out_path_);
  }
  json_container += obj;
}

void process_directory(const std::filesystem::path &relative_path) {
  std::cout << "Processing directory: " << relative_path << std::endl;
  for (auto &this_path : std::filesystem::directory_iterator{in_path(relative_path)}) {
    auto relative_this_path = std::filesystem::relative(this_path.path(), in_path("."));
    if (is_regular_file(this_path)) {
      auto extension = common::str_tolower(relative_path.extension().string());
      if (extension == ".log") continue;
      process_file(relative_this_path);
    } else if (is_directory(this_path)) {
      process_directory(relative_this_path);
    }
  }
}

int main() {
  auto in_path_ = in_path(".");
  auto out_path_ = out_path(".");
  if (!is_directory(in_path_)) {
    std::cout << "Could not locate 'in' folder. Create it and put input files here!" << std::endl;
    return 1;
  }
  if (exists(out_path_))
    remove_all(out_path_);
  process_directory(".");
  std::ofstream outfile;
  outfile.open(out_path(common::kResourcesDataFilename));
  outfile << json_container;
}
