#include "agamemnon_types.h"
#include "../thriftcassandra/cassandra_types.h"
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocolException.h>
#include <thrift/transport/TTransportException.h>

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

#define CASERET(name) case name : return #name 

std::string Error::getTypeString() const
{
  switch(type)
  {
      CASERET(GenericException);
      CASERET(NotFoundException);
      CASERET(InvalidRequestException);
      CASERET(UnavailableException);
      CASERET(TimedOutException);
      CASERET(AuthenticationException);
      CASERET(AuthorizationException);
      CASERET(SchemaDisagreementException);
      CASERET(ApplicationException);
      CASERET(ProtocolException);
      CASERET(TransportException);
      CASERET(TransportTimeoutException);
      default: return "unknown error type";
  }
}

UUID::UUID()
:m_Data(16, 0)
{
}

UUID::UUID(const std::string& UUIDString)
:m_Data(UUIDString)
{
  if(m_Data.size()!=16) m_Data.clear();
}

UUID::UUID(Bytes& bytes)
:m_Data(bytes.asString())
{
  if(m_Data.size()!=16) m_Data.clear();
}

void UUID::setAsBytes(const char* UUIDbytes)
{
  m_Data.assign(UUIDbytes, 16);
}

std::string UUID::asString() const
{
  return UUID::bytesToString(m_Data);
}

const std::string& UUID::asBytesString() const
{
  return m_Data;
}

Bytes UUID::asBytes() const
{
  return Bytes(m_Data);
}

static void charsToHex(const char* src, char* dest, int byteCount)
{
  const unsigned char* usrc = reinterpret_cast<const unsigned char*>(src);
  
  static std::string chr="0123456789abcdef";
  for(int i=0; i < byteCount; ++i)
  {
    unsigned char v = usrc[i];
    *dest++ = chr.data()[ v >> 4 ];
    *dest++ = chr.data()[ v & 0x0F ];
  }
}

static bool hexToBuffer(const char* src, size_t srcLen, unsigned char* dest)
{
  BOOST_ASSERT(srcLen%2 == 0);
  
  unsigned char v=0;
  for (size_t i=0; i < srcLen; ++i)
  {
    char c = src[i];
    char nc = c-'0';
    if (nc >=0 && nc < 10) 
    {
      v = (v<<4) | nc;
    } else {
      char lc = c-'a';
      if (lc >=0 && lc <6) 
      {
	v = (v<<4) | (lc+10);
      } else {
	char uc = c-'A';
	if (uc >= 0 && uc < 6)
	{
	  v = (v<<4) | (uc+10);
	} else return false;
      }
    }
    
    if (i % 2 == 1) dest[i/2]= v;
  }
  
  return true;
  
}

/*static*/
std::string UUID::bytesToString(const std::string& bytesString)
{
  if (bytesString.size()!=16) return std::string();
  std::string result(36, 0);
  result[8]=result[13]=result[18]=result[23]='-';
  charsToHex(bytesString.data(),    const_cast<char*>(result.data()),    4);
  charsToHex(bytesString.data()+  4, const_cast<char*>(result.data())+ 9, 2);
  charsToHex(bytesString.data()+  6, const_cast<char*>(result.data())+14, 2);
  charsToHex(bytesString.data()+  8, const_cast<char*>(result.data())+19, 2);
  charsToHex(bytesString.data()+ 10, const_cast<char*>(result.data())+24, 6);
  return result;
}

/*static*/
std::string UUID::UUIDToBytesString(const std::string& UUIDString)
{
    if (UUIDString.size()!=36) return std::string();
    std::string result(16,0);
    unsigned char* buffer = reinterpret_cast<unsigned char*>(const_cast<char*>(result.data()));
    if (!hexToBuffer(UUIDString.data(),     8, buffer)) return std::string();
    if (!hexToBuffer(UUIDString.data()+9,   4, buffer+ 4)) return std::string();
    if (!hexToBuffer(UUIDString.data()+14,  4, buffer+ 6)) return std::string();
    if (!hexToBuffer(UUIDString.data()+19,  4, buffer+ 8)) return std::string();
    if (!hexToBuffer(UUIDString.data()+24, 12, buffer+10)) return std::string();
    
    return result;
}
  
Bytes::Bytes(const std::string& bytes)
: m_Data(bytes)
{
}

Bytes::Bytes(const char* bytes, size_t len)
:m_Data(bytes,len)
{
}

size_t Bytes::size() const
{
  return m_Data.size();
}

const char* Bytes::data() const
{
  return m_Data.data();
}

const std::string& Bytes::asString() const
{
  return m_Data;
}

std::string Bytes::asHexString() const
{
  return bytesStringToHex(m_Data);
}

/*static*/
std::string Bytes::hexToBytesString(const std::string& hexString)
{
  if (hexString.size()% 2 != 0) return std::string();
  std::string result(hexString.size()/2, 0);
  bool ok = hexToBuffer(hexString.data(), hexString.size(), reinterpret_cast<unsigned char*>(const_cast<char*>(result.data())));
  if (!ok) return std::string();
  return result;
}

/*static*/
std::string Bytes::bytesStringToHex(const std::string& bytes)
{
  std::string result(bytes.size()*2, 0);
  charsToHex(bytes.data(), const_cast<char*>(result.data()), bytes.size());
  return result;
}

} //namespace teamspeak
} //namespace agamemnon