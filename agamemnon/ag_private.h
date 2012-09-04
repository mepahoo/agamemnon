#ifndef AGAMEMNON_PRIVATE_H
#define AGAMEMNON_PRIVATE_H

#include "agamemnon_types.h"

namespace teamspeak{
namespace agamemnon{
  
  ColumnDataType columnTypeStringToEnum(const std::string& columnTypeString);
  
  struct StringPointerCompare{
    inline bool operator()(const std::string* a, const std::string* b) const{
      return *a < *b;
    }
  };

} //namespace teamspeak
} //namespace agamemnon

#endif // AGAMEMNON_PRIVATE_H
