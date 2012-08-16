#include "../agamemnon/singleconnectionfactory.h"
#include "../agamemnon/client.h"
#include <boost/asio.hpp>

#include <iostream>

using namespace teamspeak::agamemnon;
using namespace std;

boost::asio::io_service IOService;

void errorFunction(const Error& error)
{
  cerr << error.message << endl;
  IOService.stop();
}

void getClusterNameDone(const std::string& clustername)
{
  cout << "Clustername of localhost is: " << clustername << endl;
  IOService.stop();
}

int main(int argc, const char* argv[] )
{

      ConnectionCommonSettings connectionCommonSettings;
      ConnectionFactory::Ptr connectionFactory(new SingleConnectionFactory(IOService, "127.0.0.1", connectionCommonSettings));
      Client::Ptr client( new Client(IOService, connectionFactory));
 
      client->getClusterName(errorFunction, getClusterNameDone);
      
      IOService.run();
      
      return 0;
}     