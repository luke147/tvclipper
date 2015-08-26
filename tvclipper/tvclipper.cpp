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

/* $Id: tvclipper.cpp 173 2011-04-22 22:00:52Z too-tired $ */

#include <unistd.h>

#include <cstring>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <memory>
#include <algorithm>
#include <typeinfo>

#include <qlabel.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qcolor.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qslider.h>
#include <qapplication.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qprocess.h>
#include <QMenu>
#include <qpushbutton.h>
#include <qaction.h>
#include <qtextbrowser.h>
#include <qfile.h>
#include <qcursor.h>
#include <qcombobox.h>
#include <qmenubar.h>
#include <qsettings.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <QTextStream>
#include <QDir>
#include <QWheelEvent>
#include <QThread>

#include "port.h"
#include "tvclipper.h"
#include "ui_tvclipper.h"
#include "mpgfile.h"
#include "avframe.h"
#include "eventlistitem.h"
#include "listitemdelegate.h"
#include "lavfmuxer.h"
#include "mpegmuxer.h"
#include "imageprovider.h"
#include "differenceimageprovider.h"
#include "busyindicator.h"
#include "progressstatusbar.h"
#include "settings.h"
#include "exception.h"

#include "DlgProgress.h"
#include "DlgMplayerError.h"
#include "DlgExport.h"

#include "version.h"

bool tvclipper::cache_friendly = true;

// **************************************************************************
// ***  busy cursor helpers

class tvclipperbusy : public busyindicator
{
protected:
    tvclipper *d;
    int bsy;
public:
    tvclipperbusy(tvclipper *_d) : busyindicator(), d(_d), bsy(0)
    {}
    ~tvclipperbusy()
    {
        while (bsy>0)
            setbusy(false);
        while (bsy<0)
            setbusy(true);
    }
    virtual void setbusy(bool busy=true)
    {
        if (busy)
            ++bsy;
        else {
            if (bsy<=0)
                return;
            --bsy;
        }
        d->setbusy(busy);
    }
};


void tvclipper::setbusy(bool b)
{
    if (b) {
        if (busy==0)
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        ++busy;
    } else if (busy>0) {
        --busy;
        if (busy==0)
            QApplication::restoreOverrideCursor();
    }
}

// **************************************************************************
// ***  tvclipper::tvclipper (private constructor)

tvclipper::tvclipper(QString orgName, QString appName, QWidget *parent, Qt::WindowFlags flags)
    :QMainWindow(parent),
      audiotrackpopup(0), recentfilespopup(0), editconvertpopup(0), audiotrackmenu(0),
      buf(8 << 20, 128 << 20),
      mpg(0), pictures(0),
      curpic(~0), showimage(true), fine(false),
      jogsliding(false), jogmiddlepic(0),
      mplayer_process(0), imgp(0), busy(0),
      viewscalefactor(1.0),
      nogui(false)
{
    this->appName = appName;
    this->orgName = orgName;
    // constructSettings(orgName, appName); // setting file: $HOME/.config/orgName/appName.conf
    constructSettings(orgName, QString()); // setting file: $HOME/.config/orgName.conf

    this->setAccessibleName(appName);
    this->setWindowFlags(flags);

    ui = new Ui_tvclipper();
    ui->setupUi(this);

#ifndef HAVE_LIB_AO
    ui->playAudio1Action->setEnabled(false);
    ui->playAudio2Action->setEnabled(false);
    ui->playToolbar->removeAction(ui->playAudio1Action);
    ui->playToolbar->removeAction(ui->playAudio2Action);
    ui->playMenu->removeAction(ui->playAudio1Action);
    ui->playMenu->removeAction(ui->playAudio2Action);
#endif // ! HAVE_LIB_AO

    audiotrackpopup=new QMenu(QString("Audio track"), this);
    ui->playMenu->addSeparator();
    audiotrackmenu=ui->playMenu->addMenu(audiotrackpopup);
    connect( audiotrackpopup, SIGNAL( triggered(QAction*) ), this, SLOT( audiotrackchosen(QAction*) ) );


    recentfilespopup=new QMenu(QString("Open recent..."), this);
    ui->fileMenu->insertMenu(ui->fileSaveAction, recentfilespopup);
    connect( recentfilespopup, SIGNAL( triggered(QAction*) ), this, SLOT( loadrecentfile(QAction*) ) );
    connect( recentfilespopup, SIGNAL( aboutToShow() ), this, SLOT( abouttoshowrecentfiles() ) );


    editconvertpopup=new QMenu(QString("Convert bookmarks"), this);
    ui->editMenu->insertMenu(ui->editBookmarkAction, editconvertpopup);
    connect( editconvertpopup, SIGNAL( triggered(QAction*) ), this, SLOT( editConvert(QAction*) ) );
    connect( editconvertpopup, SIGNAL( aboutToShow() ), this, SLOT( abouttoshoweditconvert() ) );

    setviewscalefactor(settings()->viewscalefactor);

    ListItemDelegate *itemDelegate = new ListItemDelegate(ui->eventlist);
    ui->eventlist->setItemDelegate(itemDelegate);
    ui->eventlist->setUpdatesEnabled(true);

    QAction *actionGoTo = new QAction(ui->eventlist);
    QAction *actionDelete = new QAction(ui->eventlist);
    actionGoTo->setText(QString("Go to"));
    actionDelete->setText(QString("Delete"));
    actionGoTo->setShortcut(QKeySequence(Qt::Key_Return));
    actionDelete->setShortcut(QKeySequence(QKeySequence::Delete));
    actionGoTo->setShortcutContext(Qt::WidgetShortcut);
    actionDelete->setShortcutContext(Qt::WidgetShortcut);
    ui->eventlist->addAction(actionGoTo);
    ui->eventlist->addAction(actionDelete);

    connect( actionGoTo, SIGNAL(triggered()), this, SLOT(actGoToEventItem()) );
    connect( actionDelete, SIGNAL(triggered()), this, SLOT(actDelEventItem()) );

    // install event handler
    installEventFilter(this);

    // set caption
    this->setWindowTitle(QString(VERSION_STRING));
}

// **************************************************************************
// ***  tvclipper::~tvclipper (destructor)

tvclipper::~tvclipper()
{
    if (mplayer_process) {
        mplayer_process->terminate();
        delete mplayer_process;
    }

    if (audiotrackpopup)
        delete audiotrackpopup;
    if (recentfilespopup)
        delete recentfilespopup;
    if (editconvertpopup)
        delete editconvertpopup;

    destructSettings();

    if (imgp)
        delete imgp;
    if (mpg)
        delete mpg;
    if (ui->eventlist->itemDelegate())
        delete ui->eventlist->itemDelegate();
    if(ui)
        delete ui;
}

// **************************************************************************
// ***  slots (actions)

void tvclipper::fileNew()
{
    tvclipper *d = new tvclipper(this->orgName, this->appName);
    d->show();
}

void tvclipper::fileOpen()
{
    open();
}

void tvclipper::fileSaveAs()
{
    if (prjfilen.empty() && !mpgfilen.empty() && !mpgfilen.front().empty()) {
        std::string prefix = mpgfilen.front();
        int lastdot = prefix.rfind(".");
        int lastslash = prefix.rfind("/");
        if (lastdot >= 0 && lastdot > lastslash)
            prefix = prefix.substr(0, lastdot);
        prjfilen = prefix + ".tvclipper";
        int nr = 0;
        while (QFileInfo(QString::fromStdString(prjfilen)).exists())
            prjfilen = prefix + "_" + QString::number(++nr).toStdString() + ".tvclipper";
    }

    QString s = QFileDialog::getSaveFileName(ui->tvclipperWidget,
                                           "Save project as...",
                                           QString::fromStdString(prjfilen),
                                           settings()->prjfilter,
                                           NULL,
                                           NULL);
    if (s.isNull())
        return;

    if (QFileInfo(s).exists() && question(
                "File exists - tvclipper",
                s + "\nalready exists. Overwrite?") !=
            QMessageBox::Yes)
        return;

    prjfilen=s.toStdString();
    if (!prjfilen.empty())
        fileSave();
}

void tvclipper::fileSave()
{
    if (prjfilen.empty()) {
        fileSaveAs();
        return;
    }

    EventMarks marks;
    {
        QListWidgetItem *item;

        for (int i = 0; (i < ui->eventlist->count()); i++) {
            item = ui->eventlist->item(i);

            if (typeid(*item) != typeid(EventListItem)) {
                continue;
            }

            EventListItem *eli = (EventListItem*)item;
            class EventMark mark = EventMark(QString::number(eli->getpicture()), eli->geteventtype());
            marks.push_back(mark);
        }
    }

    class XmlPrjFileWriter prjWriter = XmlPrjFileWriter(mpgfilen, QString::fromStdString(idxfilen), QString::fromStdString(expfilen), QString::number(exportformat), marks);
    bool okay = prjWriter.write(QString::fromStdString(prjfilen));
    if (!okay) {
        QMessageBox::critical(this, PROGRAM_NAME " - Failed to write project file",
                                      QString::fromStdString(prjfilen) + ": \nCould not open file",
                                      QMessageBox::Abort,
                                      QMessageBox::NoButton);
    }
    return;
}

void tvclipper::snapshotSave()
{
    std::vector<int> piclist;
    piclist.push_back(curpic);

    snapshotSave(piclist);
}

void tvclipper::chapterSnapshotsSave()
{
    int found=0;
    std::vector<int> piclist;
    {
        QListWidgetItem *item;
        for (int i = 0; (i < ui->eventlist->count()); i++) {
            item = ui->eventlist->item(i);

            if (typeid(*item) == typeid(EventListItem)) {
                EventListItem *eli=(EventListItem*)item;
                if (eli->geteventtype()==EventListItem::chapter) {
                    piclist.push_back(eli->getpicture());
                    found++;
                }
            }
        }
    }

    if (found) {
        snapshotSave(piclist, settings()->snapshot_range, settings()->snapshot_samples);
    } else {
        statusBar()->showMessage(QString("*** No chapters to save! ***"));
    }
}

