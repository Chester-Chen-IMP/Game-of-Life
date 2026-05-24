#include "../include/seedSaver.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "../include/getInput.hpp"

bool seedSaver(const std::string& seed, int iteration_counts) {
  if (seed.find('1') == std::string::npos) {
    return false;
  }

  std::ofstream seed_file("./seed.txt", std::ios::app);
  if (!seed_file) {
    return false;
  }

  auto now = std::chrono::system_clock::now();
  std::time_t time_now = std::chrono::system_clock::to_time_t(now);
  std::tm* local_time = std::localtime(&time_now);

  std::stringstream ss;
  ss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");

  std::string processed_seed;
  processed_seed.reserve(seed.size() * 3);
  for (size_t index = 0; index < seed.size(); ++index) {
    processed_seed.push_back(seed[index]);
    if (index + 1 != seed.size()) {
      processed_seed += ", ";
    }
    if ((index + 1) % CELLS_AREA == 0 && index + 1 != seed.size()) {
      processed_seed.push_back('\n');
    }
  }

  seed_file << "Seed (number of iterations:" << iteration_counts << ") "
            << "saved at " << ss.str() << " {\n"
            << processed_seed << "}\n";
  return true;
}

bool seedSaver(const std::string& seed) {
  return seed.find('1') != std::string::npos;
}
