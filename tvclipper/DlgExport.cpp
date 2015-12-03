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

#include "DlgExport.h"
#include <QFileDialog>

DlgExport::DlgExport(const QString &filename, QWidget *parent, const char *name)
    :QDialog(parent)
{
    if (name != NULL)
        setAccessibleName(QString(name));
    ui = new Ui::DlgExport();
    ui->setupUi(this);
    ui->filenameline->setText(filename);
}

DlgExport::~DlgExport()
{
    delete ui;
}

void DlgExport::fileselector()
{
    QString selectFilter = tr("MPEG program streams (*.mpg)");
    QString newfilename = QFileDialog::getSaveFileName(this,
                                                       tr("Export video..."),
                                                       ui->filenameline->text(),
                                                       tr("MPEG program streams (*.mpg);All files (*)"),
                                                       &selectFilter,
                                                       0);

    if (!newfilename.isEmpty())
        ui->filenameline->setText(newfilename);
}
