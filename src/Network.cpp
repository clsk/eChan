/*
 * eChan - Electronic Channel Services.
 * Copyright (C) 2003-2006 Alan Alvarez.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 *
*/

#include <map>
#include <string>
#include <iostream>
#include <cstdlib>

#include "Client.h"
#include "Network.h"
#include "Server.h"
#include "Channel.h"
#include "ConfigParser.h"
#include "tools.h"
#include "Socket.h"
#include "SocketException.h"
#include "debug.h"
#include "ConfigParser.h"

using std::map;
using std::string;
using std::cout;
using std::endl;

namespace eNetworks
{

Network::Network() : Servers(), ClientNumerics(), ClientNickNames(), Channels() 
{
   if (!ConfigFile.IsParsed())
   	return;

   LocalServer = new Server(ConfigFile.GetConfiguration("NUMERIC"), ConfigFile.GetConfiguration("SERVERNAME"), "", 
   	   	   	    ConfigFile.GetConfiguration("SERVERINFO"), time(0), time(0), 1, 's');

   if (LocalServer == NULL)
   {
   	debug << "Could not create server: " << ConfigFile.GetConfiguration("SERVERNAME") << endb;
   	exit(0);
   }

   try
   {
   	Socket::eSock.connect(ConfigFile.GetConfiguration("UPLINK"), StringToInt(ConfigFile.GetConfiguration("PORT")));
   }
   catch (SocketException &sockerr)
   {
   	sockerr.log();
   	sockerr.fix();
   }
}



// -----------------------------
//  Servers Management members.
// -----------------------------

bool Network::AddServer(const std::string &aNumeric, const std::string &aName, const std::string &aUpLinkNumeric,
               const std::string &aDescription, const time_t &aStartTime, const time_t &aLinkTime,
               const unsigned int &aHopCount, const char &aFlag)
{
   // Check we don't get bogus entries. If we do return false.
   if (aUpLinkNumeric.length() > 2 || aUpLinkNumeric.find(" ") != string::npos ||
       aName.find(" ") != string::npos || aNumeric.length() > 2 ||
       aNumeric.find(" ") != string::npos || aHopCount == 0 ||
       FindServerByNumeric(aNumeric) != NULL)
   {
        return false;
   }

   // Allocate a new server on the heap.
   Server *NewServer = new Server(aNumeric, aName, aUpLinkNumeric, aDescription, aStartTime, aLinkTime, aHopCount, aFlag);

   // If server could not be allocated on the heap OR there was a problem inserting the server in the
   // Server map table, return false.
   if (NewServer == NULL)
   {
   	debug << "Could not allocate memory for server..." << endb;
   	return false;
   }

   if (Servers.insert(ServerMapType::value_type(aNumeric, NewServer)).second == false)
   {
   	debug << "Could not add server to ServerMapType..." << endb;
   	delete NewServer;
   	return false;
   }

   // Local server is not on ServerMap
   if (LocalServer->GetNumeric() != aUpLinkNumeric)
   {
   	// Add this server to the DownLinks list of its uplink.
   	FindServerByNumeric(aUpLinkNumeric)->ServerDownLinks.push_back(aNumeric);
   }

// everything went well.
return true;
}

bool Network::DelServerByNumeric(const std::string &aNumeric)
{
   // try to find a server with numeric: aNumeric.
   ServerMapType::iterator ServerIter = Servers.find(aNumeric);

   // if none was found return false.
   if (ServerIter == Servers.end())
   {
   	return false;
   }

   // Delete clients that are in this server.
   for (Server::ServerClientsIterator i = ServerIter->second->ServerClients.begin(); 
        i != ServerIter->second->ServerClients.end(); i++)
   {
   	DelClientByNumeric(i->second->Numeric);
   }

   // Delete servers that are linked to this server.
   for (Server::ServerDownLinksIterator i = ServerIter->second->ServerDownLinks.begin(); i != ServerIter->second->ServerDownLinks.end(); i++)
   {
   	DelServerByNumeric(*i);
   }

   // delete the Server from memory.
   delete ServerIter->second;
   // then finally remove the server from the Server Map table.
   Servers.erase(ServerIter); 

return true;
}

bool Network::DelServerByName(const std::string &aName)
{
   // Search server by server name then call DelServerByNumeric.
   // DelServerByNumeric is faster than this.
   for (ServerMapType::iterator i = Servers.begin(); i != Servers.end(); i++)
   {
   	if (i->second->Name == aName)
   	{
   	   DelServerByNumeric(i->second->Numeric);
   	   return true;
   	}
   }

return false;
}

Server *Network::FindServerByNumeric(const std::string &aNumeric)
{
   ServerMapType::iterator ServerIter = Servers.find(aNumeric);

   if (ServerIter == Servers.end())
   {
   	return NULL;
   }
   // else
   return ServerIter->second;
}

Server *Network::FindServerByName(const std::string &aName)
{
   // Search server by server to see if the server name matches.
   // NOTE: FindServerByNumeric is faster than this.
   for (ServerMapType::iterator i = Servers.begin(); i != Servers.end(); i++)
   {
   	if (i->second->Name == aName)
   	{
   	   return i->second;
   	}
   }

return NULL;
}

// -----------------------------
//  Clients Management members.
// -----------------------------

bool Network::AddClient(const std::string &aNumeric, const std::string &aNickName, const std::string &aAccount,
                        const std::string &aUserName, const std::string &aHostName, const std::string &aB64IP,
                        const std::string &aModes, const std::string &aUserInfo, const time_t &aTimeStamp,
                        const unsigned int &aHopCount)
{

   // Check we don't get bogus entries.
   if (aNickName.length() == 0 || aNickName.find(" ") != string::npos || aNumeric.length() != 5 ||
       aUserName.length() == 0 || aUserName.find(" ") != string::npos ||
       aHostName.length() == 0 || aHostName.find(" ") != string::npos || aB64IP.length() == 0 ||
       aB64IP.find(" ") != string::npos || FindServerByNumeric(aNumeric.substr(0,2)) == NULL ||
       aTimeStamp < FindServerByNumeric(aNumeric.substr(0,2))->StartTime ||
       FindClientByNickName(aNickName) != NULL || FindClientByNumeric(aNumeric) != NULL)
   {
        return false;
   }


   // create the new client. Allocate memory for it on the heap.
   Client *NewClient = new Client(aNumeric, aNickName, aAccount, aUserName, aHostName, aB64IP, aModes, aUserInfo, 
                                  aTimeStamp, aHopCount);


   // if we couldn't allocate the new client or we can't insert the new user to the client map table, return false.
   if (NewClient == NULL || !ClientNumerics.insert(ClientNumericsMapType::value_type(aNumeric, NewClient)).second)
   {
   	delete NewClient;
   	return false;
   }

   // try to insert the new client to the user map table associated by its nicknames, if we can't delete the client,
   // remove the client from the user map table associated by nicknames and return false.
   if(ClientNickNames.insert(ClientNickNamesMapType::value_type(aNickName, NewClient)).second == false)
   {
   	ClientNumerics.erase(aNumeric);
   	delete NewClient;
   	return false;
   }

   // Make this user dependent of its server.
   FindServerByNumeric(aNumeric.substr(0,2))->ServerClients.insert(Server::ServerClientsType::value_type(aNumeric.substr(2),
                                                                                                         NewClient));

return true;
}

bool Network::DelClientByNickName(const std::string &aNickName)
{
   // Try to find the client.
   ClientNickNamesMapType::iterator ClientIter = ClientNickNames.find(aNickName);

   // return false if client with this nicname doesn't exist.
   if (ClientIter == ClientNickNames.end())
   	return false;

   // try to remove from the table associated by numerics first.
   if (ClientNumerics.erase(ClientIter->second->Numeric) != 1)
   	return false;

   // then remove the client from the client table in the server this client is dependant on.
   FindServerByNumeric(ClientIter->second->Numeric.substr(0,2))->ServerClients.erase(ClientIter->second->Numeric.substr(2));

   // remove this client from every channel it is on.
   ClientIter->second->ClearChannels();

   // finally release memory allocated and remove from table associated by nicknames.
   delete ClientIter->second;
   ClientNickNames.erase(ClientIter);

   return true;
}

bool Network::DelClientByNumeric(const std::string &aNumeric)
{
   // try to find the client.
   ClientNumericsMapType::iterator ClientIter = ClientNumerics.find(aNumeric);

   // if client doesn't exist return false.
   if (ClientIter == ClientNumerics.end())
        return false;

   // remove this client from every channel it is on.
   ClientIter->second->ClearChannels();

   // try to erase the client from the client table associated by nicknames.
   if (ClientNickNames.erase(ClientIter->second->NickName) != 1)
        return false;


   // then remove the client from the client table in the server this client is dependant on.
   FindServerByNumeric(aNumeric.substr(0,2))->ServerClients.erase(aNumeric.substr(2));

   // finally release memory allocated and remove from table associated by numerics.
   delete ClientIter->second;
   ClientNumerics.erase(ClientIter);

return true;
}

Client *Network::FindClientByNickName(const std::string &aNickName)
{

   ClientNickNamesMapType::iterator ClientIter = ClientNickNames.find(aNickName);

   if (ClientIter == ClientNickNames.end())
   	return NULL;

   return ClientIter->second;
}

Client *Network::FindClientByNumeric(const std::string &aNumeric)
{
   ClientNumericsMapType::iterator ClientIter = ClientNumerics.find(aNumeric);

   if (ClientIter == ClientNumerics.end())
   	return NULL;

   return ClientIter->second;
}


// -----------------------------
//  Channels Management members.
// -----------------------------

bool Network::AddChannel (const std::string &aName, const time_t &aTimeStamp, const std::string &aTopic, 
   	   	   	 const std::string &aModes, const std::string &aKey, const unsigned int &aLimit)
{
   // check we don't get bogus entries.
   if (aName.length() == 0 || (aName[0] != '#' && aName[0] != '+'))
    return false;

   // allocate memory for this channel.
   Channel *NewChannel = new Channel(aName, aTopic, aModes, aKey, aLimit, aTimeStamp);

   // return false if NewChannel is null (which means no memory could be allocated) or if for some reason
   // we can't add it to the channel table.
   if (NewChannel == NULL || !Channels.insert(ChannelMapType::value_type(aName, NewChannel)).second)
   {
   	delete NewChannel;
   	return false;
   }

   return true;
}

bool Network::DelChannel(const std::string &aName)
{
   return Channels.erase(aName) ? true : false;
}

Channel *Network::FindChannel(const std::string &aName)
{
   ChannelMapType::iterator ChannelIter = Channels.find(aName);  

   if (ChannelIter == Channels.end())
    return NULL;

   return ChannelIter->second;
}

Network Network::Interface;

} // namespace eNetwork
