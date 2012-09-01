#include "cassandraconnection.h"

#include "agcassandracobclient.h"
#include <boost/bind.hpp>
#include <iostream>

#ifdef HAS_ZLIB_H
#include <zlib.h>
#endif

namespace teamspeak{
namespace agamemnon{

CassandraConnection::CassandraConnection(AgCassandraCobClient* con)
:boost::enable_shared_from_this<CassandraConnection>()
, m_AgCassandraCobClient(con)
, m_NeedToCloseWhenDone(false)
, m_Timer()
, m_PreparedQueryMap()
{
}

CassandraConnection::~CassandraConnection()
{
}

void CassandraConnection::setKeyspace(const std::string& keyspace, ErrorFunction errorFunc, boost::function<void()> callback)
{
  std::string cqlCommand = "USE "+keyspace+";";
  m_AgCassandraCobClient->resetBuffers();
  m_AgCassandraCobClient->execute_cql_query(boost::bind(&CassandraConnection::setKeyspace_done, shared_from_this(), errorFunc, callback),
					    cqlCommand,
					    ::org::apache::cassandra::Compression::NONE);
  
}

void CassandraConnection::setCQLVersion(const std::string& version, ErrorFunction errorFunc, boost::function<void()> callback)
{
  m_AgCassandraCobClient->resetBuffers();
  m_AgCassandraCobClient->set_cql_version (boost::bind(&CassandraConnection::setCQLVersion_done, shared_from_this(), errorFunc, callback),
					    version);
}
    
void CassandraConnection::getClusterName(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback){
  m_AgCassandraCobClient->resetBuffers();
  m_Timer.reset();
  m_AgCassandraCobClient->describe_cluster_name(boost::bind(&CassandraConnection::getClusterName_done, shared_from_this(), errorFunc, callback));
}

void CassandraConnection::executeCQL(const std::string& cql, bool doCompress, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback)
{
  m_AgCassandraCobClient->resetBuffers();
  m_Timer.reset();
  
#ifdef HAS_ZLIB_H
  if (doCompress && cql.size() > 128) 
  {
    std::string buffer(cql.size(), 0);
    
    uLongf destLen = buffer.size();
    int compressRes = compress((Bytef *)buffer.data(), &destLen, (const Bytef *)cql.data(), cql.size());
    if (compressRes == Z_OK)
    {
      buffer.resize(destLen);
      m_AgCassandraCobClient->execute_cql_query(boost::bind(&CassandraConnection::executeCQL_done, shared_from_this(), errorFunc, callback),
						buffer,
						::org::apache::cassandra::Compression::GZIP);
      return;
    }
  }
#endif // HAS_ZLIB_H
  {
    m_AgCassandraCobClient->execute_cql_query(boost::bind(&CassandraConnection::executeCQL_done, shared_from_this(), errorFunc, callback),
					      cql,
					      ::org::apache::cassandra::Compression::NONE);
  }
}

void CassandraConnection::prepareQuery(const std::string& cql, bool doCompress, ErrorFunction errorFunc, boost::function<void(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr)> callback)
{
  m_AgCassandraCobClient->resetBuffers();
  m_Timer.reset();

#ifdef HAS_ZLIB_H
  if (doCompress && cql.size() > 128) 
  {
    std::string buffer(cql.size(), 0);
    
    uLongf destLen = buffer.size();
    int compressRes = compress((Bytef *)buffer.data(), &destLen, (const Bytef *)cql.data(), cql.size());
    if (compressRes == Z_OK)
    {
      buffer.resize(destLen);
      m_AgCassandraCobClient->prepare_cql_query(boost::bind(&CassandraConnection::prepareQuery_done, shared_from_this(), errorFunc, cql, callback),
						buffer,
						::org::apache::cassandra::Compression::GZIP);
      return;
    }
  }
#endif // HAS_ZLIB_H
  {
    m_AgCassandraCobClient->prepare_cql_query(boost::bind(&CassandraConnection::prepareQuery_done, shared_from_this(), errorFunc, cql, callback),
					      cql,
					      ::org::apache::cassandra::Compression::NONE);
  }
}

void CassandraConnection::executePreparedQuery(const PreparedQuery::Ptr & fieldData, bool doCompress, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback)
{
  PreparedQueryMap::const_iterator it = m_PreparedQueryMap.find(fieldData->m_FieldDesc);
  
  if (it == m_PreparedQueryMap.end()){
    //not prepared on this connection
    boost::function<void(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr)> callback2 = 
      boost::bind(&CassandraConnection::executePreparedQuery, shared_from_this(), fieldData, doCompress, errorFunc, callback);
      
    prepareQuery(fieldData->m_FieldDesc->query, doCompress, errorFunc, callback2);  
  } else {
    m_AgCassandraCobClient->resetBuffers();
    m_Timer.reset();
  
    m_AgCassandraCobClient->execute_prepared_cql_query(boost::bind(&CassandraConnection::executePreparedQuery_done, shared_from_this(), errorFunc, callback), it->second, fieldData->m_Data);
  }
}


void CassandraConnection::setNeedToCloseWhenDone()
{
  m_NeedToCloseWhenDone = true;
}

void CassandraConnection::setKeyspace_done(ErrorFunction errorFunc, boost::function<void()> callback)
{
  ::org::apache::cassandra::CqlResult result;
  if (!m_AgCassandraCobClient->checkTransportErrors(errorFunc)) return;
  if (!m_AgCassandraCobClient->recv_execute_cql_query(errorFunc, result)) return;
  callback();
}

void CassandraConnection::setCQLVersion_done(ErrorFunction errorFunc, boost::function<void()> callback)
{
  if (!m_AgCassandraCobClient->checkTransportErrors(errorFunc)) return;
  if (!m_AgCassandraCobClient->recv_set_cql_version (errorFunc)) return;
  callback();
}

void CassandraConnection::getClusterName_done(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback){
  std::string result;
  
  std::cout <<"GetCluserName time: "<< m_Timer.msElapsed()<<"ms"<<std::endl;
  
  if (!m_AgCassandraCobClient->checkTransportErrors(errorFunc)) return;
  if (!m_AgCassandraCobClient->recv_describe_cluster_name(errorFunc, result)) return;
  callback(result);
}

void CassandraConnection::executeCQL_done(ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback)
{
  CQLQueryResult::Ptr result(new CQLQueryResult());
  if (!m_AgCassandraCobClient->checkTransportErrors(errorFunc)) return;
  if (!m_AgCassandraCobClient->recv_execute_cql_query(errorFunc, *result->m_Result)) return;
  if (!result->parse(errorFunc)) return;
  callback(result);
}

void CassandraConnection::prepareQuery_done(ErrorFunction errorFunc, const std::string& cql, boost::function<void(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr)> callback)
{
  org::apache::cassandra::CqlPreparedResult cqlPreparedResult;
  if (!m_AgCassandraCobClient->checkTransportErrors(errorFunc)) return;
  if (!m_AgCassandraCobClient->recv_prepare_cql_query(errorFunc, cqlPreparedResult)) return;
  
  GlobalPreparedQueryCache::PreparedQueryInfo::Ptr res = GlobalPreparedQueryCache::getInstance()->storePreparedQuery(cql, &cqlPreparedResult);
  m_PreparedQueryMap[res] = cqlPreparedResult.itemId;
  callback(res);
}

void CassandraConnection::executePreparedQuery_done(ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback)
{
  CQLQueryResult::Ptr result(new CQLQueryResult());
  if (!m_AgCassandraCobClient->checkTransportErrors(errorFunc)) return;
  if (!m_AgCassandraCobClient->recv_execute_prepared_cql_query(errorFunc, *result->m_Result)) return;
  if (!result->parse(errorFunc)) return;
  callback(result);
}


} //namespace teamspeak
} //namespace agamemnon
