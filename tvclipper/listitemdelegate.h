/*  tvclipper
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

#ifndef LISTITEMDELEGATE_H
#define LISTITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QListWidget>

class ListItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    ListItemDelegate(QListWidget *l) : QAbstractItemDelegate(l) {}
    ~ListItemDelegate() {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &mpgIndex) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &mpgIndex) const;
private:
    QString getHtmlWithColours(QString text, const bool highlighted, const bool isFocusedList) const;
    QString getColourName(Qt::GlobalColor colourIndex) const;
    QString getColourName(QPalette::ColorRole colourRole) const;

public slots:

};

#endif // LISTITEMDELEGATE_H
