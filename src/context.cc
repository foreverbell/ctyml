#include "context.h"

#include <experimental/string_view>
#include <algorithm>

using std::experimental::string_view;
using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;

namespace {

int to_integer(const string_view& view) {
  if (view.empty()) {
    return -1;
  }
  int ret = 0;
  for (size_t i = 0; i < view.length(); ++i) {
    if (!isdigit(view[i])) {
      return -1;
    }
    ret = ret * 10 + view[i] - '0';
  }
  return ret;
}

// Returns the minimal positive number that doesn't appear in <vec>.
int mex(vector<int> vec) {
  std::sort(vec.begin(), vec.end());
  int expect = 1;
  for (int num : vec) {
    if (num > expect) {
      return expect;
    } else if (num == expect) {
      ++expect;
    }
  }
  return expect;
}

}  // namespace

void Context::AddBinding(const string& name, Binding* binding) {
  const size_t count = size();
  index_map_[name].push_back(count);
  bindings_.push_back({name, unique_ptr<Binding>(binding)});
}

void Context::AddName(const string& name) {
  AddBinding(name, nullptr);
}

void Context::DropBindings(size_t n) {
  assert(n <= size());

  for (size_t i = 0; i < n; ++i) {
    const bindings_iterator iter = index_map_.find(bindings_.back().first);
    iter->second.pop_back();
    if (iter->second.empty()) {
      index_map_.erase(iter);
    }
    bindings_.pop_back();
  }
}

string Context::PickFreshName(const std::string& name) {
  string fresh;
  bindings_const_iterator iter = index_map_.find(name);

  if (iter == index_map_.end()) {
    fresh = name;
  } else {
    vector<int> occupied_slots;
    while (++iter != index_map_.end()) {
      string_view view(iter->first);
      if (view.substr(0, name.length()) == name) {
        if (iter->first.length() >= name.length() + 2 && iter->first[name.length()] == '_') {
          view = view.substr(name.length() + 1);
          int number = to_integer(view);
          if (number != -1) {
            occupied_slots.push_back(number);
          }
        }
      } else {
        break;
      }
    }
    const int free = mex(std::move(occupied_slots));
    fresh = name + "_" + std::to_string(free);
  }

  AddName(fresh);

  return fresh;
}

int Context::ToIndex(const string& name) {
  const bindings_const_iterator iter = index_map_.find(name);
  return iter == index_map_.end() ? -1 : size() - 1 - iter->second.back();
}
