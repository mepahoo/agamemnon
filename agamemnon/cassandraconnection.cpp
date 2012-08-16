#include "cassandraconnection.h"

#include "agcassandracobclient.h"
#include <boost/bind.hpp>

namespace teamspeak{
namespace agamemnon{

CassandraConnection::CassandraConnection(AgCassandraCobClient* con)
:boost::enable_shared_from_this<CassandraConnection>()
, m_AgCassandraCobClient(con)
, m_NeedToCloseWhenDone(false)
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
    
void CassandraConnection::getClusterName(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback){
  m_AgCassandraCobClient->resetBuffers();
  m_AgCassandraCobClient->describe_cluster_name(boost::bind(&CassandraConnection::getClusterName_Done, shared_from_this(), errorFunc, callback));
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

void CassandraConnection::getClusterName_Done(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback){
  std::string result;
  if (!m_AgCassandraCobClient->checkTransportErrors(errorFunc)) return;
  if (!m_AgCassandraCobClient->recv_describe_cluster_name(errorFunc, result)) return;
  callback(result);
}
    
} //namespace teamspeak
} //namespace agamemnon
