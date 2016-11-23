#include <exception>
#include <string>

#include "location.h"

class type_exception : public std::exception {
 public:
  type_exception(Location location, std::string error)
    : location_(location), error_(std::move(error)) { }

  virtual ~type_exception() throw () { }

  virtual const char* what() const throw () {
    if (!is_msg_set_) {
      is_msg_set_ = true;
      msg_.append("type error: ");
      msg_.append(error_);
      msg_.append("\n");
    }
    return msg_.c_str();
  }

  Location location() const { return location_; }

 private:
  const Location location_;
  std::string error_;

  mutable std::string msg_;
  mutable bool is_msg_set_ = false;
};
