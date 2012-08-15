#include "singleconnectionfactory.h"
#include <thrift/transport/TTransportException.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "cassandraconnectionchannel.h"
#include "agcassandracobclient.h"

namespace teamspeak{
namespace agamemnon{

SingleConnectionFactory::SingleConnectionFactory(boost::asio::io_service& IOService, const std::string& host, const ConnectionCommonSettings& connectionCommonSettings)
: ConnectionFactory()
, m_PrivImpl(new PrivImpl(IOService, host, connectionCommonSettings))
{
}

/*override*/
void SingleConnectionFactory::getConnection(ErrorFunction errorFunc, boost::function<void(CassandraConnection::Ptr)> callback)
{
  m_PrivImpl->getConnection(errorFunc, callback);
}
  
/*override*/
int  SingleConnectionFactory::retryDelayOnTimeout() const
{
  return m_PrivImpl->retryDelayOnTimeout();
}

/*override*/
int  SingleConnectionFactory::maxRetryCount() const
{
  return m_PrivImpl->maxRetryCount();
}

SingleConnectionFactory::PrivImpl::PrivImpl(boost::asio::io_service& IOService, const std::string& host, const ConnectionCommonSettings& connectionCommonSettings)
:boost::enable_shared_from_this<PrivImpl>()
,m_Resolver(IOService)
,m_Host(host)
,m_CassandraConnection(NULL)
,m_Mutex()
,m_Callbacks()
,m_Busy(false)
,m_ProtocolFactory(new ::apache::thrift::protocol::TBinaryProtocolFactoryT< ::apache::thrift::transport::TMemoryBuffer>())
,m_ConnectionCommonSettings(connectionCommonSettings)
{
}

SingleConnectionFactory::PrivImpl::~PrivImpl()
{
  delete m_ProtocolFactory;
  m_ProtocolFactory = NULL;
  
  delete m_CassandraConnection;
  m_CassandraConnection = NULL;
}
 
 void SingleConnectionFactory::PrivImpl::getConnection(ErrorFunction errorFunc, boost::function<void(CassandraConnection::Ptr)> callback)
 {
   CassandraConnection* connection = NULL;
   {
      boost::lock_guard<boost::mutex> lock(m_Mutex);
      if (!m_Busy) {
	if (m_CassandraConnection) {
	  m_Busy = true;
	  connection = m_CassandraConnection;
	  m_CassandraConnection = NULL;
	  //callback is done below
	} else {
	  m_Callbacks.push(RequestQueueItem(errorFunc, callback));
	  startNewConnection(); //sets m_Busy
	  return;
	}
      } else { //m_busy == true
	m_Callbacks.push(RequestQueueItem(errorFunc, callback));
	return;
      }
   }
    
   callback(createConnectionSharedPointer(connection));
 }
        
int  SingleConnectionFactory::PrivImpl::retryDelayOnTimeout() const
{
  return m_ConnectionCommonSettings.retryDelayOnTimeoutms;
}

int  SingleConnectionFactory::PrivImpl::maxRetryCount() const
{
  return m_ConnectionCommonSettings.maxRetryCount;
}

void SingleConnectionFactory::PrivImpl::startNewConnection()
{
  m_Busy = true;
  m_ReuseCount = 0;
  boost::asio::ip::tcp::resolver::query query(m_Host, boost::lexical_cast<std::string>(m_ConnectionCommonSettings.port));
  m_Resolver.async_resolve(query, boost::bind(&SingleConnectionFactory::PrivImpl::resolveHandler, shared_from_this(), boost::asio::placeholders::error(), _2 ));
}

void SingleConnectionFactory::PrivImpl::resolveHandler(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator iterator)
{
    if (error || iterator == boost::asio::ip::tcp::resolver::iterator()){
      ErrorFunction errorFunc;
      {
	boost::lock_guard<boost::mutex> lock(m_Mutex);
	errorFunc = m_Callbacks.front().errorFunc;
	m_Callbacks.pop();
	if (!m_Callbacks.empty()) {
	  startNewConnection();
	} else {
	  m_Busy = false;
	}
      }
      
      ::apache::thrift::transport::TTransportException errorParam(::apache::thrift::transport::TTransportException::UNKNOWN, "Unable to resolve host");
      errorFunc(&errorParam);
      
    } else {
       CassandraConnectionChannel* channel = new CassandraConnectionChannel(m_Resolver.get_io_service(), m_ConnectionCommonSettings.ioTimeoutms, m_ConnectionCommonSettings.replyTimeoutms);
       boost::shared_ptr< ::apache::thrift::async::TAsyncChannel> channelPtr(channel);
       channel->connect(*iterator, m_ConnectionCommonSettings.connectTimeoutms, boost::bind(&SingleConnectionFactory::PrivImpl::connected, shared_from_this(), channelPtr));
    }
}

void SingleConnectionFactory::PrivImpl::connected(boost::shared_ptr< ::apache::thrift::async::TAsyncChannel> channelPtr)
{

  if (channelPtr->good()){
      boost::function<void(CassandraConnection::Ptr)> callback;
      {
        boost::lock_guard<boost::mutex> lock(m_Mutex);
	callback = m_Callbacks.front().callback;
	m_Callbacks.pop();
      }
      AgCassandraCobClient* agClient = new AgCassandraCobClient(channelPtr, m_ProtocolFactory);
      callback(createConnectionSharedPointer(new CassandraConnection(agClient)));
  } else {
      ErrorFunction errorFunc;
      {
	boost::lock_guard<boost::mutex> lock(m_Mutex);
	errorFunc = m_Callbacks.front().errorFunc;
	m_Callbacks.pop();
	if (!m_Callbacks.empty()){
	  startNewConnection();
	} else {
	  m_Busy = false;
	} 
      }
      ::apache::thrift::transport::TTransportException errorParam(::apache::thrift::transport::TTransportException::UNKNOWN, "Unable to connect to resolved host");
      errorFunc(&errorParam);
  }
}

void SingleConnectionFactory::PrivImpl::connectionDone(CassandraConnection* connection)
{
  
  boost::function<void(CassandraConnection::Ptr)> callback;
  
  {
    boost::lock_guard<boost::mutex> lock(m_Mutex);
    m_ReuseCount++;
    
    if (connection->needToCloseWhenDone() || m_ReuseCount >= m_ConnectionCommonSettings.maxConnectionReuseCount){
      delete connection;
      if (!m_Callbacks.empty()) {
	startNewConnection();
      } else {
	m_Busy = false;
      }
      return;
    }
    
    if (!m_Callbacks.empty()) {
      callback = m_Callbacks.front().callback;
      m_Callbacks.pop();
    } else {
      m_Busy = false;
      return;
      }
  }
  
  callback(createConnectionSharedPointer(connection));
}

CassandraConnection::Ptr SingleConnectionFactory::PrivImpl::createConnectionSharedPointer(CassandraConnection* connection)
{
  return CassandraConnection::Ptr(connection, boost::bind(&SingleConnectionFactory::PrivImpl::connectionDone, shared_from_this(), _1));
}

} //namespace teamspeak
} //namespace agamemnon
