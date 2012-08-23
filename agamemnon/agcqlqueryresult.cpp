#include "agcqlqueryresult.h"

#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>

#include "../thriftcassandra/cassandra_types.h"

#include <iostream>

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
  return m_Column.value.empty();
}

std::string CQLQueryResult::CQLColumnValue::asString() const
{
  if (isNull()) return "null";
  switch (m_Cdt) 
  {
    case INT64:
      return boost::lexical_cast<std::string>(intAsInt64());
    case BYTES:
      return "'"+Bytes::bytesStringToHex(m_Column.value)+"'";
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
      return "'"+boost::posix_time::to_iso_extended_string(intAsDateTime())+"'";
    case UUID:
      ::teamspeak::agamemnon::UUID::bytesToString(m_Column.value);
    default:
      return "'"+m_Column.value+"'";
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

bool CQLQueryResult::CQLColumnValue::asBool() const;
double CQLQueryResult::CQLColumnValue::asDouble() const;
float CQLQueryResult::CQLColumnValue::asFloat() const;
int CQLQueryResult::CQLColumnValue::asInt() const;
boost::posix_time::ptime CQLQueryResult::CQLColumnValue::asDateTime() const;
//virtual --                CQLQueryResult::CQLColumnValue::asDecimal() const;
int64_t CQLQueryResult::CQLColumnValue::writeTime() const;
int CQLQueryResult::CQLColumnValue::TTL() const;
bool CQLQueryResult::CQLColumnValue::hasWriteTime() const;
bool  CQLQueryResult::CQLColumnValue::hasTTL() const;
int64_t CQLQueryResult::CQLColumnValue::intAsInt64() const;
bool CQLQueryResult::CQLColumnValue::intAsBool() const
{
  if (m_Column.value.size()==1 && m_Column.value.data[0]==0) return false;
  return true;
}

double                    CQLQueryResult::CQLColumnValue::intAsDouble() const;
float                     CQLQueryResult::CQLColumnValue::intAsFloat() const;
int                       CQLQueryResult::CQLColumnValue::intAsInt() const;
boost::posix_time::ptime  CQLQueryResult::CQLColumnValue::intAsDateTime() const;


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
//const CQLQueryResult::CQLColumnValue CQLQueryResult::get(int rowIdx, int colIdx) const;

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
  
/*
  if (m_Result->__isset.schema){
    std::cout <<"Schema:" << std::endl<<"  default_name_type: "<< m_Result->schema.default_name_type << std::endl<<"  default_value_type: "<<m_Result->schema.default_value_type<<std::endl;
    std::cout <<"  name_types"<<std::endl;
    for(std::map<std::string, std::string>::const_iterator it = m_Result->schema.name_types.begin(); it !=m_Result->schema.name_types.end(); ++it){
      std::cout<<"    "<<it->first<<" , "<<it->second<<std::endl;
    }
    std::cout <<"  value_types"<<std::endl;
    for(std::map<std::string, std::string>::const_iterator it = m_Result->schema.value_types.begin(); it !=m_Result->schema.value_types.end(); ++it){
      std::cout<<"    "<<it->first<<" , "<<it->second<<std::endl;
    }
  }
  
  if (m_Result->__isset.rows){
    std::cout <<"Data:"<<std::endl;
    for (size_t i=0; i < m_Result->rows.size(); ++i){
      std::cout <<"  row "<<i<<": "<<std::endl<<"    key: "<<m_Result->rows[i].key<< std::endl;
      for (size_t j = 0; j < m_Result->rows[i].columns.size(); ++j){
	::org::apache::cassandra::Column& col = m_Result->rows[i].columns[j];
	 std::cout <<" col:"<<j<<" name:"<<col.name;
	 if (col.__isset.value) std::cout<<" value:'"<<col.value<<"'";
	 if (col.__isset.timestamp) std::cout<<" writetime:"<<col.timestamp;
	 if (col.__isset.ttl) std::cout<<" TTL:"<<col.ttl;
	 std::cout <<std::endl;
      }
    }
  }
  
  if (m_Result->__isset.num){
    std::cout << "Num:"<<m_Result->num<<std::endl;
  }
  return true;
  */
}
    
} //namespace teamspeak
} //namespace agamemnon