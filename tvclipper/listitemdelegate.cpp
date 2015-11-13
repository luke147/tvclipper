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

#include "listitemdelegate.h"

#include <QTextDocument>
#include <QPainter>
#include <QPalette>
#include <QIcon>

#include "settings.h"

void ListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QListWidget *eventlist = (QListWidget*) this->parent();
    bool isFocusedList = true;
    if (eventlist)
        isFocusedList = eventlist->hasFocus();

    bool isSelectedItem = option.state & QStyle::State_Selected;
    QString text = index.data(Qt::DisplayRole).toString();
    text = getHtmlWithColours(text, isSelectedItem, isFocusedList);
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    QSize iconSize = icon.actualSize(QSize(option.rect.size().height() * 2, option.rect.size().height()));
    QRect iconRect = QRect(option.rect.x() + EVENT_ITEM_ICON_MARGIN, option.rect.y() + EVENT_ITEM_ICON_MARGIN, iconSize.width(), iconSize.height());
    QPixmap iconPixmap = icon.pixmap(iconSize);
    QPoint textTopLeft = QPoint(iconRect.x() + iconSize.width() + EVENT_ITEM_ICON_MARGIN + EVENT_ITEM_TEXT_MARGIN, option.rect.y() + EVENT_ITEM_ICON_MARGIN + EVENT_ITEM_TEXT_MARGIN);
    QSize textSize = QSize(option.rect.width() - textTopLeft.x(), option.rect.height() - textTopLeft.y());
    QRect textRect = QRect(textTopLeft, textSize);
    QTextDocument document;
    document.setUndoRedoEnabled(false);

    if (isSelectedItem)
    {
        const QPalette::ColorRole color = (isFocusedList) ? QPalette::Highlight : QPalette::Window;
        painter->resetTransform();
        painter->fillRect(option.rect, option.palette.color(color));
    }

    //painter->save();
    //painter->restore();
    painter->resetTransform();

    document.setHtml(text);
    painter->drawPixmap(iconRect, iconPixmap);

    //painter->save();
    //painter->restore();
    painter->resetTransform();
    painter->translate(textRect.topLeft());
    painter->setRenderHint(QPainter::Antialiasing, true);

    document.drawContents(painter);

    //painter->save();
    //painter->restore();
}

QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();

    int w = option.rect.width();
    int h = icon.actualSize(QSize(w, w)).height() + (2 * EVENT_ITEM_ICON_MARGIN);
    return QSize(w,h);
}

QString ListItemDelegate::getHtmlWithColours(QString text, const bool highlighted, const bool isFocusedList) const
{
    QString startColor, stopColor, chapterColor, bookmarkColor, textColor;
    bool customColors = settings()->customColors;

    QString colouredText = text;
    if (highlighted && isFocusedList) {
        if (customColors) {
            startColor = settings()->colorStartHighlight;
            stopColor = settings()->colorStopHighlight;
            chapterColor = settings()->colorChapterHighlight;
            bookmarkColor = settings()->colorBookmarkHighlight;
            textColor = settings()->colorTextHighlight;
        } else {
            startColor = getColourName(DEFAULT_HIGHLIGHT_START_COLOUR);
            stopColor = getColourName(DEFAULT_HIGHLIGHT_STOP_COLOUR);
            chapterColor = getColourName(DEFAULT_HIGHLIGHT_CHAPTER_COLOUR);
            bookmarkColor = getColourName(DEFAULT_HIGHLIGHT_BOOKMARK_COLOUR);
            textColor = getColourName(DEFAULT_HIGHLIGHT_TEXT_COLOUR);
        }
        colouredText = colouredText.replace(QString(START_COLOUR_MASK), startColor);
        colouredText = colouredText.replace(QString(STOP_COLOUR_MASK), stopColor);
        colouredText = colouredText.replace(QString(CHAPTER_COLOUR_MASK), chapterColor);
        colouredText = colouredText.replace(QString(BOOKMARK_COLOUR_MASK), bookmarkColor);
        colouredText = colouredText.replace(QString(NORMAL_TEXT_COLOUR_MASK), textColor);
    } else {
        if (customColors) {
            startColor = settings()->colorStartNormal;
            stopColor = settings()->colorStopNormal;
            chapterColor = settings()->colorChapterNormal;
            bookmarkColor = settings()->colorBookmarkNormal;
            textColor = settings()->colorTextNormal;
        } else {
            startColor = getColourName(DEFAULT_NORMAL_START_COLOUR);
            stopColor = getColourName(DEFAULT_NORMAL_STOP_COLOUR);
            chapterColor = getColourName(DEFAULT_NORMAL_CHAPTER_COLOUR);
            bookmarkColor = getColourName(DEFAULT_NORMAL_BOOKMARK_COLOUR);
            textColor = getColourName(DEFAULT_NORMAL_TEXT_COLOUR);
        }
        colouredText = colouredText.replace(QString(START_COLOUR_MASK), startColor);
        colouredText = colouredText.replace(QString(STOP_COLOUR_MASK), stopColor);
        colouredText = colouredText.replace(QString(CHAPTER_COLOUR_MASK), chapterColor);
        colouredText = colouredText.replace(QString(BOOKMARK_COLOUR_MASK), bookmarkColor);
        colouredText = colouredText.replace(QString(NORMAL_TEXT_COLOUR_MASK), textColor);
    }

    return colouredText;
}

QString ListItemDelegate::getColourName(Qt::GlobalColor colourIndex) const
{
    QColor colour = colourIndex;
    return colour.name();
}

QString ListItemDelegate::getColourName(QPalette::ColorRole colourRole) const
{
    QPalette palette = QPalette();
    QColor colour = palette.color(colourRole);
    return colour.name();
}
