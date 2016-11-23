#include <exception>
#include <string>
#include <vector>

#include "location.h"

class ast_exception : public std::exception {
 public:
  ast_exception(Location location, std::string cfg, std::string error)
    : location_(location), error_(std::move(error)) {
    cfgs_.push_back(std::move(cfg));
  }

  ast_exception(ast_exception&& e, std::string cfg)
    : cfgs_(std::move(e.cfgs_)), location_(e.location_), error_(e.error_) {
    cfgs_.push_back(std::move(cfg));
  }

  virtual ~ast_exception() throw () { }

  virtual const char* what() const throw () {
    if (!is_msg_set_) {
      is_msg_set_ = true;
      msg_.append("ast error: ");
      msg_.append(error_);
      msg_.append("\n");
      msg_.append("grammar stack:\n");
      for (const std::string& cfg : cfgs_) {
        msg_.append("  " + cfg);
        msg_.append("\n");
      }
    }
    return msg_.c_str();
  }

  const std::vector<std::string>& cfg() const { return cfgs_; }
  Location location() const { return location_; }

 private:
  // Context free grammars, from bottom to up.
  std::vector<std::string> cfgs_;
  const Location location_;
  std::string error_;

  mutable std::string msg_;
  mutable bool is_msg_set_ = false;
};
