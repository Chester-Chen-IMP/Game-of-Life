#include "../include/seedSaver.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "../include/getInput.hpp"

bool seedSaver(const std::string& seed) {
  auto now = std::chrono::system_clock::now();
  std::time_t time_now = std::chrono::system_clock::to_time_t(now);

  std::tm* local_time = std::localtime(&time_now);

  std::stringstream ss;
  ss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
  std::string formatted_time = ss.str();

  std::string processed_seed{};
  int elem_counts = 0;
  bool should_feed_line = false;
  bool is_all_blank = true;
  {
    bool is_current_blank = true;

    // 检测是否种子全空（即游戏时没有输入任何内容）
    for (auto it = seed.begin(); it != seed.end(); ++it) {
      is_current_blank = *it == '0';
      if (!is_current_blank) {
        is_all_blank = false;
        break;
      }
    }
  }

  if (!is_all_blank) {
    for (auto it = seed.begin(); it != seed.end(); ++it) {
      elem_counts++;
      should_feed_line = elem_counts % CELLS_AREA == 0;
      std::string each(1, *it);

      if (std::next(it) == seed.end()) {
        processed_seed += each;
        std::ofstream seed_file("../bin/seed.txt", std::ios::app);
        seed_file << "Seed saved at " << formatted_time << " {\n"
                  << processed_seed << "}\n";
        break;
      }
      processed_seed += (each + ", ");
      if (should_feed_line) processed_seed += '\n';
    }
  }

  return is_all_blank;
}