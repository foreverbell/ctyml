#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <map>
#include <utility>
#include <vector>

class Binding { };

class Context final {
 public:
  Context();
  ~Context();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  // Duplicate name is allowed.
  void AddBinding(const std::string& name, Binding* binding);
  void AddName(const std::string& name);
  void DropBindings(size_t n);
  // Picks one fresh alias of <name> that does not appear in <bindings_>.
  std::string PickFreshName(const std::string& name);

  // Returns the smallest index (logical index).
  int ToIndex(const std::string& name);

  const std::pair<std::string, std::unique_ptr<Binding>>& get(int index) const {
    return bindings_.at(bindings_.size() - 1 - index);
  }

  size_t size() const {
    return bindings_.size();
  }

 private:
  // Stores de bruijn index in reverse order.
  std::map<std::string, std::vector<int>> index_map_;
  std::vector<std::pair<std::string, std::unique_ptr<Binding>>> bindings_;

  typedef decltype(index_map_)::iterator bindings_iterator;
  typedef decltype(index_map_)::const_iterator bindings_const_iterator;
};
