#ifndef SINGLECONNECTIONFACTORY_H
#define SINGLECONNECTIONFACTORY_H

#include "connectionfactory.h"
#include "cassandraconnection.h"
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <queue>
#include <thrift/protocol/TBinaryProtocol.h>

namespace teamspeak{
namespace agamemnon{
  
class SingleConnectionFactory : public ConnectionFactory
{
  public:
    SingleConnectionFactory(boost::asio::io_service& IOService, const std::string& host, const ConnectionCommonSettings& connectionCommonSettings);
    /*override*/ void getConnection(ErrorFunction errorFunc, boost::function<void(CassandraConnection::Ptr)> callback);
    /*override*/ int  retryDelayOnTimeout() const;
    /*override*/ int  maxRetryCount() const;
  private:
    class PrivImpl : public boost::enable_shared_from_this<PrivImpl>{
      public:
        PrivImpl(boost::asio::io_service& IOService, const std::string& host, const ConnectionCommonSettings& connectionCommonSettings);
	~PrivImpl();
        void getConnection(ErrorFunction errorFunc, boost::function<void(CassandraConnection::Ptr)> callback);
        int  retryDelayOnTimeout() const;
        int  maxRetryCount() const;
      private:
	void startNewConnection();
        void resolveHandler(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator iterator);
	void connected(boost::shared_ptr< ::apache::thrift::async::TAsyncChannel> channelPtr);
	void cqlVersionSet(CassandraConnection::Ptr connection);
	void cqlVersionSet_fail(Error error);
	void connectionDone(CassandraConnection* connection);
	CassandraConnection::Ptr createConnectionSharedPointer(CassandraConnection* connection);

        boost::asio::ip::tcp::resolver m_Resolver;
	const std::string              m_Host;
        CassandraConnection*           m_CassandraConnection;
        boost::mutex                   m_Mutex;
        std::queue<RequestQueueItem>   m_Callbacks;
	bool                           m_Busy;
	::apache::thrift::protocol::TBinaryProtocolFactoryT< ::apache::thrift::transport::TMemoryBuffer>* m_ProtocolFactory;
	ConnectionCommonSettings       m_ConnectionCommonSettings;
	int                            m_ReuseCount;
    };
    
    boost::shared_ptr<PrivImpl> m_PrivImpl;
};

} //namespace teamspeak
} //namespace agamemnon
#endif // SINGLECONNECTIONFACTORY_H
