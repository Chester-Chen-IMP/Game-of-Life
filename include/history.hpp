#pragma once
#include <stack>

#include <unordered_set>

template<typename T>
class History {
 private:
  std::stack<T> hist_stk;
  std::unordered_set<T> hist_set;

 public:
  void push(const T& val);

  void pop();

  bool contains(const T& val);
};