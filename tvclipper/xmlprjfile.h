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

#ifndef XMLPRJFILE_H
#define XMLPRJFILE_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QString>
#include <string>
#include <list>

#include "eventlistitem.h"

class EventMark
{
public:
    QString picNumber;
    EventListItem::eventtype type;
    EventMark() {
        picNumber = QString();
        type = EventListItem::none;
    }
    EventMark(QString _picNumber, EventListItem::eventtype _type)
    {
        picNumber = _picNumber;
        type = _type;
    }
};

typedef std::list<EventMark> EventMarks;

class XmlPrjFile
{
public:
    XmlPrjFile();
    XmlPrjFile(std::list<std::string> mpgList, QString idxFile, QString expFile, QString expFormat, EventMarks eventMarks);
    virtual ~XmlPrjFile();

public:
    QFile *xmlFile;
    std::list<std::string> mpgList;
    QString idxFile;
    QString expFile;
    QString expFormat;
    EventMarks eventMarks;
};

class XmlPrjFileReader : public XmlPrjFile
{
public: //public methods
    XmlPrjFileReader();
    ~XmlPrjFileReader();
    bool read(QString prjFileName, bool NoIdxAndExpFiles, QString &errorStr);
    static bool isXmlFile(QString filename);

private: //private attributes
    QXmlStreamReader *xmlDoc;

private: //private methods
    const QString readStringAttribValue(QString attribName) const;
};

class XmlPrjFileWriter : public XmlPrjFile
{
public: // public methods
    XmlPrjFileWriter(std::list<std::string> mpgList, QString idxFile, QString expFile, QString expFormat, EventMarks eventMarks);
    ~XmlPrjFileWriter();
    bool write(QString prjFilename);

private: // private attributes
    QXmlStreamWriter *xmlDoc;

private: // private methods
    bool writeFileNameToElem(QString elemName, QString filename);
    bool writeExportFileName(QString elemName, QString filename, QString exportFormat);
    bool writeMarkToElem(EventListItem::eventtype markType, QString picNumber);
};

#endif // XMLPRJFILE_H
