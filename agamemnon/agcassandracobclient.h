#ifndef AGCASSANDRACOBCLIENT_H
#define AGCASSANDRACOBCLIENT_H

#include "../thriftcassandra/Cassandra.h"
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace teamspeak{
namespace agamemnon{
  
  class AgCassandraCobClient : public ::org::apache::cassandra::CassandraCobClient{
    public:
      typedef boost::function<void(const std::exception*)> ExErrorFunction;
      AgCassandraCobClient(boost::shared_ptr< ::apache::thrift::async::TAsyncChannel> channel, ::apache::thrift::protocol::TProtocolFactory* protocolFactory)
      : CassandraCobClient(channel, protocolFactory)
      {
      }
      
      void resetBuffers();
      bool checkTransportErrors(ExErrorFunction errorFunc);

      bool recv_describe_cluster_name(ExErrorFunction errorFunc, std::string& _return);
      bool recv_execute_cql_query(ExErrorFunction errorFunc, ::org::apache::cassandra::CqlResult& _return);


  };

} //namespace teamspeak
} //namespace agamemnon

#endif // AGCASSANDRACOBCLIENT_H
