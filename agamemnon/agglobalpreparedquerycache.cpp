#include "agglobalpreparedquerycache.h"

#include "../thriftcassandra/cassandra_types.h"
#include <boost/foreach.hpp>

namespace teamspeak{
namespace agamemnon{

GlobalPreparedQueryCache::FieldData::FieldData(ColumnDataType _fieldType, const std::string& _fieldName)
: fieldType(_fieldType)
, fieldName(_fieldName)
{
}
    

void intrusive_ptr_add_ref(GlobalPreparedQueryCache::PreparedQueryInfo const* s)
{
  ++s->ref_count;
}

void intrusive_ptr_release(GlobalPreparedQueryCache::PreparedQueryInfo const* s)
{
  if ((--(s->ref_count)) == 0) {
    if (GlobalPreparedQueryCache::getInstance()->remove(s)) delete s;
  }
}

GlobalPreparedQueryCache::PreparedQueryInfo::PreparedQueryInfo(const std::string& _query, const GlobalPreparedQueryCache::FieldDataVector& _fieldData)
:query(_query)
,fieldData(_fieldData)
,ref_count(0)
{
}

GlobalPreparedQueryCache::PreparedQueryInfo::Ptr GlobalPreparedQueryCache::getPreparedQueryInfo(const std::string& query)
{
  boost::shared_lock<boost::shared_mutex> lock(m_Mutex);
  
  PreparedQueryInfoMap::const_iterator it = m_PreparedQueryInfoMap.find(&query);
  if (it == m_PreparedQueryInfoMap.end()) return GlobalPreparedQueryCache::PreparedQueryInfo::Ptr(0);
  return GlobalPreparedQueryCache::PreparedQueryInfo::Ptr(it->second);
}

GlobalPreparedQueryCache::PreparedQueryInfo::Ptr GlobalPreparedQueryCache::storePreparedQuery(const std::string& query, const org::apache::cassandra::CqlPreparedResult* cqlPreparedResult)
{
  FieldDataVector fieldVectorData = cqlPreparedResultToFieldDataVector(cqlPreparedResult);
  
  boost::upgrade_lock<boost::shared_mutex> readLock(m_Mutex);
  PreparedQueryInfoMap::const_iterator it = m_PreparedQueryInfoMap.find(&query);
  if (it != m_PreparedQueryInfoMap.end()) 
  {
    if (isSame(fieldVectorData, it->second->fieldData)) return GlobalPreparedQueryCache::PreparedQueryInfo::Ptr(it->second);
    throw GlobalPreparedQueryCacheException();
  }
  
  boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);
  //search again since something can change while waiting for write lock
  PreparedQueryInfoMap::iterator writeIt = m_PreparedQueryInfoMap.lower_bound(&query);

  if (writeIt  != m_PreparedQueryInfoMap.end()) 
  {
    if (isSame(fieldVectorData, writeIt->second->fieldData)) return GlobalPreparedQueryCache::PreparedQueryInfo::Ptr(writeIt ->second);
    throw GlobalPreparedQueryCacheException();
  }
  
  PreparedQueryInfo* data = new PreparedQueryInfo(query, fieldVectorData);
  m_PreparedQueryInfoMap.insert(writeIt, PreparedQueryInfoMap::value_type(&data->query, data));
  return GlobalPreparedQueryCache::PreparedQueryInfo::Ptr(data);
}

GlobalPreparedQueryCache::GlobalPreparedQueryCache()
:m_PreparedQueryInfoMap()
,m_Mutex()
{
}

GlobalPreparedQueryCache::~GlobalPreparedQueryCache()
{
//should always balance, but just in case
  BOOST_FOREACH( PreparedQueryInfoMap::value_type& data, m_PreparedQueryInfoMap)
  {
    delete data.second;
  }
  m_PreparedQueryInfoMap.clear();
}
    
bool GlobalPreparedQueryCache::remove(PreparedQueryInfo const* s)
{
  boost::unique_lock<boost::shared_mutex> writeLock(m_Mutex);
  PreparedQueryInfoMap::iterator it = m_PreparedQueryInfoMap.find(&s->query);
  
  if (it == m_PreparedQueryInfoMap.end()) return false;
  if (it->second !=s) return false;
  if (s->ref_count!=0)  return false;

  m_PreparedQueryInfoMap.erase(it);
  return true;
}
    
GlobalPreparedQueryCache::FieldDataVector GlobalPreparedQueryCache::cqlPreparedResultToFieldDataVector(const org::apache::cassandra::CqlPreparedResult* cqlPreparedResult) const
{
  if (!(cqlPreparedResult->__isset.variable_names && cqlPreparedResult->__isset.variable_types)) throw GlobalPreparedQueryCacheException();

  FieldDataVector result;
  for(int i=0; i < cqlPreparedResult->count; ++i)
  {
    result.push_back(FieldData(columnTypeStringToEnum(cqlPreparedResult->variable_types.at(i)), cqlPreparedResult->variable_names.at(i)));
  }
  
  return result;  
}

bool GlobalPreparedQueryCache::isSame(const GlobalPreparedQueryCache::FieldDataVector& a, const GlobalPreparedQueryCache::FieldDataVector& b) const
{
  if (a.size() != b.size()) return false;
  for(size_t i=0; i < a.size(); ++i)
  {
    if (a[i] != b[i]) return false;
    
  }
  return true;
}
        
GlobalPreparedQueryCache GlobalPreparedQueryCache::instance;

} //namespace teamspeak
} //namespace agamemnon