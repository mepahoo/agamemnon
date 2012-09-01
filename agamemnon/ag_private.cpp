#include "ag_private.h"

#include <vector>
#include <boost/assign.hpp>

namespace teamspeak{
namespace agamemnon{
  
static std::map<std::string, ColumnDataType> s_DataStringToType = 
    boost::assign::map_list_of 
     ("AsciiType", CDT_ASCII)
     ("LongType", CDT_INT64)
     ("BytesType", CDT_BYTES)
     ("BooleanType", CDT_BOOLEAN)
     ("CounterColumnType", CDT_COUNTER)
     ("DecimalType", CDT_DECIMAL)
     ("DoubleType", CDT_DOUBLE)
     ("FloatType", CDT_FLOAT)
     ("Int32Type", CDT_INT32)
     ("UTF8Type", CDT_UTF8)
     ("DateType", CDT_DATE)
     ("UUIDType", CDT_UUID)
     ("IntegerType", CDT_INTEGER);
     
ColumnDataType columnTypeStringToEnum(const std::string& columnTypeString)
{
    std::map<std::string, ColumnDataType> ::const_iterator it = s_DataStringToType.find(columnTypeString);
    if (it == s_DataStringToType.end()){
      return CDT_UNKNOWN;
    } else {
      return it->second;
    }
}

} //namespace teamspeak
} //namespace agamemnon