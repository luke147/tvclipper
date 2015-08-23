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
