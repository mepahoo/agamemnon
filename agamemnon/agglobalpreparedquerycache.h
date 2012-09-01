#ifndef AGGLOBALPREPAREDQUERYCACHE_H
#define AGGLOBALPREPAREDQUERYCACHE_H

#include "agamemnon_types.h"
#include "ag_private.h"
#include <boost/detail/atomic_count.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/thread.hpp>
#include <map>
#include <exception>

namespace org { namespace apache { namespace cassandra {
  class CqlPreparedResult;
}}}

namespace teamspeak{
namespace agamemnon{
  
class GlobalPreparedQueryCacheException : public std::exception
{
  public:
    GlobalPreparedQueryCacheException() : std::exception(){};
    virtual ~GlobalPreparedQueryCacheException() throw() {};
    virtual const char* what() const throw() { return "GlobalPreparedQueryCache";}
};

class GlobalPreparedQueryCache
{
  public:
    struct FieldData
    {
      FieldData(ColumnDataType _fieldType, const std::string& _fieldName);
      ColumnDataType fieldType;
      std::string    fieldName;
      
      bool operator ==(const FieldData& other) const 
      {
	return (this->fieldType == other.fieldType) && (this->fieldName == other.fieldName);
      }
      
      bool operator !=(const FieldData& other) const 
      {
	return (this->fieldType != other.fieldType) || (this->fieldName != other.fieldName);
      }
    };

    typedef std::vector<FieldData> FieldDataVector;
    
    class PreparedQueryInfo{
      public:
	typedef boost::intrusive_ptr<PreparedQueryInfo> Ptr;

	PreparedQueryInfo(const std::string& _query, const FieldDataVector& _fieldData);
	const std::string     query;
	const FieldDataVector fieldData;

	friend void intrusive_ptr_add_ref(PreparedQueryInfo const* s);
	friend void intrusive_ptr_release(PreparedQueryInfo const* s);
	friend class GlobalPreparedQueryCache;
      private:
         mutable boost::detail::atomic_count ref_count;
    };
    
    static GlobalPreparedQueryCache* getInstance() {return &instance;}
    PreparedQueryInfo::Ptr getPreparedQueryInfo(const std::string& query);
    PreparedQueryInfo::Ptr storePreparedQuery(const std::string& query, const org::apache::cassandra::CqlPreparedResult* cqlPreparedResult);
    
  private:
    
    typedef std::map<const std::string*, PreparedQueryInfo*, StringPointerCompare> PreparedQueryInfoMap;
    
    GlobalPreparedQueryCache();
    ~GlobalPreparedQueryCache();
    
    bool remove(PreparedQueryInfo const* s); 
    
    FieldDataVector cqlPreparedResultToFieldDataVector(const org::apache::cassandra::CqlPreparedResult* cqlPreparedResult) const;
    bool isSame(const FieldDataVector& a, const FieldDataVector& b) const;
        
    static GlobalPreparedQueryCache instance;

    PreparedQueryInfoMap        m_PreparedQueryInfoMap;
    boost::shared_mutex         m_Mutex;
    
    friend void intrusive_ptr_release(PreparedQueryInfo const* s);
};

} //namespace teamspeak
} //namespace agamemnon

#endif // AGGLOBALPREPAREDQUERYCACHE_H
