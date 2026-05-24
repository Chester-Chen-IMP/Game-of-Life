#pragma once
#include <cstdint>
#include <string>
#include <utility>

std::pair<uint32_t, int> decodeUtf8(const std::string& str, size_t pos);

bool isWchar(uint32_t cp);

int getWidth(const std::string& str);