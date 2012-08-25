#include "agcqlqueryresult.h"

#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "../thriftcassandra/cassandra_types.h"

#include <iostream>

#ifdef WIN32
  #define be64toh(x) \
	((u_int64_t)( \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x00000000000000ffULL) << 56) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x000000000000ff00ULL) << 40) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x0000000000ff0000ULL) << 24) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x00000000ff000000ULL) <<  8) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x000000ff00000000ULL) >>  8) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x0000ff0000000000ULL) >> 24) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0x00ff000000000000ULL) >> 40) | \
			(u_int64_t)(((u_int64_t)(x) & (u_int64_t)0xff00000000000000ULL) >> 56) ))
			
	#define be32toh(x) \
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

std::map<std::string, CQLQueryResult::ColumnDataType> CQLQueryResult::s_DataStringToType = 
    boost::assign::map_list_of 
     ("AsciiType", CQLQueryResult::ASCII)
     ("LongType", CQLQueryResult::INT64)
     ("BytesType", CQLQueryResult::BYTES)
     ("BooleanType", CQLQueryResult::BOOLEAN)
     ("CounterColumnType", CQLQueryResult::COUNTER)
     ("DecimalType", CQLQueryResult::DECIMAL)
     ("DoubleType", CQLQueryResult::DOUBLE)
     ("FloatType", CQLQueryResult::FLOAT)
     ("Int32Type", CQLQueryResult::INT32)
     ("UTF8Type", CQLQueryResult::UTF8)
     ("DateType", CQLQueryResult::DATE)
     ("UUIDType", CQLQueryResult::UUID)
     ("IntegerType", CQLQueryResult::INTEGER);
     
CQLQueryResult::CQLColumnValue::CQLColumnValue(const ::org::apache::cassandra::Column& column, CQLQueryResult::ColumnDataType cdt)
: m_Column(column)
, m_Cdt(cdt)
{
}

bool CQLQueryResult::CQLColumnValue::isNull() const
{
  if (!m_Column.value.empty()) return false;
  return !m_Column.__isset.timestamp;
}

std::string CQLQueryResult::CQLColumnValue::asString() const
{
  if (isNull()) return "null";
  switch (m_Cdt) 
  {
    case INT64:
      return boost::lexical_cast<std::string>(intAsInt64());
    case BYTES:
      return Bytes::bytesStringToHex(m_Column.value);
    case BOOLEAN:
      return intAsBool()? "true" : "false";
    case COUNTER:
      return boost::lexical_cast<std::string>(intAsInt64());
    case DOUBLE:
      return boost::lexical_cast<std::string>(intAsDouble());
    case FLOAT:
      return boost::lexical_cast<std::string>(intAsFloat());
    case INT32:
      return boost::lexical_cast<std::string>(intAsInt());
    case DATE:
      return boost::posix_time::to_iso_extended_string(intAsDateTime(TA_LOCALTIME));
    case UUID:
      return ::teamspeak::agamemnon::UUID::bytesToString(m_Column.value);
    default:
      return m_Column.value;
  }
}

const std::string& CQLQueryResult::CQLColumnValue::asBytes() const
{
  if (m_Cdt != CQLQueryResult::BYTES) throw ConversionException();
  return m_Column.value;
}

int64_t  CQLQueryResult::CQLColumnValue::asInt64() const
{
  switch (m_Cdt) 
  {
    case INT64: 
    case COUNTER:
      return intAsInt64();
    case DOUBLE:
      return intAsDouble();
    case FLOAT:
      return intAsFloat();
    case INT32:
      return intAsInt();
    default:
      throw ConversionException();
  }
}

bool CQLQueryResult::CQLColumnValue::asBool() const
{
  if (m_Cdt!=BOOLEAN) throw ConversionException();
  return intAsBool();
}

double CQLQueryResult::CQLColumnValue::asDouble() const
{
   switch (m_Cdt) 
  {
    case INT64: 
    case COUNTER:
      return intAsInt64();
    case DOUBLE:
      return intAsDouble();
    case FLOAT:
      return intAsFloat();
    case INT32:
      return intAsInt();
    default:
      throw ConversionException();
  }
}

float CQLQueryResult::CQLColumnValue::asFloat() const
{
   switch (m_Cdt) 
  {
    case INT64: 
    case COUNTER:
      return intAsInt64();
    case DOUBLE:
      return intAsDouble();
    case FLOAT:
      return intAsFloat();
    case INT32:
      return intAsInt();
    default:
      throw ConversionException();
  }
}

int CQLQueryResult::CQLColumnValue::asInt() const
{
   switch (m_Cdt) 
  {
    case INT64: 
    case COUNTER:
      return intAsInt64();
    case DOUBLE:
      return intAsDouble();
    case FLOAT:
      return intAsFloat();
    case INT32:
      return intAsInt();
    default:
      throw ConversionException();
  }
}

boost::posix_time::ptime CQLQueryResult::CQLColumnValue::asDateTime(TimeAdjust ta) const
{
  if (m_Cdt != DATE) throw ConversionException();
  return intAsDateTime(ta);
}

//virtual --                CQLQueryResult::CQLColumnValue::asDecimal() const;

int64_t CQLQueryResult::CQLColumnValue::writeTime() const
{
  return m_Column.timestamp;
}

int CQLQueryResult::CQLColumnValue::TTL() const
{
  return m_Column.ttl;
}

