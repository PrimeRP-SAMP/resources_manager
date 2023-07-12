BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    if (!google::IsGoogleLoggingInitialized()) {
      google::InitGoogleLogging("resources_downloader");
    }
  } else if (fdwReason == DLL_PROCESS_DETACH) {
    // curl_global_cleanup(); should be there?
  }
  return TRUE;
}

//int main() {
//  auto tree = new rm_tree(R"(D:\TestGame2)");
//  auto cdn = new rm_cdn("http://localhost/gta/gta");
//  cdn->add_header("Test", "HeaderValue");
//  tree->add_cdn(*cdn);
//  tree->fetch_updates();
//  while (tree->fetching()) {}
//  tree->check();
//  while (tree->checking()) {}
//  tree->download();
//  while (tree->downloading()) {
//    auto completed = tree->get_completed_work_amount();
//    auto total = tree->get_total_work_amount();
//    double percentage = (100.0 * completed) / total;
//    std::cout << "Downloading progress: " << percentage << std::endl;
//    std::this_thread::sleep_for(std::chrono::milliseconds(150));
//  }
////  std::cout << tree->fetch_url_path_content("version.php") << std::endl;
//  return 0;
//}
