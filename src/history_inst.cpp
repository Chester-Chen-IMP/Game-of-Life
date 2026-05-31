#include <string>

#include "history.hpp"

// Explicit instantiation for std::string to provide external symbols
// for object files compiled against older headers.
template class History<std::string>;
