#include "agcqlquery.h"

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <sstream>

namespace teamspeak{
namespace agamemnon{

/*static*/
std::string CQLQuery::escapeString(const std::string& s, bool quote )
{
  std::stringstream ss;
  if (quote) ss <<"'";
  
  for(size_t i=0; i < s.size(); ++i)
  {
    char c = s[i];
    if (c < 0)
    {
      //get bits
      unsigned char uc = static_cast<unsigned char>(c);
      unsigned char m,mc;
      int j;
      for (j=5; j > 1; --j)
      {
	mc = 254 << j;
	m = 255 << j;
	if ((uc & m)==mc) break;
      }
      if (j > 0)
      {
	int skip = 7-j;
	for (int k=0; k < skip;++k) ss << s[i+k];
	i+= (skip-1);
	continue;
      }
    }
    if (c == '\'') {
      ss << "\'\'";
    } else {
      ss << c;
    }
  }
 
  if (quote) ss <<"'";
  return ss.str();
}

/*static*/
std::string CQLQuery::booleanToString(bool b, bool quote)
{
  if (!quote) return b ? "true" : "false";
  return b ? "'true'" : "'false'";
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
  boost::posix_time::time_duration td = boost::posix_time::microsec_clock::universal_time() - boost::posix_time::from_time_t(0);
  if (accuracy == AC_MILLISECONDS) return boost::lexical_cast<std::string>(td.total_milliseconds());
  return boost::lexical_cast<std::string>(td.total_microseconds());
}

} //namespace teamspeak
} //namespace agamemnon