void tvclipper::snapshotSave(std::vector<int> piclist, int range, int samples)
{
    QString prefix;
    QString type=settings()->snapshot_type;
    QString delim=settings()->snapshot_delimiter;
    QString ext=settings()->snapshot_extension;
    int first = settings()->snapshot_first;
    int width = settings()->snapshot_width;
    int quality = settings()->snapshot_quality;

    // get unique filename
    if (picfilen.isEmpty()) {
        if(settings()->snapshot_prefix.isEmpty()) {
            if (!prjfilen.empty())
                prefix = QString::fromStdString(prjfilen);
            else if (!mpgfilen.empty() && !mpgfilen.front().empty())
                prefix = QString::fromStdString(mpgfilen.front());
        } else
            prefix = settings()->snapshot_prefix;

        if (!prefix.isEmpty()) {
            int lastdot = prefix.lastIndexOf('.');
            int lastslash = prefix.lastIndexOf('/');
            if (lastdot >= 0 && lastdot > lastslash)
                prefix = prefix.left(lastdot);
            int nr = first;
            picfilen = prefix + delim + QString::number(nr).rightJustified( width, '0' ) + "." + ext;
            while (QFileInfo(picfilen).exists())
                picfilen = prefix + delim + QString::number(++nr).rightJustified( width, '0' )+ "." + ext;
        }
    }

    QString s = QFileDialog::getSaveFileName(ui->tvclipperWidget,
                                             "Save picture as...",
                                             picfilen,
                                             QString("Images (*."+ext+")"),
                                             NULL,
                                             NULL);
    if (s.isEmpty())
        return;

    if (QFileInfo(s).exists() && question(
                "File exists - tvclipper",
                s + "\nalready exists. Overwrite?") !=
            QMessageBox::Yes)
        return;

    QImage p;
    int pic, i, nr;
    bool ok=false;
    for (std::vector<int>::iterator it = piclist.begin(); it != piclist.end(); ++it) {

        if(samples>1 && range>0)
            pic = chooseBestPicture(*it, range, samples);
        else
            pic = *it+range;

        // save selected picture to file
        if (imgp)
            p = imgp->getimage(pic,fine);
        else
            p = imageprovider(*mpg, new tvclipperbusy(this), false, viewscalefactor).getimage(pic,fine);
        if(p.save(s,type.toStdString().c_str(),quality))
            statusBar()->showMessage("Saved snapshot: " + s);
        else
            statusBar()->showMessage("*** Unable to save snapshot: " + s + "! ***");

        // try to "increment" the choosen filename for next snapshot (or use old name as default)
        // No usage of "delim", so it's possible to choose any prefix in front of the number field!
        i = s.lastIndexOf(QRegExp("\\d{"+QString::number(width)+","+QString::number(width)+"}\\."+ext+"$"));
        if (i>0) {
            nr = s.mid(i,width).toInt(&ok,10);
            if (ok)
                picfilen = s.left(i) + QString::number(++nr).rightJustified(width, '0')+ "." + ext;
            else
                picfilen = s;
        }
        else
            picfilen = s;

        s = picfilen;
    }

}

int tvclipper::chooseBestPicture(int startpic, int range, int samples)
{
    QImage p;
    QRgb col;
    int idx, x, y, w, h, pic, norm, colors;
    int r, g, b, nr=11, ng=16, nb=5;  // "borrowed" the weights from calc. of qGray = (11*r+16*g+5*b)/32
    double entropy;
    std::vector<double> histogram;

    samples = samples>0 ? samples: 1;
    samples = samples>abs(range) ? abs(range)+1: samples;

    int bestpic = startpic + range; //, bestnr = 0;
    double bestval = 0.;
    int dp = range/(samples-1);
    int ncol = nr*ng*nb;

    // choose the best picture among equidistant samples in the range (not for single snapshots!)
    for (int n=0; n<samples && samples>1 && range>0; n++) {
        pic = startpic+n*dp;

        if (imgp)
            p = imgp->getimage(pic,fine);
        else
            p = imageprovider(*mpg, new tvclipperbusy(this), false, viewscalefactor).getimage(pic,fine);

        // get a measure for complexity of picture (at least good enough to discard single colored frames!)

        // index color space and fill histogram
        w = p.width();
        h = p.height();
        histogram.assign(ncol,0.);
        for(x=0; x<w; x++)
            for(y=0; y<h; y++) {
                col=p.pixel(x,y);
                r=nr*qRed(col)/256;
                g=ng*qGreen(col)/256;
                b=nb*qBlue(col)/256;
                idx=r+nr*g+nr*ng*b;
                histogram[idx]++;
            }

        // calc. probability to fall in a given color class
        colors = 0;
        norm = w*h;
        for(unsigned int i=0; i<histogram.size(); i++)
            if(histogram[i]>0) {
                colors++;
                histogram[i] /= norm;
            }

        // calc. the information entropy (complexity) of the picture
        entropy = 0.;
        for(x=0; x<w; x++)
            for(y=0; y<h; y++) {
                col=p.pixel(x,y);
                r=nr*qRed(col)/256;
                g=ng*qGreen(col)/256;
                b=nb*qBlue(col)/256;
                idx=r+nr*g+nr*ng*b;
                entropy-=histogram[idx]*log(histogram[idx]);
            }
        entropy /= log(2.);

        //fprintf(stderr,"frame %7d, sample %4d (%7d): %10d %10.2f\n",startpic,n,pic,colors,entropy);

        // largest "information content"?
        if(entropy>bestval) {
            bestval=entropy;
            bestpic=pic;
            //bestnr=n;
        }
    }
    //fprintf(stderr,"choosing sample / frame: %4d / %7d\n!", bestnr, bestpic);

    return bestpic;
}

bool tvclipper::saveExportDlgInfo(int &expfmt, int &pipe_items_start, int &selectedAudio)
{
    if (expfilen.empty()) {
        QString newexpfilen;

        if (!prjfilen.empty())
            newexpfilen = QString::fromStdString(prjfilen);
        else if (!mpgfilen.empty() && !mpgfilen.front().empty())
            newexpfilen = QString::fromStdString(mpgfilen.front());
        else
            return false;

        if (!newexpfilen.isEmpty()) {
            QFileInfo fInfo = QFileInfo(newexpfilen);
            newexpfilen = fInfo.dir().absolutePath() + QDir::separator() + fInfo.completeBaseName();

            expfilen=newexpfilen.toStdString()+".mpg";
            int nr = 0;
            while (QFileInfo(QString::fromStdString(expfilen)).exists())
                expfilen = newexpfilen.toStdString() + "_" + QString::number(++nr).toStdString() + ".mpg";
        }
    }

    std::auto_ptr<DlgExport> expd(new DlgExport(QString::fromStdString(expfilen),this));

#ifndef __WIN32__
    // add possible user configured pipe commands
    pipe_items_start=expd->ui->muxercombo->count();
    for (unsigned int i = 0; i < settings()->pipe_command.size(); ++i)
        expd->ui->muxercombo->addItem(settings()->pipe_label[i]);
#endif

    if (settings()->export_format < 0 || settings()->export_format >= expd->ui->muxercombo->count())
        settings()->export_format = 0;
    expd->ui->muxercombo->setCurrentIndex(settings()->export_format);

    for(int a=0;a<mpg->getaudiostreams();++a) {
        expd->ui->audioList->addItem(mpg->getstreaminfo(audiostream(a)).c_str());
    }
    expd->ui->audioList->item(0)->setSelected(true);

    if (!nogui) {
        expd->show();
        if (!expd->exec())
            return false;

        settings()->export_format = expd->ui->muxercombo->currentIndex();
        expfmt = expd->ui->muxercombo->currentIndex();

        expfilen=expd->ui->filenameline->text().toStdString();
        if (expfilen.empty())
            return false;
        expd->hide();
    } else if (exportformat >= 0 && exportformat < expd->ui->muxercombo->count()) {
        expfmt = exportformat;
    }

    selectedAudio = expd->ui->muxercombo->currentIndex();

    return true;
}

void tvclipper::createChapterList(std::string &chapterstring, std::string &chaptercolumn)
{
    if (!chapterlist.empty()) {
        int nchar = 0;
        char chapter[16];
        pts_t lastch = -1;
        for (std::list<pts_t>::const_iterator it=chapterlist.begin(); it!=chapterlist.end(); ++it)
            if (*it != lastch) {
                lastch = *it;
                // formatting the chapter string
                if (nchar > 0) {
                    nchar++;
                    chapterstring += ",";
                    chaptercolumn += "\n";
                }
                nchar += sprintf(chapter,"%02d:%02d:%02d.%03d",
                                 int(lastch/(3600*90000)),
                                 int(lastch/(60*90000))%60,
                                 int(lastch/90000)%60,
                                 int(lastch/90)%1000);
                // append chapter marks to lists for plain text / dvdauthor xml-file
                chapterstring += chapter;
                chaptercolumn += chapter;
            }
    }

    return;
}

