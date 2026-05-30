#include <spawn.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "audioPlayer.hpp"
#include "gameOfLife.hpp"
#include "sentencesPrinter.hpp"

void gameRules() {
  setenv("LANG", "en_US.UTF-8", 1);
  SentencePrinter rules;
  int retry_times = 0;
  while (true) {
    rules.print();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "进入游戏吗？y(是)/n(否)/r(重新打印规则)：";
    char enter;
    std::cin >> enter;
    switch (enter) {
      case 'y':
      case 'Y':
        game();
        return;
      case 'n':
      case 'N':
      case 'q':
      case 'Q':
        return;
      case 'r':
      case 'R':
        break;
      default:
        retry_times++;
        if (retry_times >= 3) {
          std::cout << "\n\n次数达到上限，退出。\n";
          return;
        }
        std::cout << "\n\n未识别的选项，请重试（y/n/r）：";
        continue;
    }
    // 如果是 'r'，重置重试次数并继续循环
    retry_times = 0;
  }
}

int main(int argc, char* argv[]) {
  posix_spawn_file_actions_t actions;
  AudioPlayer bgm;
  bgm.setupSpawnActions(&actions);
  bgm.play("/home/MoxxieIMP/Documents/gameOfLife/src/music/lofi.mp3");
  setenv("LANG", "en_US.UTF-8", 1);
  if (argc == 1) {
    game();
    return 0;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      std::cout << "Usage: program [options]\n";
      std::cout << "       -h, --help 显示游戏帮助\n";
      std::cout << "       -r, --rule 显示游戏规则并进入\n";
      std::cout << "       -d, --direct 直接进入游戏\n";
      std::cout
        << "使用-d/--direct选项或不使用参数时，"
        << "需要按下回车键进入游戏。\n";
    } else if (arg == "-r" || arg == "--rule") {
      gameRules();
    } else if (arg == "-d" || arg == "--direct") {
      game();
    } else {
      std::cerr << "未知选项: " << arg << '\n';
      return 1;
    }
  }
  bgm.stop();
  return 0;
}