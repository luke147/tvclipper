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

#include "xmlprjfile.h"

#include <QMessageBox>
#include <iostream>

#define FILE_CODING "UTF-8"
#define XML_VERSION "1.0"
#define XML_DECLARATION "<?xml version=\"" XML_VERSION "\" encoding=\"" FILE_CODING "\"?>"
#define DOCTYPE "<!DOCTYPE tvclipper>"
#define START_DOC_TAG "tvclipper"
#define MPGFILE_TAG "mpgfile"
#define IDXFILE_TAG "idxfile"
#define EXPFILE_TAG "expfile"
#define START_EVENTMARK_TAG "start"
#define STOP_EVENTMARK_TAG "stop"
#define CHAPTER_EVENTMARK_TAG "chapter"
#define BOOKMARK_MARK_TAG "bookmark"
#define FILE_PATH_ATTRIBUTE "path"
#define EXPORT_FORMAT_ATTRIBUTE "format"
#define PICTURE_NUMBER_ATTRIBUTE "picture"

#define START_DOC_TAG_BACKWARD_COMPATIBILITY "dvbcut"


XmlPrjFile::XmlPrjFile()
{
    xmlFile = new QFile();
    eventMarks = EventMarks();
}

XmlPrjFile::XmlPrjFile(std::list<std::string> mpgList, QString idxFile, QString expFile, QString expFormat, EventMarks eventMarks)
{
    xmlFile = new QFile();

    this->mpgList = mpgList;
    this->idxFile = idxFile;
    this->expFile = expFile;
    this->expFormat = expFormat;
    this->eventMarks = eventMarks;
}

XmlPrjFile::~XmlPrjFile()
{
    if (xmlFile)
        delete xmlFile;
    xmlFile = 0;
}

XmlPrjFileReader::XmlPrjFileReader() : XmlPrjFile()
{
    xmlDoc = new QXmlStreamReader();
}

XmlPrjFileReader::~XmlPrjFileReader()
{
    if (xmlDoc)
        delete xmlDoc;
    xmlDoc = 0;
}

bool XmlPrjFileReader::isXmlFile(QString filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    char buff[512];
    QString line;
    qint64 readBytes = f.readLine(buff, 512);
    f.close();
    if (readBytes > 0)
        line = QString::fromLatin1(buff, readBytes);

    if ((!line.startsWith(QString(DOCTYPE))) && (!line.startsWith(QString(XML_DECLARATION))))
        return false;

    return true;
}

const QString XmlPrjFileReader::readStringAttribValue(QString attribName) const
{
    QString attribValue;
    QXmlStreamAttributes attribs = xmlDoc->attributes();
    for (QXmlStreamAttributes::const_iterator iter = attribs.begin(); iter != attribs.end(); iter++) {
        if (iter->name().toString() != attribName)
            continue;

        attribValue = iter->value().toString();
        if (attribValue.isEmpty()) {
            return QString();
        }

        break;
    }

    return attribValue;
}

bool XmlPrjFileReader::read(QString prjFileName, bool NoIdxAndExpFiles, QString &errorStr)
{
    xmlFile->setFileName(prjFileName);
    if (!xmlFile->open(QIODevice::ReadOnly)) {
        errorStr = xmlDoc->errorString();
        return false;
    }

    xmlDoc->setDevice(xmlFile);
    if (xmlDoc->hasError()) {
        errorStr = xmlDoc->errorString();
        return false;
    }

    mpgList.clear();
    if (!NoIdxAndExpFiles) {
        // in batch mode CLI-switches have priority!
        idxFile.clear();
        expFile.clear();
    }

    while (!xmlDoc->atEnd()) {
        QXmlStreamReader::TokenType type = xmlDoc->readNext();
        if (xmlDoc->hasError()) {
            errorStr = xmlDoc->errorString();
            return false;
        }

        if (QXmlStreamReader::StartElement == type && (xmlDoc->name() == START_DOC_TAG || xmlDoc->name() == START_DOC_TAG_BACKWARD_COMPATIBILITY)) {
            break;
        }
    }

    while (!xmlDoc->atEnd()) {
        QXmlStreamReader::TokenType type = xmlDoc->readNext();
        if (xmlDoc->hasError()) {
            errorStr = xmlDoc->errorString();
            return false;
        }

        if (QXmlStreamReader::StartElement != type)
            continue;

        if (xmlDoc->name().toString() == MPGFILE_TAG) {
            mpgList.push_back(readStringAttribValue(QString(FILE_PATH_ATTRIBUTE)).toStdString());
            continue;
        }

        if (xmlDoc->name().toString() == IDXFILE_TAG) {
            idxFile = readStringAttribValue(QString(FILE_PATH_ATTRIBUTE));
            continue;
        }

        if (xmlDoc->name() == EXPFILE_TAG) {
            expFile = readStringAttribValue(QString(FILE_PATH_ATTRIBUTE));
            expFormat = readStringAttribValue(QString(EXPORT_FORMAT_ATTRIBUTE));
            continue;
        }

        if (xmlDoc->name() == START_EVENTMARK_TAG) {
            eventMarks.push_back(EventMark(readStringAttribValue(QString(PICTURE_NUMBER_ATTRIBUTE)), EventListItem::start));
            continue;
        }

        if (xmlDoc->name() == STOP_EVENTMARK_TAG) {
            eventMarks.push_back(EventMark(readStringAttribValue(QString(PICTURE_NUMBER_ATTRIBUTE)), EventListItem::stop));
            continue;
        }

        if (xmlDoc->name() == CHAPTER_EVENTMARK_TAG) {
            eventMarks.push_back(EventMark(readStringAttribValue(QString(PICTURE_NUMBER_ATTRIBUTE)), EventListItem::chapter));
            continue;
        }

        if (xmlDoc->name() == BOOKMARK_MARK_TAG) {
            eventMarks.push_back(EventMark(readStringAttribValue(QString(PICTURE_NUMBER_ATTRIBUTE)), EventListItem::bookmark));
            continue;
        }
    }

    if (mpgList.empty()) {
        errorStr = QString("Project file is empty. No MPG file to open.");
        return false;
    }

    return true;
}

