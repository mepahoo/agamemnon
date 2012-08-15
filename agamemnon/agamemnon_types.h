#ifndef AGAMEMNON_TYPES_H
#define AGAMEMNON_TYPES_H

#include <boost/function.hpp>

namespace teamspeak{
namespace agamemnon{

  typedef boost::function<void(const std::exception*)> ErrorFunction;

} //namespace teamspeak
} //namespace agamemnon

#endif // AGAMEMNON_TYPES_H
