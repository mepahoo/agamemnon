#ifndef AGCASSANDRACOBCLIENT_H
#define AGCASSANDRACOBCLIENT_H

#include "../thriftcassandra/Cassandra.h"
#include <boost/shared_ptr.hpp>

namespace teamspeak{
namespace agamemnon{
  
  class AgCassandraCobClient : public ::org::apache::cassandra::CassandraCobClient{
    public:
      
      AgCassandraCobClient(boost::shared_ptr< ::apache::thrift::async::TAsyncChannel> channel, ::apache::thrift::protocol::TProtocolFactory* protocolFactory)
      : CassandraCobClient(channel, protocolFactory)
      {
      }
      
      void resetBuffers();
      void checkTransportErrors();

  };

} //namespace teamspeak
} //namespace agamemnon

#endif // AGCASSANDRACOBCLIENT_H
