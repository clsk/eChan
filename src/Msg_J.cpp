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

#include "Msg_J.h"
#include "Msg_C.h"
#include "debug.h"
#include "Network.h"
#include "Channel.h"

using std::cout;
using std::endl;

namespace eNetworks
{

void Msg_J::Parser()
{
   if (!Source.IsClient())
   {
   	debug << "JOIN Message with no Client source." << endb;
   }


   if ("0" == Parameters[0])
   {
   	Network::Interface.FindClientByNumeric(Source.GetNumeric())->ClearChannels();
   	return;
   }
   	
   Channel* aChannelPtr = Network::Interface.FindChannel(Parameters[0]);

   if (NULL != aChannelPtr)
   {
   	if (!aChannelPtr->AddChannelClient(Network::Interface.FindClientByNumeric(Source.GetNumeric())))
   	{
   	   debug << "Could not join " << Source.GetName() << " to channel " << Parameters[0] << "." << endb;
   	}
/*
   	else
   	{
   	   cout << "User " << Source.GetName() << " Joined channel " << Parameters[0] << "." << endl;
   	}
*/
   }
   else
   {
   	// Treat this as a CREATE Msg.
   	Msg_C aMsg(Source, Parameters);
   	aMsg.Parser();
   }

return;
}


} // namespace eNetworks
