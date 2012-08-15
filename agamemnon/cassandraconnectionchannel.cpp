#include "cassandraconnectionchannel.h"

#include <iostream>
#include <boost/bind.hpp>
#include <thrift/transport/TBufferTransports.h>
#include <boost/interprocess/detail/atomic.hpp>

namespace teamspeak{
namespace agamemnon{

CassandraConnectionChannel::CassandraConnectionChannel(boost::asio::io_service& ioservice, int ioTimeoutms, int replyTimeoutms)
: ::apache::thrift::async::TAsyncChannel()
, boost::enable_shared_from_this<CassandraConnectionChannel>()
, m_Error(false)
, m_TimedOut(false)
, m_Socket(ioservice)
, m_Timer(ioservice)
, m_IOTimeoutms(ioTimeoutms)
, m_ReplyTimeoutms(replyTimeoutms)
, m_TimerContext(0)
{
}

void CassandraConnectionChannel::connect(const boost::asio::ip::tcp::endpoint& endpoint, int connectTimeoutms, const VoidCallback& cob){
  
  //connect
  m_Socket.async_connect(endpoint, 
			 boost::bind(&CassandraConnectionChannel::connect_handler, 
				     shared_from_this(), 
				     boost::asio::placeholders::error, 
				     cob));
  settimer(connectTimeoutms);
}

void CassandraConnectionChannel::close(){
  m_Socket.close();
  m_Timer.cancel();
}

/*override*/ 
bool CassandraConnectionChannel::good() const{
  return !(m_Error || m_TimedOut);
}

/*override*/ 
bool CassandraConnectionChannel::error() const{
  return m_Error;
}

/*override*/ 
bool CassandraConnectionChannel::timedOut() const{
  return m_TimedOut;
}

/*override*/ 
void CassandraConnectionChannel::sendMessage(const VoidCallback& cob, apache::thrift::transport::TMemoryBuffer* message){
  //set message size
  uint32_t size;
  uint8_t* bufPtr;
  message->getBuffer(&bufPtr, &size);
  uint32_t* frameSize = reinterpret_cast<uint32_t*>(bufPtr);
  *frameSize = htonl(size-4);

  settimer(m_IOTimeoutms);

  m_Socket.async_write_some(boost::asio::buffer(bufPtr, size),
			    boost::bind(& CassandraConnectionChannel::write_handler, 
					shared_from_this(), 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					0,
					message,
					cob));
}

/*override*/ 
void CassandraConnectionChannel::recvMessage(const VoidCallback& cob, apache::thrift::transport::TMemoryBuffer* message){
  settimer(m_ReplyTimeoutms);

  m_Socket.async_read_some(boost::asio::buffer(&m_Insize, 4),
			    boost::bind(& CassandraConnectionChannel::read_size_handler, 
					shared_from_this(), 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					4,
					message,
					cob));
}

void CassandraConnectionChannel::settimer(int ms){
  uint32_t newContext =  boost::interprocess::detail::atomic_read32(&m_TimerContext);
  //start timeout timer
  m_Timer.expires_from_now(boost::posix_time::milliseconds(ms));
  m_Timer.async_wait(boost::bind(&CassandraConnectionChannel::onTimerExipres, shared_from_this(), boost::asio::placeholders::error, newContext));

}

void CassandraConnectionChannel::updateTimerContext(){
  boost::interprocess::detail::atomic_inc32(&m_TimerContext);
}

void CassandraConnectionChannel::handleError(const boost::system::error_code& error){
  if (error){
    m_Error = true;
  }
}

void CassandraConnectionChannel::onTimerExipres(const boost::system::error_code& error, uint32_t timerContext){
  if(!error){
    //check if the async operation completed anyway.
    if (timerContext == boost::interprocess::detail::atomic_read32(&m_TimerContext)){
	//cancel alll io's by closing the socket
	m_TimedOut = true;
	m_Socket.close();
    }
    return;
  }
  
  if (error != boost::asio::error::operation_aborted){
    std::cerr << "Error in CassandraConnectionChannel::onTimerExipres: " << error.message() << std::endl;
  }
}

void CassandraConnectionChannel::connect_handler(const boost::system::error_code& error, const VoidCallback& cob){
  updateTimerContext();
  handleError(error);
  cob();
}

void CassandraConnectionChannel::write_handler(const boost::system::error_code& error, std::size_t bytes_transferred, std::size_t bytes_written,  apache::thrift::transport::TMemoryBuffer* message, const VoidCallback& cob){
  updateTimerContext();
  if (error){
    handleError(error);
    return;
  }
  
  //get message size
  uint32_t size;
  uint8_t* bufPtr;
  message->getBuffer(&bufPtr, &size);

  bytes_written += bytes_transferred;
  
  if (bytes_written == size){
    //we are done writing
    m_Timer.cancel();
    cob();
    return;
  }
  
  //we need to continue
  bufPtr += bytes_written;
  m_Socket.async_write_some(boost::asio::buffer(bufPtr, size-bytes_written),
			    boost::bind(& CassandraConnectionChannel::write_handler, 
					shared_from_this(), 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					bytes_written,
					message,
					cob));

  settimer(m_IOTimeoutms);
}

void CassandraConnectionChannel::read_size_handler(const boost::system::error_code& error, std::size_t bytes_transferred, std::size_t bytes_to_read,  apache::thrift::transport::TMemoryBuffer* message, const VoidCallback& cob){
  updateTimerContext();
  if (error){
    handleError(error);
    return;
  }

  settimer(m_IOTimeoutms);
  
  bytes_to_read -= bytes_transferred;
  if (!bytes_to_read){
    //we are done, read the message itself
    m_Insize = ntohl(m_Insize);
    uint8_t* bufPtr = message->getWritePtr(m_Insize);
    

    m_Socket.async_read_some(boost::asio::buffer(bufPtr, m_Insize),
			     boost::bind(& CassandraConnectionChannel::read_message_handler, 
					shared_from_this(), 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					m_Insize,
					message,
					cob));
    return;
  }

  //need to read more
  uint8_t* bufPtr = reinterpret_cast<uint8_t*>(&m_Insize);
  bufPtr+= (4-bytes_to_read);
  m_Socket.async_read_some(boost::asio::buffer(bufPtr, bytes_to_read),
			   boost::bind(& CassandraConnectionChannel::read_size_handler, 
					shared_from_this(), 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					bytes_to_read,
				        message,
					cob));
}

void CassandraConnectionChannel::read_message_handler(const boost::system::error_code& error, std::size_t bytes_transferred, std::size_t bytes_to_read,  apache::thrift::transport::TMemoryBuffer* message, const VoidCallback& cob){
  updateTimerContext();
  if (error){
    handleError(error);
    return;
  }

  bytes_to_read -= bytes_transferred;
  message->wroteBytes(bytes_transferred);
  
  if (bytes_to_read){
    //we are not done, continue
    uint8_t* bufPtr = message->getWritePtr(bytes_to_read);
    
    m_Socket.async_read_some(boost::asio::buffer(bufPtr, bytes_to_read),
			     boost::bind(& CassandraConnectionChannel::read_message_handler, 
					shared_from_this(), 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					bytes_to_read,
					message,
					cob));

    settimer(m_IOTimeoutms);    
    
    return;
  }

  //we are done.
  m_Timer.cancel();
  cob();
}

} //namespace teamspeak
} //namespace agamemnon