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
    
void CassandraConnection::getClusterName(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback){
  m_AgCassandraCobClient->resetBuffers();
  m_AgCassandraCobClient->describe_cluster_name(boost::bind(&CassandraConnection::getClusterName_Done, shared_from_this(), errorFunc, callback));
}

void CassandraConnection::getClusterName_Done(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback){
  std::string result;
  try{
    m_AgCassandraCobClient->checkTransportErrors();
    m_AgCassandraCobClient->recv_describe_cluster_name(result);
  } catch (const std::exception e){
    errorFunc(&e);
    return;
  }
  callback(result);
}
    
} //namespace teamspeak
} //namespace agamemnon
