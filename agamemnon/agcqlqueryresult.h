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
    ConversionException(const std::string& msg) : std::exception(), m_Msg(msg){};
    virtual ~ConversionException() throw() {};
    virtual const char* what() const throw() { return m_Msg.c_str(); }
  private:
    std::string m_Msg;
};

class IndexOutOfRange : public std::exception
{
  public:
    IndexOutOfRange() : std::exception(){};
    virtual ~IndexOutOfRange() throw() {};
    virtual const char* what() const throw() { return "Index out of range"; }
};

class CQLQueryResult
{
  public:
    typedef boost::shared_ptr<CQLQueryResult> Ptr;
    
    enum ColumDataType {UNKNOWN, ASCII, BIGINT, BLOB, BOOLEAN, COUNTER, DECIMAL, DOUBLE, FLOAT, INT, TEXT, TIMESTAMP, UUID, VARCHAR, VARINT};
    
    class CQLColumnValue
    {
      public:
	CQLColumnValue(const ::org::apache::cassandra::Column& column, ColumDataType cdt);
	const std::string&        asString() const;
	std::string               asBlob() const;
	std::string               asUUID() const;
	int64_t                   asInt64() const;
	bool                      asBool() const;
	double                    asDouble() const;
	float                     asFloat() const;
	int                       asInt() const;
	boost::posix_time::ptime  asDataTime() const;
	//virtual --                asDecimal() const;
      private:
	const ::org::apache::cassandra::Column& m_Column;
	ColumDataType                           m_cdt;
	
	friend class CQLQueryResult;
    };

    ~CQLQueryResult();

    bool hasColumns() const;
    int getColumnCount() const;
    int getRowCount() const;
    ColumDataType getColumnType(int colIdx) const;
    std::string& getColumnName(int colIdx) const;
    const CQLColumnValue get(int rowIdx, int colIdx) const;
  
  private:
    CQLQueryResult();
    bool parse(ErrorFunction errorFunc);
    ::org::apache::cassandra::CqlResult* m_Result;
    std::vector<ColumDataType>           m_ColDataTypes;
    
    friend class CassandraConnection;
};

} //namespace teamspeak
} //namespace agamemnon

#endif // AGCQLQUERYRESULT_H