bool tvclipper::exportMpgFile(int selectedAudio, int child_pid, int pipe_fds[], int expfmt, int ip, std::string expcmd, std::string chapterstring, std::string chaptercolumn, size_t pos)
{
    DlgProgress *prgwin = 0;
    logoutput *log;
    if (nogui) {
        log = new logoutput;
    } else {
        prgwin = new DlgProgress(this);
        prgwin->setWindowTitle(QString("export - " + QString::fromStdString(expfilen)));
        log = prgwin;
    }

    //   lavfmuxer mux(fmt,*mpg,outfilename);

    std::auto_ptr<muxer> mux;
    uint32_t audiostreammask(0);

    for(int a = 0;a < mpg->getaudiostreams(); a++)
        if (selectedAudio == a)
            audiostreammask |= 1u << a;

    std::string out_file = (child_pid < 0) ? expfilen :
                                             std::string("pipe:") + QString::number(pipe_fds[1]).toStdString();

    switch(expfmt) {
    case 1:
        mux=std::auto_ptr<muxer>(new mpegmuxer(audiostreammask,*mpg,out_file.c_str(),false,0));
        break;
    case 2:
        mux=std::auto_ptr<muxer>(new lavfmuxer("dvd",audiostreammask,*mpg,out_file.c_str()));
        break;
    case 3:
        mux=std::auto_ptr<muxer>(new lavfmuxer("mpegts",audiostreammask,*mpg,out_file.c_str()));
        break;
    case 0:
    default:
        mux=std::auto_ptr<muxer>(new mpegmuxer(audiostreammask,*mpg,out_file.c_str()));
        break;
    }

    if (!mux->ready()) {
#ifndef __WIN32__
        if (child_pid > 0) {
            ::close(pipe_fds[1]);
            int wstatus;
            while (waitpid(child_pid, &wstatus, 0)==-1 && errno==EINTR);
        }
#endif
        log->printerror("Unable to set up muxer!");
        if (nogui)
            delete log;
        else {
            prgwin->finish();
            delete prgwin;
        }
        return false;
    }

    // starting export, switch source to sequential mode
    buf.setsequential(cache_friendly);

    int startpic, stoppic, savedpic=0;
    pts_t startpts=(*mpg)[0].getpts(), stoppts, savedtime=0;

    for(unsigned int num=0; num<quick_picture_lookup.size(); num++) {
        startpic=quick_picture_lookup[num].startpicture;
        startpts=quick_picture_lookup[num].startpts;
        stoppic=quick_picture_lookup[num].stoppicture;
        stoppts=quick_picture_lookup[num].stoppts;

        log->printheading("%d. Exporting %d pictures: %s .. %s",
                          num+1,stoppic-startpic,ptsstring(startpts).c_str(),ptsstring(stoppts).c_str());
        mpg->savempg(*mux,startpic,stoppic,savedpic,quick_picture_lookup.back().outpicture,log);

        savedpic=quick_picture_lookup[num].outpicture;
        savedtime=quick_picture_lookup[num].outpts;
    }

    mux.reset();

    log->printheading("Saved %d pictures (%02d:%02d:%02d.%03d)",savedpic,
                      int(savedtime/(3600*90000)),
                      int(savedtime/(60*90000))%60,
                      int(savedtime/90000)%60,
                      int(savedtime/90)%1000	);

#ifndef __WIN32__
    if (child_pid > 0) {
        ::close(pipe_fds[1]);
        int wstatus;
        while (waitpid(child_pid, &wstatus, 0)==-1 && errno==EINTR);
    }

    // do some post processing if requested
    if (ip>=0 && !settings()->pipe_post[ip].isEmpty()) {
        expcmd = std::string(settings()->pipe_post[ip].toStdString());

        if ((pos=expcmd.find("%OUTPUT%"))!=std::string::npos)
            expcmd.replace(pos,8,expfilen);
        if ((pos=expcmd.find("%CHAPTERS%"))!=std::string::npos)
            expcmd.replace(pos,10,chapterstring);

        pos=expcmd.find(' ');
        std::string which="which "+expcmd.substr(0,pos)+" >/dev/null";

        log->print("");
        log->printheading("Performing post processing");
        int irc = system(which.c_str());

        if(irc!=0) {
            critical("Command not found - tvclipper","Problems with post processing command:\n"+expcmd.substr(0,pos));
            log->print("Command not found!");
        } else {
            int irc = system(expcmd.c_str());
            if(irc!=0) {
                critical("Post processing error - tvclipper","Post processing command:\n"+
                         QString::fromStdString(expcmd)+"\n returned non-zero exit code: " +QString::number(irc));
                log->print("Command reported some problems... please check!");
            }
            //else
            //  log->print("Everything seems to be OK...");
        }
    }
#endif

    // print plain list of chapter marks
    log->print("");
    log->printheading("Chapter list");
    log->print(chaptercolumn.c_str());

    // simple dvdauthor xml file with chapter marks
    std::string filename,destname;
    if (expfilen.rfind("/")<expfilen.length())
        filename=expfilen.substr(expfilen.rfind("/")+1);
    else
        filename=expfilen;
    destname=filename.substr(0,filename.rfind("."));
    log->print("");
    log->printheading("Simple XML-file for dvdauthor with chapter marks");
    log->print("<dvdauthor dest=\"%s\">",destname.c_str());
    log->print("  <vmgm />");
    log->print("  <titleset>");
    log->print("    <titles>");
    log->print("      <pgc>");
    log->print("        <vob file=\"%s\" chapters=\"%s\" />",filename.c_str(),chapterstring.c_str());
    log->print("      </pgc>");
    log->print("    </titles>");
    log->print("  </titleset>");
    log->print("</dvdauthor>");

    if (nogui)
        delete log;
    else {
        prgwin->finish();
        delete prgwin;
    }

    // done exporting, switch back to random mode
    buf.setsequential(false);

    return true;
}

void tvclipper::fileExport()
{
    int expfmt = 0, pipe_items_start = 0, selectedAudio = 0;
    std::string chapterstring = "", chaptercolumn = "";

    if (!saveExportDlgInfo(expfmt, pipe_items_start, selectedAudio))
        return;
    createChapterList(chapterstring, chaptercolumn);


    int child_pid = -1;
    int pipe_fds[2];
    std::string expcmd;
    size_t pos;
    int ip = expfmt - pipe_items_start;

#ifndef __WIN32__
    // check for piped output
    if(ip >= 0) {
        expfmt=settings()->pipe_format[ip];
        if (settings()->pipe_command[ip].contains('|'))
            expcmd = std::string(settings()->pipe_command[ip].toStdString());
        else
            expcmd = "|"+std::string(settings()->pipe_command[ip].toStdString());

        pos = expcmd.find("%OUTPUT%");
        if (pos != std::string::npos)
            expcmd.replace(pos, 8, expfilen);
    } else
        expcmd = expfilen;

    // chapter tag can also be used with input field pipes!
    pos = expcmd.find("%CHAPTERS%");
    if (pos != std::string::npos)
        expcmd.replace(pos,10,chapterstring);

    pos = expcmd.find('|');
    if (pos != std::string::npos) {
        pos++;
        size_t end = expcmd.find(' ',pos);
        //if (!QFileInfo(expcmd.substr(pos,end-pos)).exists() ||
        //    !QFileInfo(expcmd.substr(pos,end-pos)).isExecutable()) {
        // better test if command is found in $PATH, so one don't needs to give the full path name
        std::string which = "which " + expcmd.substr(pos,end-pos) + " >/dev/null";
        int irc = system(which.c_str());
        if(irc!=0) {
            critical("Command not found - tvclipper","Problems with piped output to:\n"+expcmd.substr(pos,end-pos));
            return;
        }

        if (::pipe(pipe_fds) < 0)
            return;
        child_pid = fork();
        if (child_pid == 0) {
            ::close(pipe_fds[1]);
            if (pipe_fds[0] != STDIN_FILENO) {
                dup2(pipe_fds[0], STDIN_FILENO);
            }
            //fprintf(stderr, "Executing %s\n", expcmd.c_str()+pos);
            for (int fd=0; fd<256; ++fd)
                if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO)
                    ::close(fd);
            execl("/bin/sh", "sh", "-c", expcmd.c_str()+pos, (char *)0);
            _exit(127);
        }
        ::close(pipe_fds[0]);
        if (child_pid < 0) {
            ::close(pipe_fds[1]);
            return;
        }
    } else
#endif
        if (QFileInfo(QString::fromStdString(expfilen)).exists()) {
            if (question("File exists - tvclipper", QString::fromStdString(expfilen) + "\nalready exists. Overwrite?") == QMessageBox::No)
                return;
        }

    exportMpgFile(selectedAudio, child_pid, pipe_fds, expfmt, ip, expcmd, chapterstring, chaptercolumn, pos);

    return;
}

void tvclipper::fileClose()
{
    close();
}

void tvclipper::addEventListItem(int pic, EventListItem::eventtype type)
{
    int newItemIndex = 0;
    QPixmap p;
    EventListItem *newItem;
    QListWidgetItem *item;

    //check if requested EventListItem is already in list to avoid doubles!
    for (int i = 0; (i < ui->eventlist->count()); i++) {
        item = ui->eventlist->item(i);
        if (typeid(*item) != typeid(EventListItem)) {
            continue;
        }
        EventListItem *eli = (EventListItem*) item;
        if (pic == eli->getpicture() && type == eli->geteventtype())
            return;
    }

    // create EventListItem
    if (imgp && imgp->rtti() == IMAGEPROVIDER_STANDARD)
        p.convertFromImage(imgp->getimage(pic));
    else
        p.convertFromImage(imageprovider(*mpg, new tvclipperbusy(this), false, 4).getimage(pic));

    newItem = new EventListItem(p, ui->eventlist->width(), type, pic, (*mpg)[pic].getpicturetype(),
                                (*mpg)[pic].getpts() - firstpts);

    // get index item
    for (int i = 0; (i <= ui->eventlist->count()); i++) {
        if (i >= ui->eventlist->count()) {
            newItemIndex = i;
            break;
        }

        item = ui->eventlist->item(i);
        if (typeid(*item) != typeid(EventListItem))
            continue;

        EventListItem *eli=(EventListItem*)item;

        if (*newItem < *eli) {
            newItemIndex = i;
            break;
        }
    }

    // add the item to eventlist
    ui->eventlist->insertItem(newItemIndex, newItem);
}

void tvclipper::editBookmark()
{
    addEventListItem(curpic, EventListItem::bookmark);
}


void tvclipper::editChapter()
{
    addEventListItem(curpic, EventListItem::chapter);
    update_quick_picture_lookup_table();
}


void tvclipper::editStop()
{
    addEventListItem(curpic, EventListItem::stop);
    update_quick_picture_lookup_table();
}


void tvclipper::editStart()
{
    addEventListItem(curpic, EventListItem::start);
    update_quick_picture_lookup_table();
}