bool CQLQueryResult::CQLColumnValue::hasWriteTime() const
{
  return m_Column.__isset.timestamp;
}

bool  CQLQueryResult::CQLColumnValue::hasTTL() const
{
  return m_Column.__isset.ttl;
}

int64_t CQLQueryResult::CQLColumnValue::intAsInt64() const
{
  BOOST_ASSERT(m_Column.value.size()==8);
  const uint64_t* data = reinterpret_cast<const uint64_t*>(m_Column.value.data());
  return static_cast<int64_t>(be64toh(*data));
}

bool CQLQueryResult::CQLColumnValue::intAsBool() const
{
  if ((m_Column.value.size()==1) && (m_Column.value.data()[0]==0)) return false;
  return true;
}

double CQLQueryResult::CQLColumnValue::intAsDouble() const
{
  union {
    int64_t i;
    double  d;
  } v;
  BOOST_ASSERT(m_Column.value.size()==8);
  v.i = static_cast<int64_t>(be64toh(*reinterpret_cast<const uint64_t*>(m_Column.value.data())));
  return v.d;
}

float CQLQueryResult::CQLColumnValue::intAsFloat() const
{
  union {
    int32_t i;
    float   f;
  } v;
  BOOST_ASSERT(m_Column.value.size()==4);
  v.i = static_cast<int32_t>(be32toh(*reinterpret_cast<const uint32_t*>(m_Column.value.data())));
  return v.f;
}

int CQLQueryResult::CQLColumnValue::intAsInt() const
{
  BOOST_ASSERT(m_Column.value.size()==4);
  return static_cast<int32_t>(be32toh(*reinterpret_cast<const uint32_t*>(m_Column.value.data())));
  
}

boost::posix_time::ptime  CQLQueryResult::CQLColumnValue::intAsDateTime(TimeAdjust ta) const
{
  int64_t millisecsSinceEpoch = intAsInt64();
  boost::posix_time::ptime time =  boost::posix_time::from_time_t(millisecsSinceEpoch/1000);
  if (ta == TA_LOCALTIME) {
    typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> local_adj;
    time =  local_adj::utc_to_local(time);
  }
  time += boost::posix_time::milliseconds(millisecsSinceEpoch % 1000);

  return time;
}


CQLQueryResult::CQLQueryResult() : m_ColDataTypes()
{
  m_Result = new ::org::apache::cassandra::CqlResult();
}

CQLQueryResult::~CQLQueryResult()
{
  delete m_Result;
}

bool CQLQueryResult::wasVoidReturn() const
{
  return m_Result->type == ::org::apache::cassandra::CqlResultType::VOID;
}

bool CQLQueryResult::wasIntReturn() const
{
  return m_Result->type == ::org::apache::cassandra::CqlResultType::INT;
}

bool CQLQueryResult::wasRowsReturn() const
{
  return m_Result->type == ::org::apache::cassandra::CqlResultType::ROWS;
}

bool CQLQueryResult::hasColumns() const
{
  return m_Result->__isset.schema && m_Result->__isset.rows;
}

size_t CQLQueryResult::getColumnCount() const
{
  if (getRowCount() == 0) return 0;
  return m_Result->rows[0].columns.size();
}

size_t CQLQueryResult::getRowCount() const
{
  if (!hasColumns()) return 0;
  return m_Result->rows.size();
}

CQLQueryResult::ColumnDataType CQLQueryResult::getColumnType(size_t colIdx) const
{
  if (colIdx >= m_ColDataTypes.size()) throw IndexOutOfRangeException();
  
  return m_ColDataTypes[colIdx];
}

std::string& CQLQueryResult::getColumnName(size_t colIdx) const
{
   if (colIdx >= m_ColDataTypes.size()) throw IndexOutOfRangeException();
   
   return m_Result->rows[0].columns[colIdx].name;
}

size_t CQLQueryResult::indexOfColumnName(const std::string& colName) const
{
  if (m_ColDataTypes.empty()) return npos;
  for(size_t i=0; i < m_Result->rows[0].columns.size(); ++i)
  {
    if (m_Result->rows[0].columns[i].name == colName) return i;
  }
  return npos;
}

const CQLQueryResult::CQLColumnValue CQLQueryResult::get(size_t rowIdx, size_t colIdx) const
{
  return CQLQueryResult::CQLColumnValue( m_Result->rows[rowIdx].columns[colIdx], m_ColDataTypes[colIdx]);
}

bool CQLQueryResult::parse(ErrorFunction errorFunc)
{
  if (m_Result->type != ::org::apache::cassandra::CqlResultType::ROWS) return true;

  if (!(m_Result->__isset.schema && m_Result->__isset.rows)) throw ParseException();

  if (m_Result->rows.empty()) return true;
  
  m_ColDataTypes.reserve(m_Result->rows[0].columns.size());
  
  for(size_t i=0; i < m_Result->rows[0].columns.size(); ++i)
  {
    const std::string& colName = m_Result->rows[0].columns[i].name;
    const std::string& colType = m_Result->schema.value_types[colName];
    std::map<std::string, CQLQueryResult::ColumnDataType> ::const_iterator it = s_DataStringToType.find(colType);
    if (it == s_DataStringToType.end()){
      m_ColDataTypes.push_back(UNKNOWN);
    } else {
      m_ColDataTypes.push_back(it->second);
    }
  }
  
  return true;
}
    
} //namespace teamspeak
} //namespace agamemnon