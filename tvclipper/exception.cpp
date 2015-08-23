/*  tvclipper
    Copyright (c) 2007 Sven Over <svenover@svenover.de>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* $Id: exception.cpp 58 2007-07-23 07:03:07Z too-tired $ */

#include "exception.h"
#include <qstring.h>
#include <qmessagebox.h>

tvclipper_exception::tvclipper_exception(const std::string &__arg) : _M_msg(__arg), _M_extype()
{
}

tvclipper_exception::tvclipper_exception(const char* __arg) : _M_msg(__arg), _M_extype()
{
}

tvclipper_exception::~tvclipper_exception() throw()
{
}

const char *tvclipper_exception::what() const throw()
{
  return _M_msg.c_str();
}

void tvclipper_exception::show() const
{
  std::string extype(type());

  if (extype.empty())  
    extype="TVCLIPPER error";

  QMessageBox::critical(NULL,QString::fromStdString(extype),what(),QMessageBox::Abort,QMessageBox::NoButton);
}