void tvclipper::editAutoChapters()
{
    int inpic, chapters = 0;
    quick_picture_lookup_t::iterator it;
    QImage p1, p2;

    // the first chapter at 0sec is ALWAYS set by default from update_quick_picture_lookup_table()
    int chapter_start;
    if(settings()->chapter_interval>0) {
        chapter_start = settings()->chapter_interval;  // fixed length of intervals
    } else {
        chapter_start = quick_picture_lookup.back().outpicture/(1-settings()->chapter_interval);  // given number of chapters
        chapter_start = chapter_start > settings()->chapter_minimum ? chapter_start : settings()->chapter_minimum;
    }
    // don't make a new chapter if it would be shorter than the specified minimal length
    int chapter_max = quick_picture_lookup.back().outpicture - settings()->chapter_minimum;

    for(int outpic = chapter_start; outpic < chapter_max; outpic+=chapter_start)
        if (!quick_picture_lookup.empty()) {
            // find the entry in the quick_picture_lookup table that corresponds to given output picture
            it = std::upper_bound(quick_picture_lookup.begin(),quick_picture_lookup.end(),outpic,quick_picture_lookup_s::cmp_outpicture());
            inpic = outpic - it->outpicture + it->stoppicture;

            if(inpic+settings()->chapter_tolerance>it->stoppicture) {
                if(it == quick_picture_lookup.end()) break;
                // take begin of next START/STOP range as chapter picture if to near at end of current range
                it++;
                inpic=it->startpicture;
            } else if(settings()->chapter_tolerance>0) {
                // look for the next scene change inside specified frame tolerance (VERY SLOW!!!)
                if (!imgp)
                    imgp = new imageprovider(*mpg, new tvclipperbusy(this), false, viewscalefactor, settings()->chapter_tolerance);
                p2 = imgp->getimage(inpic,fine);
                for(int pic=inpic+1; pic<inpic+settings()->chapter_tolerance && pic<pictures; pic++) {
                    // get next picture
                    p1 = p2;
                    p2 = imgp->getimage(pic,fine);
                    if (p2.size()!=p1.size())
                        p2=p2.scaled(p1.size());

                    // calculate color distance between two consecutive frames
                    double dist=0.;
                    if (p2.depth()==32 && p1.depth()==32)
                        for (int y=0;y<p1.height();++y) {
                            QRgb *col1=(QRgb*)p1.scanLine(y);
                            QRgb *col2=(QRgb*)p2.scanLine(y);

                            for (int x=p1.width();x>0;--x) {
                                dist+=sqrt(pow(qRed(*col1)-qRed(*col2),2)+pow(qGreen(*col1)-qGreen(*col2),2)+pow(qBlue(*col1)-qBlue(*col2),2));
                                // that's a bit faster...
                                //dist+=(abs(qRed(*col1)-qRed(*col2))+abs(qGreen(*col1)-qGreen(*col2))+abs(qBlue(*col1)-qBlue(*col2)));
                                ++col1;
                                ++col2;
                            }
                        }
                    dist/=(p1.height()*p1.width());

                    // 50. seems to be a good measure for the color distance at scene changes (about sqrt(3)*50. if sum of abs values)!
                    //fprintf(stderr,"%d, DIST=%f\n",pic,dist);
                    if(dist>settings()->chapter_threshold) {
                        inpic=pic;
                        statusBar()->showMessage(QString().sprintf("%d. Scene change @ %d, DIST=%f\n",chapters+1,inpic,dist));
                        break;
                    }
                }
            }

            addEventListItem(inpic, EventListItem::chapter);
            chapters++;
        }

    if (chapters)
        update_quick_picture_lookup_table();

}

void tvclipper::editSuggest()
{
    int pic = 0, found=0;
    while ((pic = mpg->nextaspectdiscontinuity(pic)) >= 0) {
        addEventListItem(pic, EventListItem::bookmark);
        found++;
    }
    if (!found)
        statusBar()->showMessage(QString("*** No aspect ratio changes detected! ***"));
}

void tvclipper::editImport()
{
    int found=0;
    std::vector<int> bookmarks = mpg->getbookmarks();
    for (std::vector<int>::iterator b = bookmarks.begin(); b != bookmarks.end(); ++b) {
        addEventListItem(*b, EventListItem::bookmark);
        found++;
    }
    if (!found)
        statusBar()->showMessage(QString("*** No valid bookmarks available/found! ***"));
}


void tvclipper::abouttoshoweditconvert()
{
    editconvertpopup->clear();
    editconvertpopup->addAction(QString("START / STOP"));
    editconvertpopup->addAction(QString("STOP / START"));
    editconvertpopup->addAction(QString("4 : 3"));
    editconvertpopup->addAction(QString("16 : 9"));
}

void tvclipper::editConvert(QAction *action)
{
    int option = this->editconvertpopup->actions().indexOf(action);
    // convert Bookmarks to START/STOP markers
    if(option<0 || option>3) return;

    int found=0;
    std::vector<int> cutlist;
    {
        QListWidgetItem *item;
        for (int i = 0; (i < ui->eventlist->count()); i++) {
            item = ui->eventlist->item(i);

            if (typeid(*item) == typeid(EventListItem)) {
                EventListItem *eli=(EventListItem*)item;
                if (eli->geteventtype()==EventListItem::bookmark) {
                    cutlist.push_back(eli->getpicture());
                    delete item;
                    found++;
                }
            }
        }
    }

    if (found) {
        addStartStopItems(cutlist, option);

        if (found%2)
            statusBar()->showMessage(QString("*** No matching stop marker!!! ***"));
    }
    else
        statusBar()->showMessage(QString("*** No bookmarks to convert! ***"));
}

void tvclipper::addStartStopItems(std::vector<int> cutlist, int option)
{
    // take list of frame numbers and set alternating START/STOP markers
    bool alternate=true;
    EventListItem::eventtype type=EventListItem::start;
    if(option==1)
        type=EventListItem::stop;
    else if(option==2 || option==3)
        alternate=false;

    // make sure list is sorted...
    std::sort(cutlist.begin(),cutlist.end());

    // ...AND there are no old START/STOP pairs!!!
    {
        QListWidgetItem *item;
        for (int i = 0; (i < ui->eventlist->count()); i++) {
            item = ui->eventlist->item(i);

            if (typeid(*item) == typeid(EventListItem)) {
                EventListItem *eli=(EventListItem*)item;
                if (eli->geteventtype()==EventListItem::start || eli->geteventtype()==EventListItem::stop)
                    delete item;
            }
        }
    }

    for (std::vector<int>::iterator it = cutlist.begin(); it != cutlist.end(); ++it) {
        if(!alternate) {
            // set START/STOP according aspect ratio (2=4:3, 3=16:9)
            if (option == (*mpg)[*it].getaspectratio())
                type=EventListItem::start;
            else
                type=EventListItem::stop;
        }

        addEventListItem(*it, type);

        if(alternate) {
            // set START/STOP alternatingly
            if(type==EventListItem::start)
                type=EventListItem::stop;
            else
                type=EventListItem::start;
        }
    }

    update_quick_picture_lookup_table();
}

void tvclipper::viewDifference()
{
    ui->viewNormalAction->setChecked(false);
    ui->viewUnscaledAction->setChecked(false);
    ui->viewDifferenceAction->setChecked(true);

    if (imgp)
        delete imgp;
    imgp=new differenceimageprovider(*mpg,curpic, new tvclipperbusy(this), false, viewscalefactor);
    updateimagedisplay();
}


void tvclipper::viewUnscaled()
{
    ui->viewNormalAction->setChecked(false);
    ui->viewUnscaledAction->setChecked(true);
    ui->viewDifferenceAction->setChecked(false);

    if (!imgp || imgp->rtti()!=IMAGEPROVIDER_UNSCALED) {
        if (imgp)
            delete imgp;
        imgp=new imageprovider(*mpg,new tvclipperbusy(this),true,viewscalefactor);
        updateimagedisplay();
    }
}


void tvclipper::viewNormal()
{
    ui->viewNormalAction->setChecked(true);
    ui->viewUnscaledAction->setChecked(false);
    ui->viewDifferenceAction->setChecked(false);

    if (!imgp || imgp->rtti()!=IMAGEPROVIDER_STANDARD) {
        if (imgp)
            delete imgp;
        imgp=new imageprovider(*mpg,new tvclipperbusy(this),false,viewscalefactor);
        updateimagedisplay();
    }
}

void tvclipper::zoomIn()
{
    setviewscalefactor(viewscalefactor / 1.2);
}

void tvclipper::zoomOut()
{
    setviewscalefactor(viewscalefactor * 1.2);
}

void tvclipper::viewFullSize()
{
    setviewscalefactor(1.0);
}

void tvclipper::viewHalfSize()
{
    setviewscalefactor(2.0);
}

void tvclipper::viewQuarterSize()
{
    setviewscalefactor(4.0);
}

void tvclipper::viewCustomSize()
{
    setviewscalefactor(settings()->viewscalefactor_custom);
}

void tvclipper::playPlay()
{
    if (mplayer_process)
        return;


    ui->eventlist->setEnabled(false);
    ui->linslider->setEnabled(false);
    ui->jogslider->setEnabled(false);
    ui->gobutton->setEnabled(false);
    ui->goinput->setEnabled(false);
    ui->gobutton2->setEnabled(false);
    ui->goinput2->setEnabled(false);

#ifdef HAVE_LIB_AO

    ui->playAudio1Action->setEnabled(false);
    ui->playAudio2Action->setEnabled(false);
#endif // HAVE_LIB_AO

    ui->playPlayAction->setEnabled(false);
    ui->playStopAction->setEnabled(true);
    audiotrackmenu->setEnabled(false);

    ui->fileOpenAction->setEnabled(false);
    ui->fileSaveAction->setEnabled(false);
    ui->fileSaveAsAction->setEnabled(false);
    ui->snapshotSaveAction->setEnabled(false);
    ui->chapterSnapshotsSaveAction->setEnabled(false);
    ui->fileExportAction->setEnabled(false);

    showimage=false;
    ui->imagedisplay->setPixmap(QPixmap());
    ui->imagedisplay->grabKeyboard();

    fine=true;
    ui->linslider->setValue(mpg->lastiframe(curpic));
    tvclipper_off_t offset=(*mpg)[curpic].getpos().packetposition();
    mplayer_curpts=(*mpg)[curpic].getpts();

    QString process("mplayer");
    QStringList arguments;

    arguments << "-noconsolecontrols";
    tvclipper_off_t partoffset;
    int partindex = buf.getfilenum(offset, partoffset);
    if (partindex == -1)
        return;	// what else can we do?

#ifdef __WIN32__
    arguments << "-vo" << "directx:noaccel";
#endif
    arguments << "-wid" << QString().sprintf("0x%x",int(ui->imagedisplay->winId()));
    arguments << "-sb" << QString::number(offset - partoffset);
    arguments << "-geometry" << QString().sprintf("%dx%d+0+0",int(ui->imagedisplay->width()),int(ui->imagedisplay->height()));

    if (currentaudiotrack>=0 && currentaudiotrack<mpg->getaudiostreams()) {
        arguments << "-aid" << QString().sprintf("0x%x",int(mpg->mplayeraudioid(currentaudiotrack)));
    }

    // for now, pass all filenames from the current one up to the last one
    std::list<std::string>::const_iterator it = mpgfilen.begin();
    for (int i = 0; it != mpgfilen.end(); ++i, ++it)
        if (i >= partindex)
            arguments << QString::fromStdString(*it);

    mplayer_process=new QProcess(this);
    mplayer_process->setReadChannel(QProcess::StandardOutput);
    mplayer_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(mplayer_process, SIGNAL(finished(int)), this, SLOT(mplayer_exited()));
    connect(mplayer_process, SIGNAL(readyReadStandardOutput()), this, SLOT(mplayer_readstdout()));

    mplayer_success=false;
    mplayer_process->start(process, arguments);

    ui->imagedisplay->setFocus(Qt::OtherFocusReason);

    return;
}

