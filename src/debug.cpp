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

#include <fstream>
#include <string>
#include <iostream>

#include "debug.h"
#include "tools.h"

using std::cout;
using std::ofstream;
using std::string;
using std::ios;

namespace eNetworks
{

const Debug &Debug::operator <<(const string &msg) const
{
  ofstream ofs("debug.log", ios::out | ios::app);
  ofs << msg;
  cout << msg;

  return *this;
}

const Debug &Debug::operator <<(const int &msg) const
{
   ofstream ofs("debug.log", ios::out | ios::app);
   ofs << IntToString(msg);
   cout << msg;

   return *this;
}

Debug debug;
// bool DEBUG = false;

} // namespace eNetworks
