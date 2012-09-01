#ifndef AGAMEMNON_TYPES_H
#define AGAMEMNON_TYPES_H

#include <boost/function.hpp>

namespace teamspeak{
namespace agamemnon{
  
  enum TimeAdjust{ TA_LOCALTIME, TA_UTCTIME};
  
  enum ColumnDataType {CDT_UNKNOWN, CDT_ASCII, CDT_INT64, CDT_BYTES, CDT_BOOLEAN, CDT_COUNTER, CDT_DECIMAL, CDT_DOUBLE, CDT_FLOAT, CDT_INT32, CDT_UTF8, CDT_DATE, CDT_UUID, CDT_INTEGER};
      
  struct Error
  {
    enum Type
    {
      GenericException            = 0,
      NotFoundException           = 1,
      InvalidRequestException     = 2,
      UnavailableException        = 3,
      TimedOutException           = 4,
      AuthenticationException     = 5,
      AuthorizationException      = 6, 
      SchemaDisagreementException = 7,
      ApplicationException        = 8,
      ProtocolException           = 9,
      TransportException          = 10,
      TransportTimeoutException   = 11
    };
    
    Error(Type _type): type(_type), message(){}
    Error(Type _type, const std::string& _message): message(_message){}
    static Error TranslateException(const std::exception* ex);
    
    std::string getTypeString() const;
    
    Type        type;
    std::string message;
  };

  typedef boost::function<void(Error)> ErrorFunction;
  class Bytes;
  
  class UUID
  {
    public:
      UUID();
      UUID(const std::string& UUIDString);
      UUID(Bytes& bytes);
      
      void setAsBytes(const char* UUIDbytes); //16 bytes implied
      
      std::string asString() const;
      const std::string& asBytesString() const;
      Bytes asBytes() const;
      
      bool isValid() const { return m_Data.size() == 16; }
      
      static std::string bytesToString(const std::string& bytesString);
      static std::string UUIDToBytesString(const std::string& UUIDString);
    private:
      std::string m_Data;
  };
  
  class Bytes
  {
    public:
      Bytes(const std::string& bytes);
      Bytes(const char* bytes, size_t len);
      
      size_t size() const;
      const char* data() const;
      const std::string& asString() const;
      std::string asHexString() const;
      
      static std::string hexToBytesString(const std::string& hexString);
      static std::string bytesStringToHex(const std::string& bytes);
      static std::string bytesStringToHex(const char * bytes, size_t bytesLen);
    private:
      std::string m_Data;
  };

} //namespace teamspeak
} //namespace agamemnon

#endif // AGAMEMNON_TYPES_H
