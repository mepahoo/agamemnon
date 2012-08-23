#ifndef AGCQLQUERYRESULT_H
#define AGCQLQUERYRESULT_H

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <exception>
#include <vector>

#include "agamemnon_types.h"

namespace org { namespace apache { namespace cassandra {
class CqlResult;
class Column;
}}}

namespace teamspeak{
namespace agamemnon{
 
class ConversionException : public std::exception
{
  public:
    ConversionException() : std::exception(){};
    virtual ~ConversionException() throw() {};
    virtual const char* what() const throw() { return "ConversionException";}
};

class IndexOutOfRangeException : public std::exception
{
  public:
    IndexOutOfRangeException() : std::exception(){};
    virtual ~IndexOutOfRangeException() throw() {};
    virtual const char* what() const throw() { return "Index out of range"; }
};

class ParseException : public std::exception
{
  public:
    ParseException() : std::exception(){};
    virtual ~ParseException() throw() {};
    virtual const char* what() const throw() { return "Parse error"; }
};

class CQLQueryResult
{
  public:
    typedef boost::shared_ptr<CQLQueryResult> Ptr;
    
    enum ColumnDataType {UNKNOWN, ASCII, INT64, BYTES, BOOLEAN, COUNTER, DECIMAL, DOUBLE, FLOAT, INT32, UTF8, DATE, UUID, INTEGER};
    enum TimeAdjust{ LOCALTIME, UTCTIME};
    
    static const size_t npos = -1;
    class CQLColumnValue
    {
      public:
	CQLColumnValue(const ::org::apache::cassandra::Column& column, ColumnDataType cdt);
	bool                      isNull() const;
	std::string               asString() const;
	const std::string&        asBytes() const;
	int64_t                   asInt64() const;
	bool                      asBool() const;
	double                    asDouble() const;
	float                     asFloat() const;
	int                       asInt() const;
	boost::posix_time::ptime  asDateTime(TimeAdjust ta = UTCTIME) const;
	//virtual --                asDecimal() const;
	int64_t                   writeTime() const;
	int                       TTL() const;
	bool                      hasWriteTime() const;
	bool                      hasTTL() const;
      private:
	int64_t                   intAsInt64() const;
	bool                      intAsBool() const;
	double                    intAsDouble() const;
	float                     intAsFloat() const;
	int                       intAsInt() const;
	boost::posix_time::ptime  intAsDateTime(TimeAdjust ta) const;
	
	const ::org::apache::cassandra::Column& m_Column;
	ColumnDataType                          m_Cdt;
	
	friend class CQLQueryResult;
    };

    ~CQLQueryResult();

    bool wasVoidReturn() const;
    bool wasIntReturn() const;
    bool wasRowsReturn() const;
    bool hasColumns() const;
    size_t getColumnCount() const;
    size_t getRowCount() const;
    ColumnDataType getColumnType(size_t colIdx) const;
    std::string& getColumnName(size_t colIdx) const;
    size_t indexOfColumnName(const std::string& colName) const;
    const CQLColumnValue get(size_t rowIdx, size_t colIdx) const;
  
    //static std::string escapeString(const std::string& s, bool quote = true);
    //static std::string blobToHexString(const std::string& blob, bool quote = true);
    //static std::string ptimeToString(const boost::posix_time::ptime& t, bool quote = true, TimeAdjust ta=UTCTIME);
  private:
    CQLQueryResult();
    bool parse(ErrorFunction errorFunc);
    ::org::apache::cassandra::CqlResult* m_Result;
    std::vector<ColumnDataType>          m_ColDataTypes;
    
    static std::map<std::string, ColumnDataType> s_DataStringToType;
    
    friend class CassandraConnection;
};

} //namespace teamspeak
} //namespace agamemnon

#endif // AGCQLQUERYRESULT_H
