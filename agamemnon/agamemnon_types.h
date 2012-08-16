#ifndef AGAMEMNON_TYPES_H
#define AGAMEMNON_TYPES_H

#include <boost/function.hpp>

namespace teamspeak{
namespace agamemnon{
  
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

} //namespace teamspeak
} //namespace agamemnon

#endif // AGAMEMNON_TYPES_H
