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

#include <utility>

#include "MsgMonitor.h"

namespace eNetworks
{
void MsgMonitor::AddMonitor(const Tokens::Token& _Token, Bot* _Bot)
{
   mList.insert(MonitorListType::value_type(_Token, _Bot));
}

void MsgMonitor::NotifyMonitors(const Tokens::Token& _Token, const MsgSource& Source, const MsgTokenizer& Parameters)
{
   MonitorListType::iterator l_Iter = mList.find(_Token);
   if (l_Iter != mList.end())
   	l_Iter->second->onMsgMonitor(_Token, Source, Parameters);
}

std::map<Tokens::Token, Bot*> MsgMonitor::mList = MonitorListType();

} // namespace eNetworks
