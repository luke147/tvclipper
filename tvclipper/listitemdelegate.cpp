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
    QString colouredText = text;
    if (highlighted && isFocusedList) {
        colouredText = colouredText.replace(QString(START_COLOUR_MASK), getColourName(DEFAULT_HIGHLIGHT_START_COLOUR));
        colouredText = colouredText.replace(QString(STOP_COLOUR_MASK), getColourName(DEFAULT_HIGHLIGHT_STOP_COLOUR));
        colouredText = colouredText.replace(QString(CHAPTER_COLOUR_MASK), getColourName(DEFAULT_HIGHLIGHT_CHAPTER_COLOUR));
        colouredText = colouredText.replace(QString(BOOKMARK_COLOUR_MASK), getColourName(DEFAULT_HIGHLIGHT_BOOKMARK_COLOUR));
        colouredText = colouredText.replace(QString(NORMAL_TEXT_COLOUR_MASK), getColourName(DEFAULT_HIGHLIGHT_TEXT_COLOUR));
    } else {
        colouredText = colouredText.replace(QString(START_COLOUR_MASK), getColourName(DEFAULT_NORMAL_START_COLOUR));
        colouredText = colouredText.replace(QString(STOP_COLOUR_MASK), getColourName(DEFAULT_NORMAL_STOP_COLOUR));
        colouredText = colouredText.replace(QString(CHAPTER_COLOUR_MASK), getColourName(DEFAULT_NORMAL_CHAPTER_COLOUR));
        colouredText = colouredText.replace(QString(BOOKMARK_COLOUR_MASK), getColourName(DEFAULT_NORMAL_BOOKMARK_COLOUR));
        colouredText = colouredText.replace(QString(NORMAL_TEXT_COLOUR_MASK), getColourName(DEFAULT_NORMAL_TEXT_COLOUR));
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
