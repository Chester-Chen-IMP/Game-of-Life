#include "sentencesPrinter.hpp"

#include <locale.h>
#include <ncurses.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <string>
#include <thread>

#include "utf8/checked.h"

namespace {

  constexpr int kDefaultDelayChars         = 80;
  constexpr int kDefaultDelaySentences     = 500;
  constexpr int kAcceleratedDelayChars     = 10;
  constexpr int kAcceleratedDelaySentences = 80;
  constexpr int kLongPressMs               = 300;
  constexpr int kPollIntervalMs            = 20;

  // Release threshold must be larger than typical auto-repeat delay so that
  // the absence of immediate repeat events doesn't get treated as key release.
  constexpr int kReleaseThresholdMs = 400;

  std::atomic<int> delay_chars {kDefaultDelayChars};
  std::atomic<int> delay_sentences {kDefaultDelaySentences};

  bool g_space_down        = false;
  bool g_space_accelerated = false;
  std::chrono::steady_clock::time_point g_space_press_start;
  std::chrono::steady_clock::time_point g_last_space_seen =
    std::chrono::steady_clock::time_point::min();

  struct CursesGuard {
    CursesGuard() {
      setlocale(LC_ALL, "");
      initscr();
      cbreak();
      noecho();
      keypad(stdscr, TRUE);
      nodelay(stdscr, TRUE);
      ESCDELAY = 0;
      curs_set(0);
      clear();
      refresh();
    }

    ~CursesGuard() { endwin(); }
  };

  void resetLongPressState() {
    g_space_down        = false;
    g_space_accelerated = false;
    delay_chars         = kDefaultDelayChars;
    delay_sentences     = kDefaultDelaySentences;
    g_last_space_seen   = std::chrono::steady_clock::time_point::min();
  }

  void pollLongPressState() {
    int ch   = getch();
    auto now = std::chrono::steady_clock::now();
    if (ch == ' ') {
      // Record that we saw a space event now. On first detection mark press
      // start time; on subsequent repeats just update last-seen time.
      if (!g_space_down) {
        g_space_down        = true;
        g_space_accelerated = false;
        g_space_press_start = now;
        g_last_space_seen   = now;
      } else {
        g_last_space_seen = now;
        if (!g_space_accelerated) {
          auto held_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now - g_space_press_start
          )
                           .count();
          if (held_ms >= kLongPressMs) {
            g_space_accelerated = true;
            delay_chars         = kAcceleratedDelayChars;
            delay_sentences     = kAcceleratedDelaySentences;
          }
        }
      }
    } else if (ch == ERR) {
      // No input this poll. Consider it a release only if we haven't seen a
      // space event for a while (longer than keyboard's auto-repeat delay).
      if (g_space_down) {
        auto since_last_space_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(
            now - g_last_space_seen
          )
            .count();
        if (since_last_space_ms >= kReleaseThresholdMs) {
          g_space_down        = false;
          g_space_accelerated = false;
          delay_chars         = kDefaultDelayChars;
          delay_sentences     = kDefaultDelaySentences;
        }
      }
    }
  }

  void waitWithPolling(int total_ms) {
    auto start = std::chrono::steady_clock::now();
    while (true) {
      auto now = std::chrono::steady_clock::now();
      auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
          .count();
      if (elapsed >= total_ms) break;
      pollLongPressState();
      std::this_thread::sleep_for(std::chrono::milliseconds(
        std::min(kPollIntervalMs, total_ms - static_cast<int>(elapsed))
      ));
    }
  }

}  // namespace

void SentencePrinter::print() const {
  CursesGuard guard;
  resetLongPressState();
  for (const auto& line : context) slowPrint(line);
}

void SentencePrinter::slowPrint(const std::string& text) const {
#ifdef DEBUG
  std::string accumulated;
#endif
  auto it = text.begin();

  while (it != text.end()) {
    uint32_t codepoint = utf8::next(it, text.end());
    std::string utf8_char;
    utf8::append(codepoint, std::back_inserter(utf8_char));
#ifdef DEBUG
    accumulated += utf8_char;
#endif
    addstr(utf8_char.c_str());
    refresh();
    waitWithPolling(delay_chars.load());
  }

  waitWithPolling(delay_sentences.load());
}