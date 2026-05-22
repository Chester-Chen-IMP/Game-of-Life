#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "gameOfLife.hpp"
#include "sentencesPrinter.hpp"

void gameRules() {
  SentencePrinter rules;
  rules.print();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  std::cout << "进入游戏吗？y(是)/n(否)/r(重新打印规则)：";
  char enter;
  int retry_times = 0;
retry:
  retry_times++;
  if (retry_times > 3) {
    std::cout << "\n\n次数达到上限，退出。\n";
    return;
  }
  std::cin >> enter;
  switch (enter) {
    case 'y':
    case 'Y':
      game();
      break;

    case 'n':
    case 'N':
    case 'q':
    case 'Q':
      return;
      break;

    case 'r':
    case 'R':
      gameRules();
      break;

    default:
      std::cout << "\n\n未识别的选项，请重试（y/n/r）：";
      goto retry;
  }
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    game();
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      std::cout << "Usage: program [options]\n";
      std::cout << "       -h, --help 显示游戏帮助\n";
      std::cout << "       -r, --rule 显示游戏规则并进入\n";
    }

    if (arg == "-r" || arg == "--rule") {
      gameRules();
    }
  }
  return 0;
}