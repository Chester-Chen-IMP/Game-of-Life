#include "history.hpp"

template<typename T>
void History<T>::push(const T& val) {
  hist_stk.push(val);
  hist_set.insert(val);
}

template<typename T>
void History<T>::pop() {
  if (!hist_stk.empty()) {
    hist_set.erase(hist_stk.top());
    hist_stk.pop();
  }
}

template<typename T>
bool History<T>::contains(const T& val) {
  return hist_set.find(val) != hist_set.end();
}