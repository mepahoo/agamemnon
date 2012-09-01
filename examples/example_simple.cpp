#include "../agamemnon/singleconnectionfactory.h"
#include "../agamemnon/client.h"
#include "../agamemnon/agcqlquery.h"
#include <boost/asio.hpp>
#include <boost/assign.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <sstream>

using namespace teamspeak::agamemnon;
using namespace std;

void statementPrepared(Client::Ptr client, size_t cqlIdx, PreparedQuery::Ptr query); //forward decleration

boost::asio::io_service IOService;

const std::vector<std::string> CQL_Statements = boost::assign::list_of
 ("CREATE KEYSPACE agtest WITH strategy_class = 'SimpleStrategy' AND strategy_options:replication_factor = 1;")
 ("CREATE TABLE agtest.test( vascii ascii, vbigint bigint, vblob blob, vboolean boolean, vdouble double, vfloat float, vint int, vtext text, vtimestamp timestamp, vuuid uuid, vvarchar varchar, PRIMARY KEY(vascii));")
 ("INSERT INTO agtest.test(vascii, vbigint, vblob, vboolean, vdouble, vfloat, vint, vtext, vtimestamp, vuuid, vvarchar) values ('hello ascii', 10000000000, '0123456789ABCDEF', 'true', 21.345, 12.3, 1000000000, 'text', '2012-08-22T21:53:01', c42b94bd-63b4-4de5-b4f9-1b7b663ea2d7, 'varchar') USING TTL 3600;")
 ("INSERT INTO agtest.test(vascii, vtext) values ('keynr2', '');")
 ("INSERT INTO agtest.test(vascii, vbigint, vblob, vboolean, vdouble, vfloat, vint, vtext, vtimestamp, vuuid, vvarchar) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) ;")
 ("INSERT INTO agtest.test(vascii, vvarchar, vblob, vboolean, vtimestamp, vuuid) values ('keynr3',!) USING TIMESTAMP !;")
 ("SELECT vascii, vbigint, vblob, vboolean, vdouble, vfloat, vint, vtext, vtimestamp, vuuid, vvarchar, writetime(vvarchar), TTL(vvarchar) FROM agtest.test;")
 ("DROP KEYSPACE agtest;");
 
std::string createInsertValues()
{
  //vvarchar, vblob, vboolean, vtimestamp, vuuid
  std::stringstream ss;
  ss << CQLQuery::escapeString("this's is some'' text ro\xC3\x9F""e") <<","
     << CQLQuery::blobToHexString(" abcd ") << ","
     << CQLQuery::booleanToString(true) <<","
     << CQLQuery::ptimeToString(boost::posix_time::microsec_clock::universal_time()) <<","
     << CQLQuery::bytesStringToUUID("0123456789ABCDEF");
  return ss.str();
}

size_t utf8CharCount(const string& s)
{
  size_t r = 0;
  for (size_t i=0; i<s.size(); ++i)
  {
    unsigned char uc = static_cast<unsigned char>(s[i]);
    if ((uc & 0xc0) != 0x80) ++r;
  }
  return r;
}
 
void errorFunction(const Error& error)
{
  cerr << error.message << endl;
  IOService.stop();
}

