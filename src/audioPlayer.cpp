#include "audioPlayer.hpp"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

extern char** environ;

void AudioPlayer::setupSpawnActions(posix_spawn_file_actions_t* actions) {
  posix_spawn_file_actions_init(actions);

  int devnull = open("/dev/null", O_RDWR);
  if (devnull < 0) {
    posix_spawn_file_actions_destroy(actions);
    return;
  }
  posix_spawn_file_actions_adddup2(actions, devnull, STDOUT_FILENO);
  posix_spawn_file_actions_adddup2(actions, devnull, STDERR_FILENO);
  posix_spawn_file_actions_adddup2(actions, devnull, STDIN_FILENO);
  // 父进程中手动 close(devnull)
  posix_spawn_file_actions_addclose(actions, devnull);  // 子进程关闭 devnull
}

bool AudioPlayer::play(const std::string& file) {
  char* const argv[] = {(char*)"mpv", (char*)file.c_str(), (char*)"--no-video",
                        (char*)"--really-quiet", nullptr};
  posix_spawn_file_actions_t actions;
  setupSpawnActions(&actions);
  int ret = posix_spawn(&pid, "/usr/bin/mpv", &actions, nullptr, argv, environ);
  posix_spawn_file_actions_destroy(&actions);
  if (ret == 0)
    return true;
  else {
    std::cerr << strerror(ret) << "\n";
    return false;
  }
}

void AudioPlayer::stop() {
  if (pid > 0) {
    kill(pid, SIGTERM);
    waitpid(pid, nullptr, 0);
    pid = -1;
  }
}

AudioPlayer::~AudioPlayer() { stop(); }