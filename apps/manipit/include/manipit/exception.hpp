#ifndef __MANIPIT_EXCEPTION_HPP
#define __MANIPIT_EXCEPTION_HPP

#include <sstream>

namespace manipit
{

  class Exception
  {
  public:
    explicit Exception(const std::string& src, const std::string& msg) throw()
      : src_(src), msg_(msg) {}

    const char* what() const throw()
    {
      std::stringstream msg;

      msg << "manipit::Exception was thrown." << std::endl
          << "  src : " << src_ << std::endl
          << "  msg : " << msg_;

      return msg.str().c_str();
    }

  private:    
    std::string src_;
    std::string msg_;
  };
}

#endif /* __MANIPIT_EXCEPTION_HPP */
