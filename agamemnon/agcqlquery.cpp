#include "agcqlquery.h"

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/algorithm/string/find.hpp>
#include <sstream>

namespace teamspeak{
namespace agamemnon{

/*static*/
std::string CQLQuery::escapeString(const std::string& s, bool quote ){
  
  if (!quote) return boost::algorithm::replace_all_copy(s,"'","''");
  
  std::stringstream ss;
  if (quote) ss <<"'";
  ss << boost::algorithm::replace_all_copy(s,"'","''");
  if (quote) ss <<"'";
  return ss.str();
}

/*static*/
std::string CQLQuery::blobToHexString(const std::string& blob, bool quote )
{
  if (!quote) return Bytes::bytesStringToHex(blob);

  std::stringstream ss;
  if (quote) ss <<"'";
  ss << Bytes::bytesStringToHex(blob);
  if (quote) ss <<"'";
  return ss.str();
}

/*static*/
std::string CQLQuery::blobToHexString(const char* blob, size_t blobLen, bool quote )
{
  if (!quote) return Bytes::bytesStringToHex(blob, blobLen);

  std::stringstream ss;
  if (quote) ss <<"'";
  ss << Bytes::bytesStringToHex(blob, blobLen);
  if (quote) ss <<"'";
  return ss.str();
}

/*static*/
std::string CQLQuery::ptimeToString(const boost::posix_time::ptime& t)
{
  boost::posix_time::time_duration td = t - boost::posix_time::from_time_t(0);
  return boost::lexical_cast<std::string>(td.total_milliseconds());
}

/*static*/
std::string CQLQuery::bytesStringToUUID(const std::string& bytes)
{
  return UUID::bytesToString(bytes);
}

/*static*/
std::string CQLQuery::UUIDToString(const UUID& uuid)
{
  return uuid.asString();
}

/*static*/
std::string CQLQuery::BytesToHexString(const Bytes& bytes, bool quote )
{
  if (!quote) return bytes.asHexString();

  std::stringstream ss;
  if (quote) ss <<"'";
  ss << bytes.asHexString();
  if (quote) ss <<"'";
  return ss.str();
}


/*static*/
std::string CQLQuery::getTimeStamp(TimeStampAccuracy accuracy)
{
  boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - boost::posix_time::from_time_t(0);
  if (accuracy == AC_MILLISECONDS) return boost::lexical_cast<std::string>(td.total_milliseconds());
  return boost::lexical_cast<std::string>(td.total_microseconds());
}

} //namespace teamspeak
} //namespace agamemnon