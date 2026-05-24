#pragma once

#include <signal.h>
#include <spawn.h>

#include <string>

class AudioPlayer {
  pid_t pid = -1;

 public:
  void setupSpawnActions(posix_spawn_file_actions_t* actions);

  bool play(const std::string& file);

  void stop();

  ~AudioPlayer();
};