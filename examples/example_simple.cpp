#include "../agamemnon/singleconnectionfactory.h"
#include "../agamemnon/client.h"
#include <boost/asio.hpp>
#include <boost/assign.hpp>

#include <iostream>

using namespace teamspeak::agamemnon;
using namespace std;

boost::asio::io_service IOService;

const std::vector<std::string> CQL_Statements = boost::assign::list_of
 ("CREATE KEYSPACE agtest WITH strategy_class = 'SimpleStrategy' AND strategy_options:replication_factor = 1;")
 ("CREATE TABLE agtest.test( vascii ascii, vbigint bigint, vblob blob, vboolean boolean, vdouble double, vfloat float, vint int, vtext text, vtimestamp timestamp, vuuid uuid, vvarchar varchar, PRIMARY KEY(vascii));")
 ("INSERT INTO agtest.test(vascii, vbigint, vblob, vboolean, vdouble, vfloat, vint, vtext, vtimestamp, vuuid, vvarchar) values ('hello ascii', 10000000000, '0123456789ABCDEF', 'true', 21.345, 12.3, 1000000000, 'text', '2012-08-22T21:53:01', c42b94bd-63b4-4de5-b4f9-1b7b663ea2d7, 'varchar') USING TTL 3600;")
 ("INSERT INTO agtest.test(vascii,vtext) values ('keynr2', '');")
 ("SELECT vascii, vbigint, vblob, vboolean, vdouble, vfloat, vint, vtext, vtimestamp, vuuid, vvarchar, writetime(vvarchar), TTL(vvarchar) FROM agtest.test;")
 ("DROP KEYSPACE agtest;");
 
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
      maxSize[i] = std::min<size_t>(36,cqlresult->getColumnName(i).size());
      for (size_t j=0; j <cqlresult->getRowCount(); ++j) 
      {
	maxSize[i] =std::min<size_t>(std::max<size_t>(maxSize[i], cqlresult->get(j,i).asString().size()), 36);
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
  
  std::cout << "exec: "<<CQL_Statements[cqlIdx]<<std::endl;
  client->excecuteCQL(CQL_Statements[cqlIdx], errorFunction, boost::bind(getDataDone, _1, client, cqlIdx));
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