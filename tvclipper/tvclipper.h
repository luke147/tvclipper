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

#ifndef _TVCLIPPER_TVCLIPPER_H
#define _TVCLIPPER_TVCLIPPER_H

#include <QString>
#include <QVector>
#include <QList>
#include <ctime>
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

    struct quickPictureLookup_s
    {
        int startpicture;
        pts_t startpts;
        int stoppicture;
        pts_t stoppts;
        int outpicture;
        pts_t outpts;

        quickPictureLookup_s(int _startpicture, pts_t _startpts, int _stoppicture, pts_t _stoppts, int _outpicture, pts_t _outpts) :
            startpicture(_startpicture), startpts(_startpts), stoppicture(_stoppicture), stoppts(_stoppts), outpicture(_outpicture), outpts(_outpts)
        {
        }

        struct cmp_picture
        {
            bool operator()(int lhs, const quickPictureLookup_s &rhs) const
            {
                return lhs<rhs.startpicture;
            }
        };
        struct cmp_outpicture
        {
            bool operator()(int lhs, const quickPictureLookup_s &rhs) const
            {
                return lhs<rhs.outpicture;
            }
        };
    };
    typedef std::vector<quickPictureLookup_s> quickPictureLookup_t;

private:
    QString appName;
    QString orgName;
    int originalImageHeight;
    std::clock_t prevTimeForLinSlider;

private:
    void getFilesToOpen(QStringList &filenames);
    void freeMpgData();
    void setUiForOpeningFile(bool afterOpening);
    QString getIdxFileName(QString idxfilename, QString mpgFilename);
    bool prepareToOpeninMpgFile(QStringList &filenames, QString &idxfilename, QString &expfilename, EventMarks &eventMarks);
    bool saveExportDlgInfo(int &expfmt, int &pipe_items_start, int &selectedAudio);
    void createChapterList(QString &chapterstring, QString &chaptercolumn);
    bool exportMpgFile(int selectedAudio, int child_pid, int pipe_fds[], int expfmt, int ip, QString expcmd, QString chapterstring, QString chaptercolumn, int pos);
    void keyPressEvent(QKeyEvent *keyEvent);
    void keyReleaseEvent();
    void wheelEvent(QWheelEvent *wEvent);
    int getRequireImageHeight();

protected:
    quickPictureLookup_t quick_picture_lookup;
    QList<pts_t> chapterlist;

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
    QString prjfilen,idxfilen,expfilen;
    QString picfilen;
    QStringList mpgfilen;
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
    void addToRecentFiles(const QStringList &filenames, const QString &idxfilename = QString(""));
    void setViewScaleFactor(double factor);
    void setViewScaleFactor(int requireHeight);

    // special event handling (mouse wheel)
    bool eventFilter(QObject *, QEvent *e);

    void updateTimeDisplay();
    void updateQuickPictureLookupTable();

    // QMessagebox interface
    int question(const QString & caption, const QString & text);
    int critical(const QString & caption, const QString & text);

    // filename handling
    void makeCanonical(QString &filename);
    void makeCanonical(QStringList &filenames);

    // generic event item adder
    void addEventListItem(int pic, EventListItem::eventtype type);

    // save given (or current) picture (or select the best from a given number of samples inside a range)
    void snapshotSave(std::vector<int> piclist, int range=0, int samples=1);
    int chooseBestPicture(int startpic, int range, int smaples);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void resizeEvent(QResizeEvent *event);

protected slots:
    void helpAboutAction_activated();
    void helpContentAction_activated();

public:
    ~tvclipper();
    explicit tvclipper(QString orgName, QString appName, QWidget *parent = NULL, Qt::WindowFlags flags = Qt::Window);
    void open(QStringList filenames = QStringList(), QString idxfilename = QString(), QString expfilename = QString());
    void setbusy(bool b=true);
    void batchmode(bool b=true) { nogui = b; }
    void exportoptions(int format=0, bool bof=true, bool eof=true) { exportformat = format; start_bof=bof; stop_eof=eof; }
    // static tvclipper *New(std::string filename=std::string(), std::string idxfilename=std::string());
    void addStartStopItems(std::vector<int>, int option=0);
    int getTimePerFrame() { return timeperframe>0 && timeperframe<5000 ? timeperframe : 3003; }
    QString getNonExistFilename(QString filename, QString extWithDoth);

public slots:
    void fileNew();
    void fileOpen();
    void fileSaveAs();
    void fileSave();
    void snapshotSave();
    void chapterSnapshotsSave();
    void fileExport();
    void fileClose();
    void editBookmark();
    void editChapter();
    void editStop();
    void editStart();
    void editAutoChapters();
    void editSuggest();
    void editImport();
    void editConvert(QAction *action);
    void abouttoshoweditconvert();
    void viewDifference();
    void viewUnscaled();
    void viewNormal();
    void zoomIn();
    void zoomOut();
    void viewFullSize();
    void viewHalfSize();
    void viewQuarterSize();
    void viewCustomSize();
    void playAudio2();
    void playAudio1();
    void playStop();
    void playPlay();
    void jogsliderreleased();
    void jogslidervalue(int);
    void linslidervalue(int);
    void goToEventItemPic(QListWidgetItem *lbi);
    void eventlistcontextmenu(const QPoint &point);
    void mplayer_exited();
    void mplayer_readstdout();
    void clickedgo();
    void clickedgo2();
    void updateImageDisplay();
    void audiotrackchosen(QAction *action);
    void loadrecentfile(QAction *action);
    void abouttoshowrecentfiles();
    void delCurrentEventItem();
    void enteredEventList();
};

#endif // _TVCLIPPER_TVCLIPPER_H
