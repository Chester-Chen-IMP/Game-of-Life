#pragma once
#include <stack>

#include <unordered_set>

template<typename T>
class History {
 private:
  std::stack<T> hist_stk;
  std::unordered_set<T> hist_set;

 public:
  // void push(const T& val) {
  //   hist_stk.push(val);
  //   hist_set.insert(val);
  // }

  // Accept by-value to allow efficient moves for rvalues while keeping
  // a simple, correct implementation for both lvalues and rvalues.
  // Compatibility overload for code compiled against older headers that
  // expected `push(const T&)`. It forwards to the by-value overload.
  // Overloads for lvalues and rvalues to avoid ambiguity.
  void push(const T& val) {
    hist_stk.push(val);
    hist_set.insert(val);
  }

  void push(T&& val) {
    hist_stk.push(std::move(val));
    // Insert a copy of the value now stored on the stack into the set.
    hist_set.insert(hist_stk.top());
  }

  void pop() noexcept {
    if (hist_stk.empty()) return;
    hist_set.erase(hist_stk.top());
    hist_stk.pop();
  }

  bool contains(const T& val) const {
    return hist_set.find(val) != hist_set.end();
  }
};