void tvclipper::playStop()
{
    if (mplayer_process)
        mplayer_process->terminate();
    ui->imagedisplay->setFocus(Qt::OtherFocusReason);

    return;
}

void tvclipper::playAudio1()
{
#ifdef HAVE_LIB_AO
    qApp->processEvents();
    try
    {
        mpg->playaudio(currentaudiotrack,curpic,-2000);
    }
    catch (const tvclipper_exception &ex)
    {
        ex.show();
    }

    ui->imagedisplay->setFocus(Qt::OtherFocusReason);
#endif // HAVE_LIB_AO

    return;
}

void tvclipper::playAudio2()
{
#ifdef HAVE_LIB_AO
    qApp->processEvents();
    try
    {
        mpg->playaudio(currentaudiotrack,curpic,2000);
    }
    catch (const tvclipper_exception &ex)
    {
        ex.show();
    }

    ui->imagedisplay->setFocus(Qt::OtherFocusReason);
#endif // HAVE_LIB_AO

    return;
}

// **************************************************************************
// ***  slots

void tvclipper::linslidervalue(int newpic)
{
    if (!mpg || newpic==curpic)
        return;
    if (!fine)
        newpic=mpg->lastiframe(newpic);
    if (!jogsliding)
        jogmiddlepic=newpic;
    if (newpic==curpic)
        return;

    curpic=newpic;
    if (!jogsliding)
        jogmiddlepic=newpic;

    update_time_display();
    updateimagedisplay();

    return;
}

void tvclipper::jogsliderreleased()
{
    jogsliding=false;
    jogmiddlepic=curpic;
    ui->jogslider->setValue(0);
}

void tvclipper::jogslidervalue(int v)
{
    if (!mpg || (v==0 && curpic==jogmiddlepic))
        return;
    jogsliding=true;

    int relpic=0;

    /*
  if (v>jog_offset)
  relpic=int(exp(alpha*(v-jog_offset))+.5);
  else if (v<-jog_offset)
  relpic=-int(exp(alpha*(-v-jog_offset))+.5);
  */
    /*
  alternative function
  (fits better to external tick interval setting, because jog_offset
  only affects scale at small numbers AND range of 1 frame is NOT smaller
  than range of 0 and 2 frames as in old function!)
  */
    if (v>0) {
        relpic=int(exp(alpha*v)-settings()->jog_offset);
        if (relpic<0) relpic=0;
    }
    else if (v<0) {
        relpic=-int(exp(-alpha*v)-settings()->jog_offset);
        if (relpic>0) relpic=0;
    }

    int newpic=jogmiddlepic+relpic;
    if (newpic<0)
        newpic=0;
    else if (newpic>=pictures)
        newpic=pictures-1;

    if (relpic>=settings()->jog_threshold) {
        newpic=mpg->nextiframe(newpic);
        fine=false;
    } else if (relpic<=-settings()->jog_threshold) {
        fine=false;
    } else
        fine=true;

    if (curpic!=newpic)
        ui->linslider->setValue(newpic);

    fine=false;

    return;
}

void tvclipper::doubleclickedeventlist(QListWidgetItem *lbi)
{
    if (typeid(*lbi) != typeid(EventListItem))
        return;

    fine=true;
    ui->linslider->setValue(((EventListItem*)lbi)->getpicture());
    fine=false;
}

void tvclipper::actDelEventItem()
{
    QListWidgetItem *lbi = this->ui->eventlist->currentItem();
    if (!lbi)
        return;

    if (typeid(*lbi) != typeid(EventListItem))
        return;

    EventListItem *eli = (EventListItem*) lbi;

    EventListItem::eventtype type = eli->geteventtype();
    delete eli;
    if (type != EventListItem::bookmark)
        update_quick_picture_lookup_table();

    return;
}

void tvclipper::actGoToEventItem()
{
    QListWidgetItem *lbi = this->ui->eventlist->currentItem();
    if (!lbi)
        return;

    if (typeid(*lbi) != typeid(EventListItem))
        return;

    EventListItem *eli = (EventListItem*) lbi;

    fine = true;
    ui->linslider->setValue(eli->getpicture());
    fine = false;

    return;
}

void tvclipper::eventlistcontextmenu(const QPoint &point)
{
    QListWidgetItem *lbi = this->ui->eventlist->itemAt(point);
    if (!lbi)
        return;

    EventListItem *eli = dynamic_cast<EventListItem*>(lbi);
    if (eli == NULL)
        return;

    QListWidget *lb = lbi->listWidget();

    QMenu popup(lb);
    QList<QAction*>::const_iterator listEnd = lb->actions().end();
    if (!lb->actions().isEmpty()) {
        for (QList<QAction*>::const_iterator it = lb->actions().begin(); *it != *listEnd; it++) {
            popup.addAction(*it);
        }
    }
    QAction *actDelOthers = popup.addAction("Delete others");
    QAction *actDelAll = popup.addAction("Delete all");
    QAction *actDelStartStops = popup.addAction("Delete all start/stops");
    QAction *actDelCpts = popup.addAction("Delete all chapters");
    QAction *actDelBmks = popup.addAction("Delete all bookmarks");
    QAction *actConvToStart = popup.addAction("Convert to start marker");
    QAction *actConvToStop = popup.addAction("Convert to stop marker");
    QAction *actConvToCpt = popup.addAction("Convert to chapter marker");
    QAction *actConvToBmk = popup.addAction("Convert to bookmark");
    QAction *actDispDiff = popup.addAction("Display difference from this picture");

    EventListItem::eventtype cmptype = EventListItem::none, cmptype2 = EventListItem::none;

    QAction *currentAction = popup.exec(QCursor::pos());
    if (!currentAction) {
        return;
    }

    // these actions have slot triggered connected with a method
    if (lb->actions().contains(currentAction)) {
        return;
    }

    if (currentAction == actDelOthers)
    {
        QListWidgetItem *item;
        for (int i = lb->count() - 1; i > -1; i--) {
            item = lb->item(i);
            if (item != lbi)
                delete item;
        }

        update_quick_picture_lookup_table();
        return;
    }

    if (currentAction == actDelAll)
    {
        lb->clear();
        update_quick_picture_lookup_table();
        return;
    }

    if (currentAction == actConvToStart)
    {
        eli->setEventType(EventListItem::start);
        update_quick_picture_lookup_table();

        return;
    }

    if (currentAction == actConvToStop)
    {
        eli->setEventType(EventListItem::stop);
        update_quick_picture_lookup_table();
        return;
    }

    if (currentAction == actConvToCpt)
    {
        eli->setEventType(EventListItem::chapter);
        update_quick_picture_lookup_table();
        return;
    }

    if (currentAction == actConvToBmk)
    {
        eli->setEventType(EventListItem::bookmark);
        update_quick_picture_lookup_table();
        return;
    }

    if (currentAction == actDispDiff)
    {
        if (imgp)
            delete imgp;
        imgp=new differenceimageprovider(*mpg,eli->getpicture(),new tvclipperbusy(this),false,viewscalefactor);
        updateimagedisplay();
        ui->viewNormalAction->setChecked(false);
        ui->viewUnscaledAction->setChecked(false);
        ui->viewDifferenceAction->setChecked(true);
        return;
    }

    if (currentAction == actDelStartStops || currentAction == actDelBmks || currentAction == actDelCpts)
    {
        if (currentAction == actDelStartStops)
        {
            cmptype=EventListItem::start;
            cmptype2=EventListItem::stop;
        }
        if (currentAction == actDelBmks)
        {
            cmptype = EventListItem::bookmark;
        }
        if (currentAction == actDelCpts)
        {
            cmptype = EventListItem::chapter;
        }

        for (int i = lb->count() - 1; i > -1; i--)
        {
            EventListItem *item = (EventListItem*) lb->item(i);
            if (item->geteventtype() == cmptype || item->geteventtype() == cmptype2)
                delete item;
        }

        if (cmptype!=EventListItem::bookmark)
        {
            update_quick_picture_lookup_table();
        }
    }

    return;
}

void tvclipper::clickedgo()
{
    QString text=ui->goinput->text();
    text.trimmed();
    bool okay=false;
    int inpic;
    if (text.contains(':') || text.contains('.')) {
        okay=true;
        inpic=string2pts(text.toStdString())/getTimePerFrame();
    }
    else
        inpic=text.toInt(&okay,0);
    if (okay) {
        fine=true;
        ui->linslider->setValue(inpic);
        fine=false;
    }
    // ui->goinput->clear();

    ui->imagedisplay->setFocus();

}

void tvclipper::clickedgo2()
{
    QString text=ui->goinput2->text();
    text.trimmed();
    bool okay=false;
    int inpic, outpic;
    if (text.contains(':') || text.contains('.')) {
        okay=true;
        outpic=string2pts(text.toStdString())/getTimePerFrame();
    }
    else
        outpic=text.toInt(&okay,0);
    if (okay && !quick_picture_lookup.empty()) {
        // find the entry in the quick_picture_lookup table that corresponds to given output picture
        quick_picture_lookup_t::iterator it=
                std::upper_bound(quick_picture_lookup.begin(),quick_picture_lookup.end(),outpic,quick_picture_lookup_s::cmp_outpicture());
        inpic=outpic-it->outpicture+it->stoppicture;
        fine=true;
        ui->linslider->setValue(inpic);
        fine=false;
    }
    // ui->goinput2->clear();

    ui->imagedisplay->setFocus();
}

