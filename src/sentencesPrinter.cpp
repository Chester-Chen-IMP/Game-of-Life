#include "sentencesPrinter.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#include "utf8/checked.h"

inline void sleep(unsigned int delay) {
  std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}

void SentencePrinter::print() const {
  std::cout << "\033[2J\033[H" << std::flush;
  for (int i = 0; i < 6; ++i) {
    slowPrint(context[i]);
  }
}

void SentencePrinter::slowPrint(const std::string& text) const {
  setenv("LANG", "en_US.UTF-8", 1);
#ifdef DEBUG
  std::string accumulated;
#endif
  auto it = text.begin();

  while (it != text.end()) {
    uint32_t codeprint = utf8::next(it, text.end());
    std::string utf8_char;
    utf8::append(codeprint, std::back_inserter(utf8_char));
#ifdef DEBUG
    accumulated += utf8_char;
#endif
    std::cout << utf8_char << std::flush;
    sleep(80);
  }

  sleep(500);
}