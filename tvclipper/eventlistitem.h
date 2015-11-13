/*  dvbcut
    Copyright (c) 2005 Sven Over <svenover@svenover.de>

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

#ifndef _TVCLIPPER_EVENTLISTIEM_H_
#define _TVCLIPPER_EVENTLISTIEM_H_

#include <QPixmap>
#include <QListWidget>
#include <QListWidgetItem>
#include "pts.h"


class EventListItem : public QListWidgetItem
{
public:
    enum eventtype { none, start, stop, chapter, bookmark };

public:
    EventListItem( const QPixmap &pixmap, int width, eventtype type, int picture, int picturetype, pts_t _pts, QListWidget *parent = NULL );
    ~EventListItem();

    const QPixmap *pixmap() const
    {
        return &pm;
    }

    int getpicture() const
    {
        return pic;
    }

    pts_t getpts() const
    {
        return pts;
    }

    enum eventtype geteventtype() const
    {
        return evtype;
    }

    void setEventType(enum eventtype type);
    int	 height() const;
    int	 width( const QListWidget *lb )  const;

    int rtti() const;
    static int RTTI()
    {
        return 294268923;
    }

    QString getString(bool coloured = true) const;
    bool operator <(const EventListItem &other) const;
    bool operator <(const QListWidgetItem &other) const;

private:
    QPixmap pm;
    enum eventtype evtype;
    int pic;
    int pictype;
    pts_t pts;
};

#endif // ifndef _TVCLIPPER_EVENTLISTIEM_H_