void tvclipper::mplayer_exited()
{
    if (mplayer_process) {
        if (!mplayer_success)// && !mplayer_process->normalExit())
        {
            DlgMplayerError *meb=new DlgMplayerError(this);
            meb->setText(mplayer_out);
        }
        mplayer_process->deleteLater();
        mplayer_process=0;
    }

    ui->eventlist->setEnabled(true);
    ui->linslider->setEnabled(true);
    ui->jogslider->setEnabled(true);
    ui->gobutton->setEnabled(true);
    ui->goinput->setEnabled(true);
    ui->eventlist->setEnabled(true);
    ui->linslider->setEnabled(true);
    ui->jogslider->setEnabled(true);
    ui->gobutton->setEnabled(true);
    ui->goinput->setEnabled(true);
    ui->gobutton2->setEnabled(true);
    ui->goinput2->setEnabled(true);

#ifdef HAVE_LIB_AO

    ui->playAudio1Action->setEnabled(true);
    ui->playAudio2Action->setEnabled(true);
#endif // HAVE_LIB_AO

    ui->playPlayAction->setEnabled(true);
    ui->playStopAction->setEnabled(false);
    audiotrackmenu->setEnabled(true);

    ui->fileOpenAction->setEnabled(true);
    ui->fileSaveAction->setEnabled(true);
    ui->fileSaveAsAction->setEnabled(true);
    ui->snapshotSaveAction->setEnabled(true);
    ui->chapterSnapshotsSaveAction->setEnabled(true);
    ui->fileExportAction->setEnabled(true);

    ui->imagedisplay->releaseKeyboard();

    int cp=curpic;
    jogmiddlepic=curpic;
    curpic=-1;
    showimage=true;
    fine=true;
    linslidervalue(cp);
    ui->linslider->setValue(cp);
    fine=false;
}

void tvclipper::mplayer_readstdout()
{
    if (!mplayer_process)
        return;

    QString line(mplayer_process->readAllStandardOutput());
    if (!line.length())
        return;

    if (!mplayer_success)
        mplayer_out += line;

    int pos=line.indexOf("V:");
    if (pos<0)
        return;

    line.remove(0,pos+2);
    line=line.trimmed();
    for(pos=0;pos<(signed)line.length();++pos)
        if ((line[pos]<'0' || line[pos]>'9')&&line[pos]!='.')
            break;
    line.truncate(pos);
    if (line.length()==0)
        return;

    bool okay;
    double d=line.toDouble(&okay);
    if (d==0 || !okay)
        return;

    if (!mplayer_success) {
        mplayer_success=true;
        mplayer_out.truncate(0);
    }
    pts_t pts=mplayer_ptsreference(pts_t(d*90000.),mplayer_curpts);


    if (pts==mplayer_curpts)
        return;

    int cp=curpic;

    if (pts>mplayer_curpts) {
        while (cp+1<pictures && pts>mplayer_curpts)
            mplayer_curpts=(*mpg)[++cp].getpts();
    } else if (pts<mplayer_curpts-18000) {
        while (cp>1 && mplayer_curpts>pts)
            mplayer_curpts=(*mpg)[--cp].getpts();
    }

    ui->linslider->setValue(cp);
}

void tvclipper::updateimagedisplay()
{
    if (showimage) {
        if (!imgp)
            imgp=new imageprovider(*mpg,new tvclipperbusy(this),false,viewscalefactor);
        QImage px=imgp->getimage(curpic,fine);
        ui->imagedisplay->setMinimumSize(px.size());
        ui->imagedisplay->setPixmap(QPixmap::fromImage(px));
        qApp->processEvents();
    }
}

void tvclipper::audiotrackchosen(QAction *action)
{
    int id = this->audiotrackpopup->actions().indexOf(action);
    if (id<0 || id>mpg->getaudiostreams())
        return;
    currentaudiotrack=id;
    for(int a=0;a<mpg->getaudiostreams();++a)
        audiotrackpopup->actions().at(a)->setChecked(a == id);
}

void tvclipper::abouttoshowrecentfiles()
{
    recentfilespopup->clear();
    QString menuitem;
    for(unsigned int id=0; id<settings()->recentfiles.size(); id++) {
        menuitem=QString(QString::fromStdString(settings()->recentfiles[id].first.front()));
        if(settings()->recentfiles[id].first.size()>1)
            menuitem += " ... (+" + QString::number(settings()->recentfiles[id].first.size()-1) + ")";
        recentfilespopup->addAction(menuitem);
    }
}

void tvclipper::loadrecentfile(QAction *action)
{
    int id = this->recentfilespopup->actions().indexOf(action);
    if (id<0 || id>=(signed)settings()->recentfiles.size())
        return;
    open(settings()->recentfiles[(unsigned)id].first, settings()->recentfiles[(unsigned)id].second);
}

void tvclipper::getFilesToOpen(std::list<std::string> &filenames)
{

    QStringList fn = QFileDialog::getOpenFileNames(ui->tvclipperWidget,
                                                   QString("Open files..."),
                                                   settings()->lastdir,
                                                   settings()->loadfilter,
                                                   NULL,
                                                   NULL);
    if (fn.empty()) {
        //      fprintf(stderr,"open(): QFileDialog::getOpenFileNames() returned EMPTY filelist!!!\n");
        //      fprintf(stderr,"        If you didn't saw a FileDialog, please check your 'lastdir' settings variable...");
        return;
    }
    for (QStringList::const_iterator it = fn.begin(); it != fn.end(); ++it)
        filenames.push_back(it->toStdString());

    // remember last directory if requested
    if (settings()->lastdir_update) {
        QFileInfo fileInfo = QFileInfo(fn.front());
        settings()->lastdir = fileInfo.absoluteDir().canonicalPath();
    }

    return;
}

QString tvclipper::getIdxFileName(QString idxfilename, QString mpgFilename)
{

    if (!idxfilename.isEmpty())
        return idxfilename;

    idxfilename = mpgFilename + ".idx";
    if (nogui)
        return idxfilename;

    QUrl u = QUrl::fromLocalFile(idxfilename);
    QString relname = u.toString(); // for file://file-path // u.path(); // for just file path
    QString s = QFileDialog::getSaveFileName(ui->tvclipperWidget,
                                             QString("Choose the name of the index file"),
                                             relname,
                                             settings()->idxfilter,
                                             NULL,
                                             NULL);
    if (s.isEmpty()) {
        if (mpg) {
            delete mpg;
            mpg = 0;
        }
        ui->fileOpenAction->setEnabled(true);
        return QString();
    }

    return s;
}

void tvclipper::freeMpgData()
{
    // if an MPEG file has already been opened, close it and reset the UI
    if (mpg) {
        delete mpg;
        mpg = 0;
    }

    curpic = ~0;
    if (imgp) {
        delete imgp;
        imgp = 0;
    }

    return;
}

void tvclipper::setUiForOpeningFile(bool afterOpening)
{
    ui->fileOpenAction->setEnabled(afterOpening);
    ui->fileSaveAction->setEnabled(afterOpening);
    ui->fileSaveAsAction->setEnabled(afterOpening);
    ui->snapshotSaveAction->setEnabled(afterOpening);
    ui->chapterSnapshotsSaveAction->setEnabled(afterOpening);
    ui->fileExportAction->setEnabled(afterOpening);
    ui->playPlayAction->setEnabled(afterOpening);
    audiotrackmenu->setEnabled(afterOpening);

    ui->editStartAction->setEnabled(afterOpening);
    ui->editStopAction->setEnabled(afterOpening);
    ui->editChapterAction->setEnabled(afterOpening);
    ui->editBookmarkAction->setEnabled(afterOpening);
    ui->editAutoChaptersAction->setEnabled(afterOpening);
    ui->editSuggestAction->setEnabled(afterOpening);
    ui->editImportAction->setEnabled(afterOpening);
    //editConvertAction->setEnabled(afterOpening);

    ui->viewNormalAction->setEnabled(afterOpening);
    ui->viewUnscaledAction->setEnabled(afterOpening);
    ui->viewDifferenceAction->setEnabled(afterOpening);

    ui->eventlist->setEnabled(afterOpening);
    ui->imagedisplay->setEnabled(afterOpening);
    ui->pictimelabel->setEnabled(afterOpening);
    ui->pictimelabel2->setEnabled(afterOpening);
    ui->picinfolabel->setEnabled(afterOpening);
    ui->picinfolabel2->setEnabled(afterOpening);
    ui->goinput->setEnabled(afterOpening);
    ui->gobutton->setEnabled(afterOpening);
    ui->goinput2->setEnabled(afterOpening);
    ui->gobutton2->setEnabled(afterOpening);
    ui->linslider->setEnabled(afterOpening);
    ui->jogslider->setEnabled(afterOpening);


#ifdef HAVE_LIB_AO

    if (afterOpening && mpg) {
        if (mpg->getaudiostreams()) {
            ui->playAudio1Action->setEnabled(true);
            ui->playAudio2Action->setEnabled(true);
        }
    } else {
        ui->playAudio1Action->setEnabled(false);
        ui->playAudio2Action->setEnabled(false);
    }
#endif // HAVE_LIB_AO

    if (afterOpening) {
        linslidervalue(0);

        for(int a = 0; a < mpg->getaudiostreams(); ++a) {
            audiotrackpopup->addAction(QString::fromStdString(mpg->getstreaminfo(audiostream(a))));
        }
        if (mpg->getaudiostreams()>0) {
            audiotrackpopup->actions().at(0)->setChecked(true);
            currentaudiotrack=0;
        } else {
            currentaudiotrack=-1;
        }

        ui->imagedisplay->setFocus(Qt::OtherFocusReason);
    } else {
        ui->playStopAction->setEnabled(false);
        ui->viewNormalAction->setChecked(true);
        ui->viewUnscaledAction->setChecked(false);
        ui->viewDifferenceAction->setChecked(false);

        audiotrackpopup->clear();
        ui->eventlist->clear();
        ui->imagedisplay->setBackgroundRole(QPalette::Window);
        ui->imagedisplay->setMinimumSize(QSize(0,0));
        ui->imagedisplay->setPixmap(QPixmap());
        ui->pictimelabel->clear();
        ui->pictimelabel2->clear();
        ui->picinfolabel->clear();
        ui->picinfolabel2->clear();
    }

    ui->linslider->setValue(0);
    ui->jogslider->setValue(0);

    return;
}

