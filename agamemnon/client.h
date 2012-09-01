#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <exception>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "agamemnon_types.h"
#include "connectionfactory.h"
#include "agcqlqueryresult.h"
#include "agpreparedquery.h"

namespace teamspeak{
namespace agamemnon{

class Client : public boost::enable_shared_from_this<Client>
{
  public:
    typedef boost::shared_ptr<Client> Ptr;
    
    Client(boost::asio::io_service& IOService, ConnectionFactory::Ptr& connectionFactory);

    void getClusterName(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback);
    void prepareQuery(const std::string& cql, ErrorFunction errorFunc, boost::function<void(PreparedQuery::Ptr)> callback);
    void excecuteCQL(const std::string& cql, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback);
    void executePreparedQuery(const PreparedQuery::Ptr & fieldData, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback);
    
  private:
    void ensureConnection(ErrorFunction errorFunc, boost::function<void(CassandraConnection::Ptr connection)> continuation);
    void ensureConnection_done(CassandraConnection::Ptr connection, boost::function<void(CassandraConnection::Ptr connection)> continuation);
    void generic_RetryError(const Error& error, ErrorFunction errorFunc, boost::function<void()> retryFunc);
    void generic_ReconnectWaitDone(const boost::system::error_code& error, ErrorFunction errorFunc, boost::function<void()> retryFunc);
    
    void prepareQuery_done(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr& preparedQueryInfo, boost::function<void(PreparedQuery::Ptr)> callback);

    ConnectionFactory::Ptr                m_ConnectionFactory;
    CassandraConnection::Ptr              m_Connection;
    int                                   m_RetryCount;
    boost::asio::deadline_timer           m_Timer;
};

} //namespace teamspeak
} //namespace agamemnon

#endif // CLIENT_H
