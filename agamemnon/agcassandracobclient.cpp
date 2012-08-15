#include "agcassandracobclient.h"
#include <thrift/transport/TTransportException.h>
#include <thrift/async/TAsyncChannel.h>
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
      
void AgCassandraCobClient::checkTransportErrors(){
  if (!channel_->good()){
    if (channel_->timedOut()){
      throw TTransportException(TTransportException::TIMED_OUT);
    } else {
      throw TTransportException(TTransportException::UNKNOWN);
    }
  }
}

} //namespace teamspeak
} //namespace agamemnon