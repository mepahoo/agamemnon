#ifndef AGPREPAREDQUERY_H
#define AGPREPAREDQUERY_H

#include "agamemnon_types.h"
#include "agglobalpreparedquerycache.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <exception>

namespace teamspeak{
namespace agamemnon{

class PreparedQueryException : public std::exception
{
  public:
    enum Cause {INDEX};
    PreparedQueryException(Cause cause) : std::exception(), m_Cause(cause){};
    virtual ~PreparedQueryException() throw() {};
    virtual const char* what() const throw() { 
      switch (m_Cause){
	case INDEX: return "Index out of range"; 
	default: return "unknown PreparedQueryException";
      }
    }
  private:
    Cause m_Cause;
};

class PreparedQuery
{
  public:
    typedef boost::shared_ptr<PreparedQuery> Ptr;
    
    size_t fieldCount() const;
    
    const std::string& getFieldName(size_t idx) const;
    int getFieldIndex(const std::string& fieldName) const;
    size_t getRequiredFieldIndex(const std::string& fieldName) const;
    
    ColumnDataType  getFieldType(size_t idx) const;
    ColumnDataType  getFieldType(const std::string& fieldName) const;
    
    void clear();
    void clear(size_t idx);
    void clear(const std::string& fieldName);
    
    void set(size_t idx, const std::string& value); //also works for UUID and Bytes (or any other data type) in raw data form
    void set(const std::string& fieldName, const std::string& value);
    
    void set(size_t idx, const Bytes& value);
    void set(const std::string& fieldName, const Bytes& value);
    
    void set(size_t idx, const UUID& value);
    void set(const std::string& fieldName, const UUID& value);

    void set(size_t idx, int64_t value);
    void set(const std::string& fieldName, int64_t value);
    
    void set(size_t idx, bool value);
    void set(const std::string& fieldName, bool value);

    void set(size_t idx, double value);
    void set(const std::string& fieldName, double value);
    
    void set(size_t idx, float value);
    void set(const std::string& fieldName, float value);
    
    void set(size_t idx, int32_t value);
    void set(const std::string& fieldName, int32_t value);

    void set(size_t idx, const boost::posix_time::ptime& value); //always use utc
    void set(const std::string& fieldName, const boost::posix_time::ptime& value); //always use utc
    
  private:
    PreparedQuery(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr& fieldDesc);
    
    std::vector<std::string>                         m_Data;
    GlobalPreparedQueryCache::PreparedQueryInfo::Ptr m_FieldDesc;
    
    friend class Client;
    friend class CassandraConnection;
};

} //namespace teamspeak
} //namespace agamemnon

#endif // AGPREPAREDQUERY_H
