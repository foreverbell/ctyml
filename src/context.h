#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <map>
#include <utility>
#include <vector>

#include "ast.h"

class Binding {
 public:
  Binding(const Term* term, const TermType* type) : term_(term), type_(type) { }

  const Term* term() const { return term_.get(); }
  const TermType* type() const { return type_.get(); }

 private:
  const std::unique_ptr<const Term> term_;  // nullable.
  const std::unique_ptr<const TermType> type_;  // nullable.
};

class Context final {
 public:
  Context() = default;
  ~Context() = default;

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  // Duplicate name is allowed.
  void AddBinding(const std::string& name, Binding* binding);
  void AddName(const std::string& name);
  void DropBindings(size_t n);
  void DropBindingsTo(size_t new_size) { DropBindings(size() - new_size); }

  // Picks one fresh alias of <name> that does not appear in <bindings_>.
  std::string PickFreshName(const std::string& name);

  // Returns the smallest index (logical index), "-1" if not found.
  int ToIndex(const std::string& name) const;

  const std::pair<std::string, std::unique_ptr<Binding>>& get(int index) const {
    return bindings_.at(bindings_.size() - 1 - index);
  }

  size_t size() const {
    return bindings_.size();
  }

 private:
  // Stores deBruijn index in reverse order.
  std::map<std::string, std::vector<int>> index_map_;
  std::vector<std::pair<std::string, std::unique_ptr<Binding>>> bindings_;

  typedef decltype(index_map_)::iterator bindings_iterator;
  typedef decltype(index_map_)::const_iterator bindings_const_iterator;
};
