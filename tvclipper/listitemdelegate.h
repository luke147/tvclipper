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
