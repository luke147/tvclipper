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

/* $Id: tvclipper.h 165 2009-06-27 17:08:09Z too-tired $ */

#ifndef _DVBCUT_DVBCUT_H
#define _DVBCUT_DVBCUT_H

#include <string>
#include <vector>
#include <list>
#include "mpgfile.h"
#include "ui_tvclipper.h"
#include "pts.h"
#include "eventlistitem.h"
#include "xmlprjfile.h"

#include <QMainWindow>

namespace Ui {
class tvclipper;
}

class QProcess;
class imageprovider;

class tvclipper: public QMainWindow
{
    Q_OBJECT

public:
    static bool cache_friendly;
    QMenu *audiotrackpopup,*recentfilespopup,*editconvertpopup;

    struct quick_picture_lookup_s
    {
        int startpicture;
        pts_t startpts;
        int stoppicture;
        pts_t stoppts;
        int outpicture;
        pts_t outpts;

        quick_picture_lookup_s(int _startpicture, pts_t _startpts, int _stoppicture, pts_t _stoppts, int _outpicture, pts_t _outpts) :
            startpicture(_startpicture), startpts(_startpts), stoppicture(_stoppicture), stoppts(_stoppts), outpicture(_outpicture), outpts(_outpts)
        {
        }

        struct cmp_picture
        {
            bool operator()(int lhs, const quick_picture_lookup_s &rhs) const
            {
                return lhs<rhs.startpicture;
            }
        };
        struct cmp_outpicture
        {
            bool operator()(int lhs, const quick_picture_lookup_s &rhs) const
            {
                return lhs<rhs.outpicture;
            }
        };
    };
    typedef std::vector<quick_picture_lookup_s> quick_picture_lookup_t;

private:
    QString appName;
    QString orgName;

private:
    void getFilesToOpen(std::list<std::string> &filenames);
    void freeMpgData();
    void setUiForOpeningFile(bool afterOpening);
    QString getIdxFileName(QString idxfilename, QString mpgFilename);
    bool prepareToOpeninMpgFile(std::list<std::string> &filenames, std::string &idxfilename, std::string &expfilename, EventMarks &eventMarks);
    bool saveExportDlgInfo(int &expfmt, int &pipe_items_start, int &selectedAudio);
    void createChapterList(std::string &chapterstring, std::string &chaptercolumn);
    bool exportMpgFile(int selectedAudio, int child_pid, int pipe_fds[], int expfmt, int ip, std::string expcmd, std::string chapterstring, std::string chaptercolumn, size_t pos);
    void keyPressEvent(QKeyEvent *keyEvent);
    void keyReleaseEvent();
    void wheelEvent(QWheelEvent *wEvent);

protected:
    quick_picture_lookup_t quick_picture_lookup;
    std::list<pts_t> chapterlist;

    QAction* audiotrackmenu;
    inbuffer buf;
    mpgfile *mpg;
    int pictures;
    int curpic;
    double alpha;
    pts_t firstpts;
    int timeperframe;
    bool showimage;
    bool fine;
    bool jogsliding;
    int jogmiddlepic;
    std::string prjfilen,idxfilen,expfilen;
    QString picfilen;
    std::list<std::string> mpgfilen;
    QProcess *mplayer_process;
    bool mplayer_success;
    QString mplayer_out;
    pts_t mplayer_curpts;
    imageprovider *imgp;
    int busy;
    double viewscalefactor;
    int currentaudiotrack;
    bool nogui;
    int exportformat;
    bool start_bof;
    bool stop_eof;
    Ui_tvclipper *ui;

protected:
    //   QPixmap getpixmap(int picture, bool allgop=false);
    void exportvideo(const char *fmt);
    void addtorecentfiles(const std::list<std::string> &filenames, const std::string &idxfilename=std::string());
    void setviewscalefactor(double factor);

    // special event handling (mouse wheel)
    bool eventFilter(QObject *, QEvent *e);

    void update_time_display();
    void update_quick_picture_lookup_table();

    // QMessagebox interface
    int question(const QString & caption, const QString & text);
    int critical(const QString & caption, const QString & text);
    int critical(const QString & caption, const std::string & text);

    // filename handling
    void make_canonical(std::string &filename);
    void make_canonical(std::list<std::string> &filenames);

    // generic event item adder
    void addEventListItem(int pic, EventListItem::eventtype type);

    // save given (or current) picture (or select the best from a given number of samples inside a range)
    void snapshotSave(std::vector<int> piclist, int range=0, int samples=1);
    int chooseBestPicture(int startpic, int range, int smaples);

protected slots:
    virtual void helpAboutAction_activated();
    virtual void helpContentAction_activated();

public:
    ~tvclipper();
    explicit tvclipper(QString orgName, QString appName, QWidget *parent = NULL, Qt::WindowFlags flags = Qt::Window);
    void open(std::list<std::string> filenames = std::list<std::string>(), std::string idxfilename = std::string(), std::string expfilename = std::string());
    void setbusy(bool b=true);
    void batchmode(bool b=true) { nogui = b; }
    void exportoptions(int format=0, bool bof=true, bool eof=true) { exportformat = format; start_bof=bof; stop_eof=eof; }
    // static tvclipper *New(std::string filename=std::string(), std::string idxfilename=std::string());
    void addStartStopItems(std::vector<int>, int option=0);
    int getTimePerFrame() { return timeperframe>0 && timeperframe<5000 ? timeperframe : 3003; }

public slots:
    virtual void fileNew();
    virtual void fileOpen();
    virtual void fileSaveAs();
    virtual void fileSave();
    virtual void snapshotSave();
    virtual void chapterSnapshotsSave();
    virtual void fileExport();
    virtual void fileClose();
    virtual void editBookmark();
    virtual void editChapter();
    virtual void editStop();
    virtual void editStart();
    virtual void editAutoChapters();
    virtual void editSuggest();
    virtual void editImport();
    virtual void editConvert(QAction *action);
    virtual void abouttoshoweditconvert();
    virtual void viewDifference();
    virtual void viewUnscaled();
    virtual void viewNormal();
    virtual void zoomIn();
    virtual void zoomOut();
    virtual void viewFullSize();
    virtual void viewHalfSize();
    virtual void viewQuarterSize();
    virtual void viewCustomSize();
    virtual void playAudio2();
    virtual void playAudio1();
    virtual void playStop();
    virtual void playPlay();
    virtual void jogsliderreleased();
    virtual void jogslidervalue(int);
    virtual void linslidervalue(int);
    virtual void doubleclickedeventlist(QListWidgetItem *lbi);
    virtual void eventlistcontextmenu(const QPoint &point);
    virtual void mplayer_exited();
    virtual void mplayer_readstdout();
    virtual void clickedgo();
    virtual void clickedgo2();
    virtual void updateimagedisplay();
    virtual void audiotrackchosen(QAction *action);
    virtual void loadrecentfile(QAction *action);
    virtual void abouttoshowrecentfiles();
    void actDelEventItem();
    void actGoToEventItem();
};

#endif