#include "agcqlqueryresult.h"

#include "../thriftcassandra/cassandra_types.h"

#include <iostream>

namespace teamspeak{
namespace agamemnon{
  
CQLQueryResult::CQLQueryResult() : m_ColDataTypes()
{
  m_Result = new ::org::apache::cassandra::CqlResult();
}

CQLQueryResult::~CQLQueryResult()
{
  delete m_Result;
}

/*
datatypes from cassandra
        comparators.put("ascii", "AsciiType");
        comparators.put("bigint", "LongType");
        comparators.put("blob", "BytesType");
        comparators.put("boolean", "BooleanType");
        comparators.put("counter", "CounterColumnType");
        comparators.put("decimal", "DecimalType");
        comparators.put("double", "DoubleType");
        comparators.put("float", "FloatType");
        comparators.put("int", "Int32Type");
        comparators.put("text", "UTF8Type");
        comparators.put("timestamp", "DateType");
        comparators.put("uuid", "UUIDType");
        comparators.put("varchar", "UTF8Type");
        comparators.put("varint", "IntegerType");
*/

bool CQLQueryResult::parse(ErrorFunction errorFunc)
{
  if (m_Result->type != ::org::apache::cassandra::CqlResultType::ROWS) return true;
  
  if (m_Result->__isset.schema){
    /* lets parse the schema */
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
    /*lets output rows? */
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
  return true;
}
    
} //namespace teamspeak
} //namespace agamemnon