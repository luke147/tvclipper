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

#include "exception.h"
#include <QObject>
#include <QMessageBox>
#include <QDebug>

tvclipper_exception::tvclipper_exception(const QString &__arg) : _M_msg(__arg), _M_extype()
{
}

tvclipper_exception::tvclipper_exception(const tvclipper_exception &e)
{
    this->_M_extype = e.type();
    this->_M_msg = e.msg();
}
/*
tvclipper_exception::tvclipper_exception(const char* __arg) : _M_msg(__arg), _M_extype()
{
}
*/
tvclipper_exception::~tvclipper_exception() throw()
{
}

const char* tvclipper_exception::what() const throw()
{
    return _M_msg.toStdString().c_str();
}

void tvclipper_exception::raise() const {
    throw *this;
}

const QString &tvclipper_exception::msg() const throw()
{
    return _M_msg;
}

const QString &tvclipper_exception::type() const throw()
{
    return _M_extype;
}

void tvclipper_exception::show() const
{
    QString extype = type();

    if (extype.isEmpty())
        extype = QObject::tr("TVCLIPPER error");

    QString text = what();
    qCritical() << extype << ": " << text;
    QMessageBox::critical(NULL, extype, text, QMessageBox::Close, QMessageBox::Close);
}

tvclipper_exception *tvclipper_exception::clone() const {
    return new tvclipper_exception(*this);
}
