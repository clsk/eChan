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

#include <string>

#include "Client.h"
#include "Channel.h"
#include "debug.h"

using std::string;

namespace eNetworks
{

bool Client::AddChannel(Channel *aChannelPtr)
{
   if (aChannelPtr == NULL)
    return false;

   return ChannelMap.insert(ChannelMapType::value_type(aChannelPtr->GetName(), aChannelPtr)).second;
}

bool Client::DelChannel(Channel *aChannelPtr)
{
   if (aChannelPtr == NULL)
    return false;

   if (ChannelMap.erase(aChannelPtr->GetName()) != 1)
    return false;
   // else
   return true;
}

void Client::ClearChannels()
{

   for (ChannelMapType::iterator ChannelIter = ChannelMap.begin(); 
	   ChannelIter != ChannelMap.end(); 
	   ChannelIter++)
   {
   	
	   string name = ChannelIter->first;
	   if (!ChannelIter->second->DelChannelClient(this))
	   {
		   debug << "Could not Delete channel " << name << " off client " << NickName << " in Client::ClearChannels()." << endb;
	   }

	   // DelChannelClient will delete the pointer to 
	   // ChannelClient making it a bad pointer.
	   ChannelIter = ChannelMap.begin();
	   if ( ChannelIter == ChannelMap.end() )
		   break;
   }
}

} // namespace eNetworks
