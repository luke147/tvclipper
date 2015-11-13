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

#ifndef _TVCLIPPER_PROGRESSWINDOW_H
#define _TVCLIPPER_PROGRESSWINDOW_H

class QIcon;

#include <string>
#include <qstring.h>
#include "ui_DlgProgress.h"
#include "logoutput.h"

class DlgProgress: public QDialog, public logoutput
{
    Q_OBJECT
protected:
    bool cancelwasclicked;
    bool waitingforclose;

    static QString quotetext(const char* text);
    void closeEvent(QCloseEvent *e);
    Ui::DlgProgress *ui;

private:
    QIcon warningIcon;
    QIcon errorIcon;

public:
    DlgProgress(QWidget *parent = 0, const char *name = 0);
    ~DlgProgress();
    virtual bool cancelled()
    {
        return cancelwasclicked;
    }
    void finish();
    void print(const char *fmt, ...);
    void printheading(const char *fmt, ...);
    void printinfo(const char *fmt, ...);
    void printerror(const char *fmt, ...);
    void printwarning(const char *fmt, ...);

public slots:
    void setprogress(int permille);
    void clickedcancel();
};

#endif
