#include "agamemnon_types.h"
#include "../thriftcassandra/cassandra_types.h"
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocolException.h>
#include <thrift/transport/TTransportException.h>

//using namespace ;
//using namespace apache::thrift;
  
namespace teamspeak{
namespace agamemnon{
  
/*static*/
Error Error::TranslateException(const std::exception* ex)
{
  const ::org::apache::cassandra::NotFoundException* nfe = dynamic_cast<const ::org::apache::cassandra::NotFoundException*> (ex);
  if (nfe) return Error(NotFoundException, nfe->what());
  
  const ::org::apache::cassandra::InvalidRequestException* ire = dynamic_cast<const ::org::apache::cassandra::InvalidRequestException*>(ex);
  if (ire) return Error(InvalidRequestException, ire->why);

  const ::org::apache::cassandra::UnavailableException* ue = dynamic_cast<const ::org::apache::cassandra::UnavailableException*>(ex);
  if (ue) return Error(UnavailableException, ue->what());
  
  const ::org::apache::cassandra::TimedOutException* toe = dynamic_cast<const ::org::apache::cassandra::TimedOutException*>(ex);
  if (toe) return Error(TimedOutException, toe->what());
  
  const ::org::apache::cassandra::AuthenticationException* athee = dynamic_cast<const ::org::apache::cassandra::AuthenticationException*>(ex);
  if (athee) return Error(AuthenticationException, athee->why);
  
  const ::org::apache::cassandra::AuthorizationException* athoe = dynamic_cast<const ::org::apache::cassandra::AuthorizationException*>(ex);
  if (athoe) return Error(AuthorizationException, athoe->why);
  
  const ::org::apache::cassandra::SchemaDisagreementException* sde = dynamic_cast<const ::org::apache::cassandra::SchemaDisagreementException*>(ex);
  if (sde) return Error(SchemaDisagreementException, sde->what());
  
  const ::apache::thrift::TApplicationException* ae = dynamic_cast<const ::apache::thrift::TApplicationException*>(ex);
  if (ae) return Error(ApplicationException, ae->what());
  
  const ::apache::thrift::protocol::TProtocolException* pe = dynamic_cast<const ::apache::thrift::protocol::TProtocolException*>(ex);
  if (pe) return Error(ProtocolException, pe->what());
  
  const ::apache::thrift::transport::TTransportException* tre = dynamic_cast<const ::apache::thrift::transport::TTransportException*>(ex);
  if (tre){
    if (tre->getType() == ::apache::thrift::transport::TTransportException::TIMED_OUT) return (Error(TransportTimeoutException, tre->what()));
    return Error(TransportException, tre->what());
  }
  
  return Error(GenericException, ex->what());
}

} //namespace teamspeak
} //namespace agamemnon