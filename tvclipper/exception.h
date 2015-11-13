/*  dvbcut
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

 *  tvclipper
    Copyright (c) 2015 Lukáš Vlček

    This file is part of TV Clipper.

    TV Clipper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TV Clipper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TV Clipper. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _TVCLIPPER_EXCEPTION_H
#define _TVCLIPPER_EXCEPTION_H

#include <exception>
#include <string>

class tvclipper_exception : public std::exception
{
protected:
  std::string _M_msg;
  std::string _M_extype;
public:
  explicit tvclipper_exception(const std::string &__arg);
  explicit tvclipper_exception(const char* __arg);
  virtual ~tvclipper_exception() throw();
  
  virtual const char *what() const throw();
  
  const std::string &msg() const throw() { return _M_msg; }
  const std::string &type() const throw() { return _M_extype; }
  
  void show() const;
};

#endif // ifndef _TVCLIPPER_EXCEPTION_H
