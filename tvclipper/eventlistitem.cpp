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

#include <typeinfo>

#include <QTextDocument>
#include <QApplication>
#include <QIcon>

#include "eventlistitem.h"
#include "settings.h"

EventListItem::EventListItem(const QPixmap &pixmap, int width,
                             eventtype type, int picture, int picturetype, pts_t _pts, QListWidget *parent ) :
    QListWidgetItem(parent), pm(pixmap), evtype(type), pic(picture), pictype(picturetype), pts(_pts)
/*
 * Parent has to be NULL if QListWidget::insertItem method is called.
 * Operator< is overriden for right functionality QListWidget::insertItem method.
 * It isn't necessary to parent has been NULL if QListWidget::addItem method is called.
 */
{
    if (pm.width() > EVENT_ITEM_ICON_WIDTH || pm.height() > EVENT_ITEM_ICON_HEIGHT)
        pm=pm.scaled(EVENT_ITEM_ICON_WIDTH, EVENT_ITEM_ICON_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (width == 0) {
        QTextDocument document;
        document.setHtml(getString(false));
        width = pm.width() + document.size().width() + EVENT_ITEM_SPACE_BETWEEN_ELEMENTS;
    }

    setSizeHint(QSize(width, height()));
    setIcon(QIcon(pm));
    setText(getString());
}

EventListItem::~EventListItem()
{}


int EventListItem::rtti() const
{
    return RTTI();
}

int EventListItem::height() const
{
    int h = 0;

    if (!pm.isNull())
        h = pm.height();

    return qMax( h + (EVENT_ITEM_ICON_MARGIN * 2), QApplication::globalStrut().height() );
}

void EventListItem::setEventType(eventtype type)
{
    evtype = type;
    setText(getString());
    return;
}

QString EventListItem::getString(bool coloured) const
{
    QString label;
    const char normalTextColour[8] = "#000000";
    if (coloured) {
        if (evtype==start)
            label = settings()->start_label;
        else if (evtype==stop)
            label = settings()->stop_label;
        else if (evtype==chapter)
            label = settings()->chapter_label;
        else if (evtype==bookmark)
            label = settings()->bookmark_label;

    } else {
        label = QString(EVENT_ITEM_TEST_LABEL);
    }

    return label + QString().sprintf("<br><font color=\"%s\">%02d:%02d:%02d.%03d</font><br><font color=\"%s\">%d (%c)</font>",
                                     (coloured) ? NORMAL_TEXT_COLOUR_MASK : normalTextColour,
                                     int(pts/(3600*90000)),
                                     int(pts/(60*90000))%60,
                                     int(pts/90000)%60,
                                     int(pts/90)%1000,
                                     (coloured) ? NORMAL_TEXT_COLOUR_MASK : normalTextColour,
                                     pic,
                                     ((const char *)".IPB....")[pictype&7]);
}

bool EventListItem::operator <(const EventListItem &other) const
{
    if (this->getpicture() < other.getpicture())
        return true;

    return false;
}

bool EventListItem::operator <(const QListWidgetItem &other) const
{
    if (typeid(EventListItem*) != typeid(other))
        return false;
    EventListItem *o = (EventListItem*) &other;
    if (this->getpicture() < o->getpicture())
        return true;

    return false;
}
