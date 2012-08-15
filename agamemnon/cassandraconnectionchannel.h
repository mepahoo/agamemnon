#ifndef CASSANDRACONNECTIONCHANNEL_H
#define CASSANDRACONNECTIONCHANNEL_H

#include <thrift/async/TAsyncChannel.h>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace teamspeak{
namespace agamemnon{
  
class CassandraConnectionChannel : public ::apache::thrift::async::TAsyncChannel, public boost::enable_shared_from_this<CassandraConnectionChannel>
{
  public:
    CassandraConnectionChannel(boost::asio::io_service& ioservice, int ioTimeoutms, int replyTimeoutms);
    
    void connect(const boost::asio::ip::tcp::endpoint& endpoint, int connectTimeoutms, const VoidCallback& cob);
    void close();
    
    /*override*/ bool good() const;
    /*override*/ bool error() const;
    /*override*/ bool timedOut() const;

    /*override*/ void sendMessage(const VoidCallback& cob, apache::thrift::transport::TMemoryBuffer* message);

    /*override*/ void recvMessage(const VoidCallback& cob, apache::thrift::transport::TMemoryBuffer* message);
    
  private:
    void settimer(int ms);
    void updateTimerContext();
    void handleError(const boost::system::error_code& error);
    void onTimerExipres(const boost::system::error_code& error, uint32_t timerContext);
    void connect_handler(const boost::system::error_code& error, const VoidCallback& cob);
    void write_handler(const boost::system::error_code& error, std::size_t bytes_transferred, std::size_t bytes_written,  apache::thrift::transport::TMemoryBuffer* message, const VoidCallback& cob);
    
    void read_size_handler(const boost::system::error_code& error, std::size_t bytes_transferred, std::size_t bytes_to_read,  apache::thrift::transport::TMemoryBuffer* message, const VoidCallback& cob);
    void read_message_handler(const boost::system::error_code& error, std::size_t bytes_transferred, std::size_t bytes_to_read,  apache::thrift::transport::TMemoryBuffer* message, const VoidCallback& cob);
    
    bool                         m_Error;
    bool                         m_TimedOut;
    boost::asio::ip::tcp::socket m_Socket;
    boost::asio::deadline_timer  m_Timer;
    int                          m_IOTimeoutms;
    int                          m_ReplyTimeoutms;
    uint32_t                     m_Insize;
    volatile uint32_t            m_TimerContext;
};

} //namespace teamspeak
} //namespace agamemnon

#endif // CASSANDRACONNECTIONCHANNEL_H
