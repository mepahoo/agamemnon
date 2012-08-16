#include "agcassandracobclient.h"
#include <thrift/transport/TTransportException.h>
#include <thrift/async/TAsyncChannel.h>
#include "../thriftcassandra/cassandra_types.h"
#include <thrift/protocol/TProtocolException.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>

namespace teamspeak{
namespace agamemnon{
  
using namespace ::apache::thrift::transport;

void AgCassandraCobClient::resetBuffers(){
  itrans_->resetBuffer();
  otrans_->resetBuffer();
  
  //reserve 4 bytes for frame size in out
  otrans_->getWritePtr(4);
  otrans_->wroteBytes(4);
}
      
bool AgCassandraCobClient::checkTransportErrors(ExErrorFunction errorFunc){
  if (!channel_->good()){
    if (channel_->timedOut()){
      TTransportException ex(TTransportException::TIMED_OUT);
      errorFunc(&ex);
      return false;
    } else {
      TTransportException ex(TTransportException::UNKNOWN);
      errorFunc(&ex);
      return false;
    }
  }
  return true;
}

bool AgCassandraCobClient::common_recv(ExErrorFunction errorFunc, const std::string& fname)
{
  int32_t rseqid = 0;
  std::string rfname;
  ::apache::thrift::protocol::TMessageType mtype;

  iprot_->readMessageBegin(rfname, mtype, rseqid);
  if (mtype == ::apache::thrift::protocol::T_EXCEPTION) {
    ::apache::thrift::TApplicationException x;
    x.read(iprot_);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
    errorFunc(&x);
    return false;
  }
  if (mtype != ::apache::thrift::protocol::T_REPLY) {
    iprot_->skip(::apache::thrift::protocol::T_STRUCT);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
  }
  if (fname != rfname) {
    iprot_->skip(::apache::thrift::protocol::T_STRUCT);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();
  }
  return true;
}

bool AgCassandraCobClient::recv_describe_cluster_name(ExErrorFunction errorFunc, std::string& _return)
{

  try{
    if (!common_recv(errorFunc, "describe_cluster_name")) return false;

    ::org::apache::cassandra::Cassandra_describe_cluster_name_presult result;
    result.success = &_return;
    result.read(iprot_);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();

    if (result.__isset.success) {
      // _return pointer has now been filled
      return true;
    }

    ::apache::thrift::TApplicationException ex(::apache::thrift::TApplicationException::MISSING_RESULT, "describe_cluster_name failed: unknown result");
    errorFunc(&ex);
    return false;
  } catch (const std::exception& e){
    errorFunc(&e);
    return false;
  }
}

bool AgCassandraCobClient::recv_execute_cql_query(ExErrorFunction errorFunc, ::org::apache::cassandra::CqlResult& _return)
{

  try{
    if (!common_recv(errorFunc, "execute_cql_query")) return false;
    
    ::org::apache::cassandra::Cassandra_execute_cql_query_presult result;
    result.success = &_return;
    result.read(iprot_);
    iprot_->readMessageEnd();
    iprot_->getTransport()->readEnd();

    if (result.__isset.success) {
      // _return pointer has now been filled
      return true;
    }
    if (result.__isset.ire) {
      errorFunc(&result.ire);
      return false;
    }
    if (result.__isset.ue) {
      errorFunc(&result.ue);
      return false;
    }
    if (result.__isset.te) {
      errorFunc(&result.te);
      return false;
    }
    if (result.__isset.sde) {
      errorFunc(&result.sde);
      return false;
    }
    ::apache::thrift::TApplicationException ex(::apache::thrift::TApplicationException::MISSING_RESULT, "execute_cql_query failed: unknown result");
    errorFunc(&ex);
    return false;
   } catch (const std::exception& e){
    errorFunc(&e);
    return false;
  }
}


} //namespace teamspeak
} //namespace agamemnon