void getDataDone(CQLQueryResult::Ptr cqlresult, Client::Ptr client, size_t cqlIdx)
{
  std::cout <<"OK"<<std::endl;
  if (cqlIdx==CQL_Statements.size()-1)
  { 
    IOService.stop();
    return;
  }
  
  if (cqlresult->hasColumns())
  {
    std::vector<int> maxSize;
    maxSize.resize(cqlresult->getColumnCount());
    
    for (size_t i=0; i < cqlresult->getColumnCount(); ++i)
    {
      /*calc max col size*/
      maxSize[i] = std::min<size_t>(36, utf8CharCount(cqlresult->getColumnName(i)));
      for (size_t j=0; j <cqlresult->getRowCount(); ++j) 
      {
	maxSize[i] =std::min<size_t>(std::max<size_t>(maxSize[i], utf8CharCount(cqlresult->get(j,i).asString())), 36);
      }
      
      /*write the header*/
      if (i) 
      {
	cout << "|";
      }
      
      std::cout.width(maxSize[i]);
      std::cout << std::left << cqlresult->getColumnName(i);
    }
    std::cout <<std::endl;
    
    /*write header line*/
    std::cout.fill('-');
    for (size_t j=0; j < cqlresult->getColumnCount(); ++j)
    {
      if (j) std::cout <<"+";
      std::cout.width(maxSize[j]);
      std::cout<<"-";
    }
    std::cout<<std::endl;
    
    /*write selected data*/
    std::cout.fill(' ');
    for (size_t i=0; i <cqlresult->getRowCount(); ++i) 
    {
      for (size_t j=0; j < cqlresult->getColumnCount(); ++j)
      {
	if (j) std::cout<<"|";
	std::cout.width(maxSize[j]);
	std::cout<<cqlresult->get(i,j).asString();
      }
      std::cout << std::endl;
    } 
  }
  
  /*Handle next sql statement*/
  cqlIdx++;
  
  std::string cqlStatement = CQL_Statements[cqlIdx];

  size_t pos;
  /*check for prepared statement*/
  pos = cqlStatement.find('?');
  if (pos != string::npos) {
    std::cout << "prepare: "<<cqlStatement<<std::endl;
    client->prepareQuery(cqlStatement, errorFunction, boost::bind(statementPrepared, client, cqlIdx, _1));
    return;
  }
  

  /* do some replacing */
  pos = cqlStatement.find('!');
  if (pos != string::npos) {
    cqlStatement.replace(pos, 1, createInsertValues());
    pos = cqlStatement.find('!');
    if (pos != string::npos) cqlStatement.replace(pos, 1, CQLQuery::getTimeStamp());
  }
  
  /*execute*/
  std::cout << "exec: "<<cqlStatement<<std::endl;
  client->excecuteCQL(cqlStatement, errorFunction, boost::bind(getDataDone, _1, client, cqlIdx));
}

void statementPrepared(Client::Ptr client, size_t cqlIdx, PreparedQuery::Ptr query)
{
  std::cout <<"Execute prepared statement"<<std::endl;
 //(vascii, vbigint, vblob, vboolean, vdouble, vfloat, vint, vtext, vtimestamp, vuuid, vvarchar) values 
 //('hello ascii', 10000000000, '0123456789ABCDEF', 'true', 21.345, 12.3, 1000000000, 'text', '2012-08-22T21:53:01', c42b94bd-63b4-4de5-b4f9-1b7b663ea2d7, 'varchar') USING TTL 3600;")
  query->set("vascii", "hello asc1");
  query->set("vbigint", static_cast<int64_t>(10000000001ll));
  query->set("vblob", Bytes("\x01\x23\x45\x67\x89\xAB\xCD\xE0")); //don't realy need to convert to bytes here.. just for demo purposes
  query->set("vboolean", false);
  query->set("vdouble", 21.346);
  query->set("vfloat",  12.4f);
  query->set("vint", 1000000001);
  query->set("vtext", "tex1");
  query->set("vtimestamp", boost::posix_time::time_from_string("2012-08-22 21:53:02"));
  query->set("vuuid", UUID("c42b94bd-63b4-4de5-b4f9-1b7b663ea2d8"));
  query->set("vvarchar", "varcha1");
  
  client->executePreparedQuery(query, errorFunction, boost::bind(getDataDone, _1, client, cqlIdx));
}

void getClusterNameDone(Client::Ptr client, const std::string& clustername)
{
  cout << "Clustername of localhost is: " << clustername << endl;
  
  /*execute first cql statement*/
  std::cout << "exec: "<<CQL_Statements[0]<<std::endl;
  client->excecuteCQL(CQL_Statements[0], errorFunction, boost::bind(getDataDone, _1, client, 0));
}

int main(int argc, const char* argv[] )
{
      ConnectionCommonSettings connectionCommonSettings;
      
      ConnectionFactory::Ptr connectionFactory(new SingleConnectionFactory(IOService, "127.0.0.1", connectionCommonSettings));
      Client::Ptr client( new Client(IOService, connectionFactory));
 
      client->getClusterName(errorFunction, boost::bind(&getClusterNameDone, client, _1));
      
      IOService.run();
      
      return 0;
}     