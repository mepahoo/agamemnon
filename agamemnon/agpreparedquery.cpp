#include "agpreparedquery.h"
#include <boost/static_assert.hpp>

#ifdef WIN32
  #define htobe64(x) \
	((u_int64_t)( \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x00000000000000ffULL) << 56) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x000000000000ff00ULL) << 40) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x0000000000ff0000ULL) << 24) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x00000000ff000000ULL) <<  8) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x000000ff00000000ULL) >>  8) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x0000ff0000000000ULL) >> 24) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x00ff000000000000ULL) >> 40) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0xff00000000000000ULL) >> 56) ))
			
	#define htobe32(x) \
	((u_int32_t)( \
			(((u_int32_t)(x) & (u_int32_t)0x000000ffUL) << 24) | \
			(((u_int32_t)(x) & (u_int32_t)0x0000ff00UL) <<  8) | \
			(((u_int32_t)(x) & (u_int32_t)0x00ff0000UL) >>  8) | \
			(((u_int32_t)(x) & (u_int32_t)0xff000000UL) >> 24) ))
#else
#include <endian.h>
#endif

namespace teamspeak{
namespace agamemnon{
  
size_t PreparedQuery::fieldCount() const
{
  return m_Data.size();
}

const std::string& PreparedQuery::getFieldName(size_t idx) const
{
  return m_FieldDesc->fieldData.at(idx).fieldName;
}
  
int PreparedQuery::getFieldIndex(const std::string& fieldName) const
{
  const GlobalPreparedQueryCache::FieldDataVector& fd = m_FieldDesc->fieldData;
  for(size_t i=0; i < fd.size(); ++i)
  {
    if (fd[i].fieldName == fieldName) return i;
  }
  return -1;
}

size_t PreparedQuery::getRequiredFieldIndex(const std::string& fieldName) const
{
  const GlobalPreparedQueryCache::FieldDataVector& fd = m_FieldDesc->fieldData;
  for(size_t i=0; i < fd.size(); ++i)
  {
    if (fd[i].fieldName == fieldName) return i;
  }
  throw PreparedQueryException(PreparedQueryException::INDEX);
}

ColumnDataType PreparedQuery::getFieldType(size_t idx) const
{
  return m_FieldDesc->fieldData.at(idx).fieldType;
}

ColumnDataType PreparedQuery::getFieldType(const std::string& fieldName) const
{
  return m_FieldDesc->fieldData.at(getRequiredFieldIndex(fieldName)).fieldType;
}

void PreparedQuery::clear()
{
  for(size_t i=0; i < m_Data.size(); ++i) m_Data[i].clear();
}

void PreparedQuery::clear(size_t idx)
{
  m_Data.at(idx).clear();
}

void PreparedQuery::clear(const std::string& fieldName)
{
  m_Data.at(getRequiredFieldIndex(fieldName)).clear();
}

void PreparedQuery::set(size_t idx, const std::string& value)
{
  m_Data.at(idx) = value;
}

void PreparedQuery::set(const std::string& fieldName, const std::string& value)
{
  m_Data[getRequiredFieldIndex(fieldName)] = value;
}

void PreparedQuery::set(size_t idx, const Bytes& value)
{
  m_Data.at(idx) = value.asString();
}

void PreparedQuery::set(const std::string& fieldName, const Bytes& value)
{
  m_Data[getRequiredFieldIndex(fieldName)] = value.asString();
}

void PreparedQuery::set(size_t idx, const UUID& value)
{
  m_Data.at(idx) = value.asBytesString();
}

void PreparedQuery::set(const std::string& fieldName, const UUID& value)
{
  m_Data[getRequiredFieldIndex(fieldName)] = value.asBytesString();
}

void PreparedQuery::set(size_t idx, int64_t value)
{
  union {
   int64_t i;
   char    c[8];
  } val;
  
  BOOST_STATIC_ASSERT(sizeof(val)==8);
  
  val.i = htobe64(value);
  m_Data.at(idx).assign(val.c, sizeof(val));
}

void PreparedQuery::set(const std::string& fieldName, int64_t value)
{
  set(getRequiredFieldIndex(fieldName), value);
}

void PreparedQuery::set(size_t idx, bool value)
{
  union {
   bool b;
   char c[1];
  } val;
  
  BOOST_STATIC_ASSERT(sizeof(val)==1);
  
  val.b = value;
  m_Data.at(idx).assign(val.c, sizeof(val));
}

void PreparedQuery::set(const std::string& fieldName, bool value)
{
  set(getRequiredFieldIndex(fieldName), value);
}

void PreparedQuery::set(size_t idx, double value)
{
  union {
   int64_t i;
   double  d;
   char    c[8];
  } val;
  
  BOOST_STATIC_ASSERT(sizeof(val)==8);
  
  val.d = value;
  val.i = htobe64(val.i);
  m_Data.at(idx).assign(val.c, sizeof(val));
}

void PreparedQuery::set(const std::string& fieldName, double value)
{
  set(getRequiredFieldIndex(fieldName), value);
}

void PreparedQuery::set(size_t idx, float value)
{
  union {
   int32_t i;
   float   f;
   char    c[4];
  } val;
  
  BOOST_STATIC_ASSERT(sizeof(val)==4);
  
  val.f = value;
  val.i = htobe32(val.i);
  m_Data.at(idx).assign(val.c, sizeof(val));
}

void PreparedQuery::set(const std::string& fieldName, float value)
{
  set(getRequiredFieldIndex(fieldName), value);
}

void PreparedQuery::set(size_t idx, int32_t value)
{
  union {
   int32_t i;
   char    c;
  } val;
  
  BOOST_STATIC_ASSERT(sizeof(val)==4);
  
  val.i = htobe32(value);
  m_Data.at(idx).assign(&val.c, sizeof(val));
}

void PreparedQuery::set(const std::string& fieldName, int32_t value)
{
  set(getRequiredFieldIndex(fieldName), value);
}

void PreparedQuery::set(size_t idx, const boost::posix_time::ptime& value)
{
  union {
   int64_t i;
   char    c[8];
  } val;
  
  BOOST_STATIC_ASSERT(sizeof(val)==8);
  
  boost::posix_time::time_duration td = value - boost::posix_time::from_time_t(0);
  int64_t timeInms = td.total_milliseconds();
  
  val.i = htobe64(timeInms);
  m_Data.at(idx).assign(val.c, sizeof(val));
}

void PreparedQuery::set(const std::string& fieldName, const boost::posix_time::ptime& value)
{
  set(getRequiredFieldIndex(fieldName), value);
}

PreparedQuery::PreparedQuery(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr& fieldDesc)
:m_Data()
,m_FieldDesc(fieldDesc)
{
  m_Data.resize(fieldDesc->fieldData.size());
}
    
} //namespace teamspeak
} //namespace agamemnon