bool tvclipper::prepareToOpeninMpgFile(std::list<std::string> &filenames, std::string &idxfilename, std::string &expfilename, EventMarks &eventMarks)
{
    std::string filename;

    if (filenames.empty()) {
        getFilesToOpen(filenames);
    }

    // hmmmm,... do we really need this? With fn.empty() we never reach this line...
    if (filenames.empty())
      return false;

    prjfilen = "";
    make_canonical(filenames);

    filename = filenames.front();

    // a valid file name has been entered
    setWindowTitle(QString(VERSION_STRING " - " + QString::fromStdString(filename)));

    // reset inbuffer
    buf.reset();

    freeMpgData();

    setUiForOpeningFile(false);

    class XmlPrjFileReader *prjReader = new XmlPrjFileReader();
    if (prjReader->isXmlFile(QString::fromStdString(filename))) {
        QString errorStr;
        bool okay = prjReader->read(QString::fromStdString(filename), nogui, errorStr);
        if (!okay) {
            critical(PROGRAM_NAME " - Failed to read project file", QString::fromStdString(filename) + ":\n" + errorStr);
            ui->fileOpenAction->setEnabled(true);
            return false;
        }

        filenames = prjReader->mpgList;
        idxfilename = prjReader->idxFile.toStdString();
        expfilename = prjReader->expFile.toStdString();
        eventMarks = prjReader->eventMarks;
        {
            bool okay;
            exportformat = prjReader->expFormat.toInt(&okay);
            if (!okay)
                exportformat = 0;

        }

        prjfilen = filename;
        filename = filenames.front();
    }

    idxfilename = getIdxFileName(QString::fromStdString(idxfilename), QString::fromStdString(filename)).toStdString();
    if (idxfilename.empty())
        return false;
    make_canonical(idxfilename);

    return true;
}

// **************************************************************************
// ***  public functions

void tvclipper::open(std::list<std::string> filenames, std::string idxfilename, std::string expfilename)
{
    EventMarks eventMarks;
    std::string filename;
    if ( !prepareToOpeninMpgFile(filenames, idxfilename, expfilename, eventMarks) )
        return;

    filename = filenames.front();

    tvclipperbusy busy(this);
    busy.setbusy(true);

    {
        std::string errormessage;
        std::list<std::string>::const_iterator it = filenames.begin();
        while (it != filenames.end() && buf.open(*it, &errormessage))
            ++it;
        buf.setsequential(cache_friendly);
        if (it == filenames.end()) {
            mpg = mpgfile::open(&buf, &errormessage);
        }
        busy.setbusy(false);

        if (!mpg) {
            critical("Failed to open file - tvclipper", errormessage);
            ui->fileOpenAction->setEnabled(true);
            return;
        }
    }

    pictures=-1;

    if (!idxfilename.empty()) {
        std::string errorstring;
        busy.setbusy(true);
        pictures=mpg->loadindex(idxfilename.c_str(),&errorstring);
        int serrno=errno;
        busy.setbusy(false);
        if (nogui && pictures > 0)
            fprintf(stderr,"Loaded index with %d pictures!\n",pictures);
        if (pictures == -1 && serrno != ENOENT) {
            delete mpg;
            mpg=0;
            critical("Failed to open file - tvclipper",errorstring);
            ui->fileOpenAction->setEnabled(true);
            return;
        }
        if (pictures==-2) {
            delete mpg;
            mpg=0;
            critical("Invalid index file - tvclipper", errorstring);
            ui->fileOpenAction->setEnabled(true);
            return;
        }
        if (pictures<=-3) {
            delete mpg;
            mpg=0;
            critical("Index file mismatch - tvclipper", errorstring);
            ui->fileOpenAction->setEnabled(true);
            return;
        }
    }

    if (pictures < 0) {
        progressstatusbar psb(statusBar());
        psb.setprogress(500);
        psb.print("Indexing '%s'...",filename.c_str());
        std::string errorstring;
        busy.setbusy(true);
        pictures=mpg->generateindex(idxfilename.empty()?0:idxfilename.c_str(),&errorstring,&psb);
        busy.setbusy(false);
        if (nogui && pictures > 0)
            fprintf(stderr,"Generated index with %d pictures!\n",pictures);

        if (psb.cancelled()) {
            delete mpg;
            mpg=0;
            ui->fileOpenAction->setEnabled(true);
            return;
        }


        if (pictures<0) {
            delete mpg;
            mpg=0;
            critical("Error creating index - tvclipper",
                     QString("Cannot create index for\n")+QString::fromStdString(filename)+":\n"+QString::fromStdString(errorstring));
            ui->fileOpenAction->setEnabled(true);
            return;
        } else if (!errorstring.empty()) {
            critical("Error saving index file - tvclipper", QString::fromStdString(errorstring));
        }
    }

    if (pictures<1) {
        delete mpg;
        mpg=0;
        critical("Invalid MPEG file - tvclipper",
                 QString("The chosen file\n")+QString::fromStdString(filename)+"\ndoes not contain any video");
        ui->fileOpenAction->setEnabled(true);
        return;
    }

    // file loaded, switch to random mode
    buf.setsequential(false);

    mpgfilen=filenames;
    idxfilen=idxfilename;
    expfilen=expfilename;
    picfilen=QString::null;
    if (prjfilen.empty())
        addtorecentfiles(mpgfilen,idxfilen);
    else {
        std::list<std::string> dummy_list;
        dummy_list.push_back(prjfilen);
        addtorecentfiles(dummy_list);
    }

    firstpts=(*mpg)[0].getpts();
    timeperframe=(*mpg)[1].getpts()-(*mpg)[0].getpts();

    double fps=27.e6/double(mpgfile::frameratescr[(*mpg)[0].getframerate()]);
    ui->linslider->setMaximum(pictures-1);
    ui->linslider->setSingleStep(int(300*fps));
    ui->linslider->setPageStep(int(900*fps));
    if (settings()->lin_interval > 0)
        ui->linslider->setTickInterval(int(settings()->lin_interval*fps));

    //alpha=log(jog_maximum)/double(100000-jog_offset);
    // with alternative function
    alpha=log(settings()->jog_maximum)/100000.;
    if (settings()->jog_interval > 0 && settings()->jog_interval <= 100000)
        ui->jogslider->setTickInterval(int(100000/settings()->jog_interval));

    ui->imagedisplay->setBackgroundRole(QPalette::NoRole);
    curpic = 1;
    showimage = true;
    fine=false;
    jogsliding=false;
    jogmiddlepic=0;
    imgp=new imageprovider(*mpg,new tvclipperbusy(this),false,viewscalefactor);

    {
        EventListItem *eli=new EventListItem(QPixmap::fromImage(imgp->getimage(0)), 0,EventListItem::start, 9999999, 2, 0);
        ui->eventlist->setMinimumWidth(eli->sizeHint().width());
        delete eli;
    }

    if (!eventMarks.empty()) {
        for (EventMarks::const_iterator it = eventMarks.begin(); it != eventMarks.end(); it++)
        {
            bool okay = false;
            int picnum;
            if (it->picNumber.contains(':') || it->picNumber.contains('.')) {
                okay = true;
                picnum = string2pts(it->picNumber.toStdString()) / getTimePerFrame();
            } else {
                picnum = it->picNumber.toInt(&okay);
            }
            if (okay && picnum>=0 && picnum<pictures) {
                EventListItem *item = new EventListItem(QPixmap::fromImage(imgp->getimage(picnum)), ui->eventlist->sizeHint().width(), it->type, picnum, (*mpg)[picnum].getpicturetype(),(*mpg)[picnum].getpts()-firstpts, ui->eventlist);
                ui->eventlist->addItem(item);
            }
        }
    }

    update_quick_picture_lookup_table();

    setUiForOpeningFile(true);

    return;
}

// **************************************************************************
// ***  protected functions

void tvclipper::addtorecentfiles(const std::list<std::string> &filenames, const std::string &idxfilename)
{

    for(std::vector<std::pair<std::list<std::string>,std::string> >::iterator it=settings()->recentfiles.begin();
        it!=settings()->recentfiles.end();)
        // checking the size and the first/last filename should be enough... but maybe I'm to lazy! ;-)
        if (it->first.size()==filenames.size() && it->first.front()==filenames.front() && it->first.back()==filenames.back())
            it=settings()->recentfiles.erase(it);
        else
            ++it;

    settings()->recentfiles.insert(settings()->recentfiles.begin(),std::pair<std::list<std::string>,std::string>(filenames,idxfilename));

    while (settings()->recentfiles.size()>settings()->recentfiles_max)
        settings()->recentfiles.pop_back();
}

void tvclipper::setviewscalefactor(double factor)
{


    if (factor<=0.0)
        factor=1.0;
    ui->viewFullSizeAction->setChecked(factor==1.0);
    ui->viewHalfSizeAction->setChecked(factor==2.0);
    ui->viewQuarterSizeAction->setChecked(factor==4.0);
    ui->viewCustomSizeAction->setChecked(factor==settings()->viewscalefactor_custom);


    settings()->viewscalefactor = factor;


    if (factor!=viewscalefactor) {
        viewscalefactor=factor;
        if (imgp) {
            imgp->setviewscalefactor(factor);
            updateimagedisplay();
        }
    }


}

void tvclipper::keyReleaseEvent() {
    jogsliderreleased();
    return;
}

void tvclipper::keyPressEvent(QKeyEvent *keyEvent) {
    int newpos = -1;

    if (keyEvent->key() == Qt::Key_Home) {
        newpos = 0;
    }

    if (keyEvent->key() == Qt::Key_End) {
        newpos = pictures - 1;
    }

    if (keyEvent->key() == Qt::Key_PageDown) {
        newpos = curpic - ui->linslider->pageStep();
    }

    if (keyEvent->key() == Qt::Key_PageUp) {
        newpos = curpic + ui->linslider->pageStep();
    }

    if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) {
        int delta = settings()->wheel_delta;

        if (keyEvent->modifiers() & Qt::ShiftModifier)
            delta *= (WHEEL_INCR_SHIFT + 1);
        if (keyEvent->modifiers() & Qt::ControlModifier)
            delta *= (WHEEL_INCR_CTRL + 1);
        if (keyEvent->modifiers() & Qt::AltModifier)
            delta *= (WHEEL_INCR_ALT + 1);

        if (keyEvent->key() == Qt::Key_Left)
            delta = -delta;

        newpos = curpic + delta;
    }

    if (newpos == -1) {
        return;
    }

    if (newpos < 0)
        newpos = 0;
    if (newpos >= pictures)
        newpos = pictures - 1;

    ui->linslider->setValue(newpos);

    return;
}

void tvclipper::wheelEvent(QWheelEvent *wEvent) {
    if (ui->eventlist->rect().contains(wEvent->pos()))
        return;

    ui->linslider->setValue(curpic + wEvent->delta());

    return;
}

bool tvclipper::eventFilter(QObject *, QEvent *e) {
    if (e->type() != QEvent::KeyPress && e->type() != QEvent::KeyRelease && e->type() != QEvent::Wheel) {
        e->ignore();
        return false;
    }

    if (e->type() == QEvent::Wheel) {
        QWheelEvent *wEvent = static_cast<QWheelEvent *>(e);
        wheelEvent(wEvent);
        wEvent->accept();
        return true;
    }

    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        keyPressEvent(keyEvent);
        keyEvent->accept();
        return true;
    }

    if (e->type() == QEvent::KeyRelease) {
        keyReleaseEvent();
        e->accept();
        return true;
    }

    e->ignore();
    return false;
}

