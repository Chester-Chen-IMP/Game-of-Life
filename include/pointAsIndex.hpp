#pragma once
#include <array>
#include <cstddef>

struct Point {
  int x, y;

  Point() : x(0), y(0) {}
  Point(int X, int Y) : x(X), y(Y) {}
};

template <typename T, size_t N>
bool at(std::array<std::array<T, N>, N>& grid, const Point& p) {
  if (p.x < 0 || p.y < 0 || p.x > N - 1 || p.y > N - 1) {
    return false;
  }
  return grid[p.x][p.y];
}

template <typename T, size_t N>
const bool at(const std::array<std::array<T, N>, N>& grid, const Point& p) {
  if (p.x < 0 || p.y < 0 || p.x > N - 1 || p.y > N - 1) {
    return false;
  }
  return grid[p.x][p.y];
}