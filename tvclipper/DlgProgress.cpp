/*  tvclipper
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
*/

/* $Id: progresswindow.cpp 175 2011-04-24 11:47:34Z too-tired $ */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include <qprogressbar.h>
#include <qtextbrowser.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <QPainter>
#include <QTextBlockFormat>
#include <QTextBlock>
#include <QCloseEvent>

#include "DlgProgress.h"

DlgProgress::DlgProgress(QWidget *parent, const char *name)
    :QDialog(parent), logoutput(),
      cancelwasclicked(false), waitingforclose(false)
{
    if (name != NULL)
        setAccessibleName(name);
    ui = new Ui::DlgProgress();
    ui->setupUi(this);

    warningIcon = QIcon::fromTheme(QString("dialog-warning"));
    errorIcon = QIcon::fromTheme(QString("dialog-error"));

    ui->logBrowser->textCursor().setPosition(0);

    show();
    qApp->processEvents();
}

DlgProgress::~DlgProgress()
{
    delete ui;
}

void DlgProgress::closeEvent(QCloseEvent *e)
{
    if (waitingforclose)
        e->accept();
    else
        e->ignore();
}

void DlgProgress::finish()
{
    ui->cancelbutton->setEnabled(false);
    waitingforclose=true;
    ui->cancelbutton->setText( tr( "Close" ) );
    ui->cancelbutton->setEnabled(true);
    exec();
}

void DlgProgress::setprogress(int permille)
{
    if (permille==currentprogress)
        return;
    currentprogress=permille;
    ui->progressBar->setValue(permille);
    qApp->processEvents();
}

void DlgProgress::print(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    char *text=0;
    if (vasprintf(&text,fmt,ap)<0 || (text==0))
        return;

    if (*text) {
        ui->logBrowser->insertHtml(QString("<span class=\"text\" style=\"white-space: pre;\">") + quotetext(text) + QString("<br/></span>"));
    } else
        ui->logBrowser->insertHtml(QString("<br>"));
    free(text);
    qApp->processEvents();
}

void DlgProgress::printheading(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    char *text=0;
    if (vasprintf(&text,fmt,ap)<0 || (text==0))
        return;

    ui->logBrowser->insertHtml(QString("<h4 style=\"text-decoration: underline;\" class=\"heading\">")+quotetext(text)+"<br/></h4>");

    free(text);
    qApp->processEvents();
}

void DlgProgress::printinfo(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    char *text=0;
    if (vasprintf(&text,fmt,ap)<0 || (text==0))
        return;

    ui->logBrowser->insertHtml(QString("<span>")+quotetext(text)+"<br/></span>");
    free(text);
    qApp->processEvents();
}

void DlgProgress::printerror(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    char *text=0;
    if (vasprintf(&text,fmt,ap)<0 || (text==0))
        return;

    QImage image = errorIcon.pixmap(16, 16).toImage();
    ui->logBrowser->textCursor().insertImage(image);

    ui->logBrowser->insertHtml(QString("<span style=\"color: red; font-weight: bold;\">")+quotetext(text)+"<br/></span>");
    free(text);
    qApp->processEvents();
}

void DlgProgress::printwarning(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    char *text=0;
    if (vasprintf(&text,fmt,ap)<0 || (text==0))
        return;

    QImage image = warningIcon.pixmap(16, 16).toImage();
    ui->logBrowser->textCursor().insertImage(image);

    ui->logBrowser->insertHtml(QString("<span>")+quotetext(text)+"<br/></span>");
    free(text);
    qApp->processEvents();
}

void DlgProgress::clickedcancel()
{
    if ((cancelwasclicked==false) && (waitingforclose==false)) {
        // button function is cancel
        cancelwasclicked=true;
        ui->cancelbutton->setEnabled(false);
        qApp->processEvents();
        ui->cancelbutton->setText( tr( "Close" ) );
        ui->cancelbutton->setEnabled(true);
    } else {
        // button function is close
        close();
    }
}

QString DlgProgress::quotetext(const char *text)
{
    return QString(text).replace('&',QString("&amp;")).replace('<',QString("&lt;")).replace('>',QString("&gt;"));
}