int tvclipper::question(const QString & caption, const QString & text)
{
    if (nogui) {
        fprintf(stderr, "%s\n%s\n(assuming No)\n", caption.toStdString().c_str(), text.toStdString().c_str());
        return QMessageBox::No;
    }
    return QMessageBox::question(this, caption, text, QMessageBox::Yes,
                                 QMessageBox::No | QMessageBox::Default | QMessageBox::Escape);
}

int tvclipper::critical(const QString & caption, const std::string & text) {
    return critical(caption,  QString::fromStdString(text));
}

int tvclipper::critical(const QString & caption, const QString & text)
{
    if (nogui) {
        fprintf(stderr, "%s\n%s\n(aborting)\n", caption.toStdString().c_str(), text.toStdString().c_str());
        return QMessageBox::Abort;
    }
    return QMessageBox::critical(this, caption, text,
                                 QMessageBox::Abort, QMessageBox::NoButton);
}

void tvclipper::make_canonical(std::string &filename)
{
    if (filename[0] != '/') {
        char resolved_path[PATH_MAX];
        char *rp = realpath(filename.c_str(), resolved_path);
        if (rp)
            filename = rp;
    }
}

void tvclipper::make_canonical(std::list<std::string> &filenames)
{
    std::list<std::string>::const_iterator it = filenames.begin();
    std::list<std::string> newlist;

    while (it != filenames.end()) {
        std::string tmp = *it;
        make_canonical(tmp);
        newlist.push_back(tmp);
        ++it;
    }
    filenames = newlist;
}

inline static QString timestr(pts_t pts)
{
    return QString().sprintf("%02d:%02d:%02d.%03d",
                             int(pts/(3600*90000)),
                             int(pts/(60*90000))%60,
                             int(pts/90000)%60,
                             int(pts/90)%1000);
}

void tvclipper::update_time_display()
{
    const mpgIndex::picture &idx=(*mpg)[curpic];
    const pts_t pts=idx.getpts()-firstpts;
    const char *AR[]={"forbidden","1:1","4:3","16:9","2.21:1","reserved"};
    const char *FR[]={"forbidden","23.976","24","25","29.97","30","50","59.94","60","reserved"};

    int outpic=0;
    pts_t outpts=0;
    QChar mark = ' ';

    // find the entry in the quick_picture_lookup table that corresponds to curpic
    quick_picture_lookup_t::iterator it=
            std::upper_bound(quick_picture_lookup.begin(),quick_picture_lookup.end(),curpic,quick_picture_lookup_s::cmp_picture());

    if (it!=quick_picture_lookup.begin())
    {
        // curpic is not before the first entry of the table
        --it;
        if (curpic < it->stoppicture)
        {
            // curpic is between (START and STOP[ pics of the current entry
            outpic=curpic-it->stoppicture+it->outpicture;
            outpts=pts-it->stoppts+it->outpts;
            mark = '*';
        }
        else
        {
            // curpic is after the STOP-1 pic of the current entry
            outpic=it->outpicture;
            outpts=it->outpts;
        }
    }

    QString curtime =
            QString(QChar(IDX_PICTYPE[idx.getpicturetype()]))
            + " " + timestr(pts);
    QString outtime =
            QString(mark) + " " + timestr(outpts);
    ui->pictimelabel->setText(curtime);
    ui->pictimelabel2->setText(outtime);
    ui->goinput->setText(QString::number(curpic));
    ui->goinput2->setText(QString::number(outpic));

    int res=idx.getresolution();	// are found video resolutions stored in index?
    if (res) {
        // new index with resolution bits set and lookup table at the end
        ui->picinfolabel->setText(QString::number(mpg->getwidth(res)) + "x"
                                  + QString::number(mpg->getheight(res)));
    } else {
        // in case of an old index file type (or if we don't want to change the index format/encoding?)
        // ==> get info directly from each image (which could be somewhat slower?!?)
        QImage p = imageprovider(*mpg, new tvclipperbusy(this), true).getimage(curpic,false);
        ui->picinfolabel->setText(QString::number(p.width()) + "x"
                                  + QString::number(p.height()));
    }
    ui->picinfolabel2->setText(QString(FR[idx.getframerate()]) + "fps, "
            + QString(AR[idx.getaspectratio()]));

}

void tvclipper::update_quick_picture_lookup_table() {
    // that's the (only) place where the event list should be scanned for
    // the exported pictures ranges, i.e. for START/STOP/CHAPTER markers!
    quick_picture_lookup.clear();
    chapterlist.clear();

    chapterlist.push_back(0);

    int startpic, stoppic, outpics = 0, lastchapter = -2;
    pts_t startpts, stoppts, outpts = 0;
    bool realzero = false;

    if(!nogui) {
        // overwrite CLI options
        start_bof = settings()->start_bof;
        stop_eof = settings()->stop_eof;
    }

    if (start_bof) {
        startpic = 0;
        startpts = (*mpg)[0].getpts() - firstpts;
    } else {
        startpic=-1;
        startpts=0;
    }

    {
        QListWidgetItem *item;
        for (int i = 0; (i < ui->eventlist->count()); i++) {
            item = ui->eventlist->item(i);

            if (typeid(*item) == typeid(EventListItem)) {
                const EventListItem &eli = *static_cast<const EventListItem*>(item);
                switch (eli.geteventtype()) {
                case EventListItem::start:
                    if (startpic < 0 || (start_bof && startpic == 0 && !realzero)) {
                        startpic = eli.getpicture();
                        startpts = eli.getpts();
                        if (startpic==0)
                            realzero=true;
                        // did we have a chapter in the eventlist directly before?
                        if (lastchapter == startpic)
                            chapterlist.push_back(outpts);
                    }
                    break;

                case EventListItem::stop:
                    if (startpic >= 0) {
                        stoppic = eli.getpicture();
                        stoppts = eli.getpts();
                        outpics += stoppic - startpic;
                        outpts += stoppts - startpts;

                        quick_picture_lookup.push_back(quick_picture_lookup_s(startpic, startpts, stoppic, stoppts, outpics, outpts));

                        startpic = -1;
                    }
                    break;

                case EventListItem::chapter:
                    lastchapter = eli.getpicture();
                    if (startpic >= 0)
                        chapterlist.push_back(eli.getpts() - startpts + outpts);
                    break;

                default:
                    break;
                }
            }
        }
    }

    // last item in list was a (real or virtual) START
    if (stop_eof && startpic>=0) {
        // create a new export range by adding a virtual STOP marker at EOF
        stoppic=pictures-1;
        stoppts=(*mpg)[stoppic].getpts()-firstpts;
        outpics+=stoppic-startpic;
        outpts+=stoppts-startpts;

        quick_picture_lookup.push_back(quick_picture_lookup_s(startpic,startpts,stoppic,stoppts,outpics,outpts));
    }

    update_time_display();
}

void tvclipper::helpAboutAction_activated()
{
    QMessageBox::about(this, tr(PROGRAM_NAME),
                       tr("<head></head><body><span style=\"font-family: Helvetica,Arial,sans-serif;\">"
                          "<p>%1</p>"
                          "eMail: <a href=\"mailto:lukas.vlcek777@gmail.com\">"
                          "lukas.vlcek777@gmail.com</a></p>"
                          "<p>This program is free software; you can redistribute it and/or "
                          "modify it under the terms of the GNU General Public License as "
                          "published by the Free Software Foundation; either version 2 of "
                          "the License, or (at your option) any later version. This "
                          "program is distributed in the hope that it will be useful, "
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU "
                          "General Public License for more details.</p>"
                          "<p>You should have received a copy of the GNU General Public License along "
                          "with this program; if not, see "
                          "<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.</p>"
                          "</span></body></html>").arg(VERSION_STRING));
}

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>

class helpDialog : public QDialog {
public:
    helpDialog(QWidget *parent, const char *name, QString file)
        : QDialog(parent)
    {
        this->setAccessibleName(name);
        vbox = new QVBoxLayout(this);
        vbox->setGeometry(QRect(0, 0, 640, 480));
        viewer = new QTextBrowser(this);
        hbox = new QHBoxLayout(this);
        prev = new QPushButton(tr("Prev"), this);
        next = new QPushButton(tr("Next"), this);
        home = new QPushButton(tr("Home"), this);
        close = new QPushButton(tr("Close"), this);
        close->setDefault(true);
        connect(prev, SIGNAL(clicked()), viewer, SLOT(backward()));
        connect(viewer, SIGNAL(backwardAvailable(bool)), prev, SLOT(setEnabled(bool)));
        connect(next, SIGNAL(clicked()), viewer, SLOT(forward()));
        connect(viewer, SIGNAL(forwardAvailable(bool)), next, SLOT(setEnabled(bool)));
        connect(home, SIGNAL(clicked()), viewer, SLOT(home()));
        connect(close, SIGNAL(clicked()), this, SLOT(accept()));
        viewer->setSource(file);
        setWindowTitle(tr("tvclipper help"));
        show();
    }
    virtual ~helpDialog() {
        delete prev;
        delete next;
        delete home;
        delete close;
        delete hbox;
        delete viewer;
        delete vbox;
    }
private:
    QVBoxLayout *vbox;
    QHBoxLayout *hbox;
    QTextBrowser *viewer;
    QPushButton *prev, *next, *home, *close;
};

void tvclipper::helpContentAction_activated()
{
    QDir appDir(qApp->applicationDirPath());
    // first search in the directory containing tvclipper
    QString helpFile = appDir.absolutePath() + "/tvclipper_en.html";
#ifndef __WIN32__
    // Unix/Linux: search in the associated share subdirectory
    if (!QFile::exists(helpFile)) {
        QString dirPath = appDir.absolutePath();
        helpFile = dirPath.remove(appDir.dirName()) + "/share/tvclipper/tvclipper_en.html";
    }
#endif
    if (QFile::exists(helpFile)) {
        helpDialog dlg(this, "helpDialog", helpFile);
        dlg.exec();
    }
    else {
        QMessageBox::information(this, tr(PROGRAM_NAME),
                                 tr("Help file %1 not available").arg(helpFile));
    }
}
