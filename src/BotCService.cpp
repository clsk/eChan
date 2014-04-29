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

#include <iostream>
#include <mysql++.h>

#include "BotCService.h"
#include "Network.h"
#include "MsgMonitor.h"
#include "P10Tokens.h"
#include "Server.h"
#include "Channel.h"
#include "Command.h"
#include "CommandWHOIS.h"
#include "SqlManager.h"
#include "ConfigParser.h"
#include "Client.h"
#include "debug.h"
#include "SQL.h"
#include "SqlChannelAccess.h"

using std::cout;
using std::endl;
using mysqlpp::Query;
using mysqlpp::Result;
using eNetworks::cservice::SQL;
using eNetworks::cservice::SqlChannelAccess;

namespace eNetworks
{
BotCService::BotCService() : 
   	     Bot(ConfigFile.GetConfiguration("NUMERIC")+"AAA", ConfigFile.GetConfiguration("NICK"), "", 
                 ConfigFile.GetConfiguration("USERNAME"), ConfigFile.GetConfiguration("HOSTNAME"), 
    	   	 "DAqAoB", "idk", ConfigFile.GetConfiguration("CLIENTINFO"))
{
   MsgMonitor::AddMonitor(Tokens::END_OF_BURST, this);
   MsgMonitor::AddMonitor(Tokens::JOIN, this);

   cservice::Command::AddCommand("whois", cservice::Command::WHOIS);
   cservice::Command::AddCommand("quote", cservice::Command::QUOTE);
   cservice::Command::AddCommand("login", cservice::Command::LOGIN);
   cservice::Command::AddCommand("up", cservice::Command::UP);
   cservice::Command::AddCommand("down", cservice::Command::DOWN);
   cservice::Command::AddCommand("op", cservice::Command::OP);
   cservice::Command::AddCommand("deop", cservice::Command::DEOP);
   cservice::Command::AddCommand("voice", cservice::Command::VOICE);
   cservice::Command::AddCommand("devoice", cservice::Command::DEVOICE);
   cservice::Command::AddCommand("hello", cservice::Command::HELLO);
   cservice::Command::AddCommand("register", cservice::Command::REGISTER);
   cservice::Command::AddCommand("adduser", cservice::Command::ADDUSER);
   cservice::Command::AddCommand("modinfo", cservice::Command::MODINFO);
   cservice::Command::AddCommand("remuser", cservice::Command::REMUSER);
   cservice::Command::AddCommand("purge", cservice::Command::PURGE);
   cservice::Command::AddCommand("die", cservice::Command::DIE);
   cservice::Command::AddCommand("access", cservice::Command::ACCESS);
}

void BotCService::onPRIVMSG(const MsgSource& Source, const MsgTokenizer& Parameters)
{
   cout << "[IN]: " << Source.GetName() << " PRIVMSG " << theClient.GetNickName() << " :";
   for (unsigned int i = 0; i < Parameters.size(); i++)
   	cout << Parameters[i] << " ";

   cout << endl;

   std::string Message = "";

   for (unsigned int i = 0; i < Parameters.size(); i++)
   {
   	Message += Parameters[i];
   	Message += " ";
   }

   // Reply to CTCP messages.
   if (Parameters[Parameters.size()-1] == "\001VERSION\001")
   {
   	SendNotice(Network::Interface.FindClientByNumeric(Source.GetNumeric()), "\001VERSION Electronic Channel Services (eChan) v0.5.2 Alpha by Alan Alvarez (clsk@IRC).\001");
   	return;
   }

   Message = Message.erase(Message.length()-1, 1); // get rid of extra space character.
   cservice::Command::ParseCommands(this, Network::Interface.FindClientByNumeric(Source.GetNumeric()), Message);
}

void BotCService::onNOTICE(const MsgSource& Source, const MsgTokenizer& Parameters)
{
   cout << "[IN]: " << Source.GetName() << " NOTICE " << theClient.GetNickName() << " :";
   for (unsigned int i = 0; i < Parameters.size(); i++)
   	cout << Parameters[i] << " " << endl;

   cout << endl;
}

// We shouldn't be getting kicked in the first place.
void BotCService::onKICK(const MsgSource& Source, const MsgTokenizer& Parameters)
{
}

void BotCService::onMsgMonitor(const Tokens::Token& _Token, const MsgSource& Source, const MsgTokenizer& Parameters)
{
   switch(_Token)
   {
   	case Tokens::END_OF_BURST:
   	{
   	   if (Network::Interface.FindServerByNumeric(Source.GetNumeric())->GetUpLink() != LocalServer->GetNumeric())
   	   	return;

   	   // Lets JOIN all registered channels.
   	   Query query = SqlManager::query();
   	   query << "SELECT SqlChannel.name FROM SqlChannel";

   	   // TODO: Use a ResUse container here instead of Result.
   	   Result result = query.store();

   	   for (Result::iterator Iter = result.begin(); Iter != result.end(); Iter++)
   	   {
   	   	string strChannel = (*Iter).at(0).c_str();
   	   	if (strChannel[0] != '#') // Check for special channels.
   	   	   continue;

   	   	Join(strChannel);
   	   	// OP the bot.
   	   	RawMsg(theClient.GetNumeric().substr(0,2) + " M " + strChannel + " +o " + theClient.GetNumeric());
   	   }
   	}
   	break;

   	case Tokens::JOIN:
   	{
   	   Client* l_client = Source.GetClient();
   	   Channel* l_channel = Network::Interface.FindChannel(Parameters[0]);
   	   if (l_channel->IsChannelClient(&theClient))
   	   {
   	   	if (l_client->IsLogged())
   	   	{
   	   	   SqlChannelAccess* l_SqlChannelAccess = SQL::Interface.FindChannelAccess((unsigned int)l_client->GetID(), Parameters[0]);
   	   	   if (l_SqlChannelAccess == NULL)
   	   	   	return;

   	   	   switch(l_SqlChannelAccess->getAutomode())
   	   	   {
   	   	   	case SqlChannelAccess::AUTOMODE_OP:
   	   	   	   GiveOP(Parameters[0], l_client->GetNickName());
   	   	   	   break;

   	   	   	case SqlChannelAccess::AUTOMODE_VOICE:
   	   	   	   GiveVoice(Parameters[0], l_client->GetNickName());
   	   	   	   break;
   	   	   }
   	   	}
   	   }
   	}
   	break;

   	default:
   	   debug << "WARNING: Got Monitored not requested message in BotCService.cpp::onMsgMonitor()" << endb;
   }
}

} // namespace eNetworks
