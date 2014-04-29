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
#include <iostream>

#include "Msg_AC.h"
#include "Network.h"
#include "Server.h"
#include "Client.h"
#include "debug.h"

using std::string;
using std::cout;
using std::endl;

namespace eNetworks
{

void Msg_AC::Parser()
{
   	if (!Source.IsServer())
   	{
   	   debug << "Protocol Error: Client sending AC Token" << endb;
   	}
   	
   	Client* l_Client = Network::Interface.FindClientByNumeric(Parameters[0]);
   	if (NULL == l_Client)
   	{
   	   debug << "Error: Could not find client with Token (" << Parameters[0] << ") in database from AC Token MSG" << endb;
   	} 

   	l_Client->SetAccount(Parameters[1]);
   	l_Client->AddMode('r');
}


} // namespace eNetworks
