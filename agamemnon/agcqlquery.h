#ifndef AGCQLQUERRY_H
#define AGCQLQUERRY_H

#include <boost/date_time/posix_time/posix_time.hpp>

#include "agamemnon_types.h"

namespace teamspeak{
namespace agamemnon{

class CQLQuery
{
  public:
    
    enum TimeStampAccuracy {AC_MILLISECONDS, AC_MICROSECONDS};
    
    static std::string escapeString(const std::string& s, bool quote = true);
    static std::string booleanToString(bool b, bool quote = true);
    static std::string blobToHexString(const std::string& blob, bool quote = true);
    static std::string blobToHexString(const char* blob, size_t blobLen, bool quote = true);
    static std::string ptimeToString(const boost::posix_time::ptime& t); //always use utc here to prevent mistakes
    static std::string bytesStringToUUID(const std::string& bytes);
    static std::string UUIDToString(const UUID& uuid);
    static std::string BytesToHexString(const Bytes& bytes, bool quote = true);
    static std::string getTimeStamp(TimeStampAccuracy=AC_MICROSECONDS);
};

} //namespace teamspeak
} //namespace agamemnon

#endif // AGCQLQUERRY_H