XmlPrjFileWriter::XmlPrjFileWriter(std::list<std::string> mpgList, QString idxFile, QString expFile, QString expFormat, EventMarks eventMarks)
    : XmlPrjFile(mpgList, idxFile, expFile, expFormat, eventMarks)
{
    xmlDoc = new QXmlStreamWriter();
}

XmlPrjFileWriter::~XmlPrjFileWriter()
{
    if (xmlDoc)
        delete xmlDoc;
    xmlDoc = 0;
}

bool XmlPrjFileWriter::write(QString prjFilename)
{
    bool okay;
    xmlFile->setFileName(prjFilename);
    if (!xmlFile->open(QIODevice::WriteOnly)) {
        return false;
    }

    xmlDoc->setDevice(xmlFile);
    xmlDoc->setCodec(FILE_CODING);
    xmlDoc->writeStartDocument(QString(XML_VERSION));
    xmlDoc->writeCharacters(QString("\n"));
    xmlDoc->writeDTD(QString(DOCTYPE));
    xmlDoc->writeCharacters(QString("\n"));
    xmlDoc->writeStartElement(QString(START_DOC_TAG));

    for (std::list<std::string>::const_iterator it = mpgList.begin(); it != mpgList.end(); it++) {
        okay = writeFileNameToElem(QString(MPGFILE_TAG), QString::fromStdString(*it));
        if (!okay)
            return false;
    }

    if (!idxFile.isEmpty()) {
        okay = writeFileNameToElem(QString(IDXFILE_TAG), idxFile);
        if (!okay)
            return false;
    }

    if (!expFile.isEmpty()) {
        okay = writeExportFileName(QString(EXPFILE_TAG), expFile, expFormat);
        if (!okay)
            return false;
    }

    for (EventMarks::const_iterator it = eventMarks.begin(); it != eventMarks.end(); it++) {
        okay = writeMarkToElem(it->type, it->picNumber);
        if (!okay)
            return false;
    }

    xmlDoc->writeCharacters(QString("\n"));
    xmlDoc->writeEndElement(); // end document tag: </tvclipper>
    xmlDoc->writeEndDocument();

    if (xmlDoc->hasError())
        return false;

    xmlFile->close();

    return true;
}

bool XmlPrjFileWriter::writeFileNameToElem(QString elemName, QString filename)
{
    xmlDoc->writeCharacters(QString(" "));
    xmlDoc->writeStartElement(elemName);
    xmlDoc->writeAttribute(QString(FILE_PATH_ATTRIBUTE), filename);
    xmlDoc->writeEndElement();
    xmlDoc->writeCharacters(QString("\n"));
    if (xmlDoc->hasError())
        return false;

    return true;
}

bool XmlPrjFileWriter::writeExportFileName(QString elemName, QString filename, QString exportFormat)
{
    xmlDoc->writeCharacters(QString(" "));
    xmlDoc->writeStartElement(elemName);
    xmlDoc->writeAttribute(QString(FILE_PATH_ATTRIBUTE), filename);
    xmlDoc->writeAttribute(QString(EXPORT_FORMAT_ATTRIBUTE), exportFormat);
    xmlDoc->writeEndElement();
    xmlDoc->writeCharacters(QString("\n"));
    if (xmlDoc->hasError())
        return false;

    return true;
}

bool XmlPrjFileWriter::writeMarkToElem(EventListItem::eventtype markType, QString picNumber)
{
    QString elemName;

    switch (markType) {
    case EventListItem::start:
        elemName = "start";
        break;
    case EventListItem::stop:
        elemName = "stop";
        break;
    case EventListItem::chapter:
        elemName = "chapter";
        break;
    case EventListItem::bookmark:
        elemName = "bookmark";
        break;
    default:
        return true;
    }

    xmlDoc->writeCharacters(QString(" "));
    xmlDoc->writeStartElement(elemName);
    xmlDoc->writeAttribute(QString("picture"), picNumber);
    xmlDoc->writeEndElement();
    xmlDoc->writeCharacters(QString("\n"));
    if (xmlDoc->hasError())
        return false;

    return true;
}
