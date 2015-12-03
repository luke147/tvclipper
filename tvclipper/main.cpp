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

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <clocale>
#include <string>
#include <list>

#include <unistd.h>

#ifdef HAVE_LIB_AO
#include <ao/ao.h>
#endif // HAVE_LIB_AO

extern "C" {
#include <libavformat/avformat.h>
}

#include <QApplication>
#include <QImage>
#include <QTextCodec>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

#include "tvclipper.h"
#include "mpgfile.h"
#include "mpgIndex.h"
#include "version.h"
#include "settings.h"
#include "exception.h"


class AppParams {
public:
    bool generateidx;
    bool batchmode, start_bof, stop_eof;
    int exportformat;
    QString idxfilename, expfilename;
    std::vector<QString> cutlist;
    QStringList filenames;
    const uint npos = ~0;

    AppParams(void): generateidx(false), batchmode(false), start_bof(true), stop_eof(true), exportformat(0) {}

    size_t findFirstOfOrNotFind(const QString &text, const QString &chars, bool find)
    {
        bool founded = false;
        size_t pos = 0;

        for (QString::const_iterator it = text.constBegin(); it != text.constEnd(); it++, pos++)
        {
            if(chars.contains(*it) == find) {
                founded = true;
                break;
            }
        }
        if (!founded) {
            pos = ~0;
        }

        return pos;
    }
};

static char *argv0;

void usage_exit(int rv = 1) {
    fprintf(stderr,
        "%s\n"
        "\t%s -generateidx [-idx <idxfilename>] [<mpgfilename> ...]\n"
        "\t%s -batch [ OPTIONS ] <prjfilename> | <mpgfilename> ...\n\n"
        "%s\n"
        "\t-cut 4:3|16:9|TS|TS2|<list>, -exp <expfilename>,\n"
        "\t-format <num>, -automarker <num>\n\n",
        QObject::tr("Usage %1:").arg(VERSION_STRING).toStdString().c_str(), argv0, argv0, QObject::tr("Options:").toStdString().c_str());
    fprintf(stderr,
            QObject::tr("If no input files are specified, `tvclipper -generateidx' reads from\n"
                        "standard input.  By default, it also writes the index to standard\n"
                        "output, but you can specify another destination with `-idx'.\n\n").toStdString().c_str());
    fprintf(stderr,
            QObject::tr("In batch mode you can use `-cut' to keep only 4:3 resp. 16:9 frames or\n"
                        "create automatically alternating START/STOP cut markers for the bookmarks\n"
                        "imported from the input transport stream (TS, TS2) or for a given list of\n"
                        "frame numbers / time stamps (you can use any of ',-|;' as separators).\n"
                        "Without any (valid) cut markers the whole file will be converted!\n\n").toStdString().c_str());
    fprintf(stderr,
            QObject::tr("The -exp switch specifies the name of the exported file, with -format\n"
                        "the default export format (0=MPEG program stream/DVD) can be changed and\n"
                        "-automarker sets START/STOP markers at BOF/EOF (0=none,1=BOF,2=EOF,3=both).\n\n").toStdString().c_str());
    fprintf(stderr,
            QObject::tr("Options may be abbreviated as long as they remain unambiguous.\n\n").toStdString().c_str());
    exit(rv);
}

void printCopyright() {
    printf("%s Copyright (c) %d %s\n"
           "This program comes with ABSOLUTELY NO WARRANTY.\n"
           "This is free software, and you are welcome to redistribute it\n"
           "under certain conditions.\n", PROGRAM_NAME, COPYRIGHT_YEAR, AUTHOR_NAME);
    QFileInfo appFileInfo(argv0);
    QFileInfo copyingFileInfo(appFileInfo.absoluteDir().absolutePath() + "/" + DOCDIR + "/" + "COPYING");
    printf("\n%s is free software: you can redistribute it and/or modify\n"
           "it under the terms of the GNU General Public License as published by\n"
           "the Free Software Foundation, either version 3 of the License, or\n"
           "(at your option) any later version.\n\n"

           "%s is distributed in the hope that it will be useful,\n"
           "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
           "GNU General Public License for more details.\n\n"

           "You should have received a copy of the GNU General Public License\n"
           "along with %s. If not, see <http://www.gnu.org/licenses/>.\n\n"

           "For more details read file: %s\n\n"
           "%s is based on dvbcut.\n\n"

           "dvbcut\n"
           "Copyright (c) 2005 Sven Over <svenover@svenover.de>\n\n"

           "This program is free software; you can redistribute it and/or modify\n"
           "it under the terms of the GNU General Public License as published by\n"
           "the Free Software Foundation; either version 2 of the License, or\n"
           "(at your option) any later version.\n\n"

           "This program is distributed in the hope that it will be useful,\n"
           "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
           "GNU General Public License for more details.\n\n"

           "You should have received a copy of the GNU General Public License\n"
           "along with this program; if not, write to the Free Software\n"
           "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n", PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME, copyingFileInfo.canonicalFilePath().toStdString().c_str(), PROGRAM_NAME);
}

AppParams* readParams(int argc, char *argv[]) {
    class AppParams *appParams = new AppParams();
    /* --------------------- process arguments ----------------------------- */
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-' && argv[i][0] != '+') {
            // process input files
            // (that way files also can come first / options last and
            // argument processing only happens once and only in this loop!)
            appParams->filenames.push_back(argv[i]);
            continue;
        }

        // process switches / options
        size_t n = strlen(argv[i]);
        if (n == 2 && argv[i][1] == '-') {	// POSIX argument separator
            continue;
        }
        if (strncmp(argv[i], "-batch", n) == 0) {
            appParams->batchmode = true;
            continue;
        }
        if (strncmp(argv[i], "-generateidx", n) == 0) {
            appParams->generateidx = true;
            continue;
        }
        if (strncmp(argv[i], "-voracious", n) == 0) {
            tvclipper::cache_friendly = false;
            continue;
        }
        if (strncmp(argv[i], "-idx", n) == 0 && ++i < argc) {
            appParams->idxfilename = argv[i];
            continue;
        }
        if (strncmp(argv[i], "-exp", n) == 0 && ++i < argc) {
            appParams->expfilename = argv[i];
            continue;
        }
        if (strncmp(argv[i], "-format", n) == 0 && ++i < argc) {
            appParams->exportformat = atoi(argv[i]);
            continue;
        }
        if (strncmp(argv[i], "-automarker", n) == 0 && ++i < argc) {
            int bofeof = atoi(argv[i]);
            appParams->start_bof = (bofeof&1)==1;
            appParams->stop_eof  = (bofeof&2)==2;
            continue;
        }
        if (strncmp(argv[i], "-cut", n) == 0 && ++i < argc) {
            char *pch = strtok(argv[i],",-|;");
            while (pch) {
                if (strlen(pch))
                    appParams->cutlist.push_back(pch);
                pch = strtok(NULL,",-|;");
            }
            continue;
        }

        usage_exit();
    }

    return appParams;
}

int genIdx(QStringList filenames, QString idxfilename) {
    QString mpgfilename = "<stdin>";
    QString errormessage;
    inbuffer buf(8 << 20, 128 << 20);
    bool okay = true;
    int rv;

    if (filenames.empty()) {
        // no filenames given, read from stdin
        okay = buf.open(STDIN_FILENO, &errormessage);
    } else {
        mpgfilename = filenames.front();  // use first one (for now)
        if (idxfilename.isEmpty()) {
            idxfilename = mpgfilename + idxfile_ext_with_dot;
        }

        for (QStringList::iterator it = filenames.begin(); it != filenames.end(); it++) {
            bool o = buf.open(*it, &errormessage);
            if (!o)
                okay = o;
        }
    }

    if (!okay) {
        fprintf(stderr, "%s: %s\n", argv0, errormessage.toStdString().c_str());
        return 1;
    }
    buf.setsequential(tvclipper::cache_friendly);

    mpgfile *mpg = mpgfile::open(&buf, &errormessage);
    if (mpg == 0) {
        fprintf(stderr, "%s: %s\n", argv0, errormessage.toStdString().c_str());
        return 1;
    }

    mpgIndex idx(*mpg);
    int pics = idx.generate();
    if (pics <= 0) {
        fprintf(stderr, "%s: %s: %s\n", argv0, mpgfilename.toStdString().c_str(), (pics < 0) ? strerror(errno) : QObject::tr("no pictures found").toStdString().c_str());
        if (mpg)
            delete mpg;
        return 1;
    }

    if (idxfilename.isEmpty()) {
        rv = idx.save(STDOUT_FILENO);
        idxfilename = "<stdout>";
    } else {
        rv = idx.save(idxfilename.toStdString().c_str());
    }

    if (mpg)
        delete mpg;

    if (rv < 0) {
        fprintf(stderr, "%s: %s: %s\n", argv0, idxfilename.toStdString().c_str(), strerror(errno));
        return 1;
    }

    return 0;
}

void exportInBatchMode(tvclipper *mainWin, AppParams *appParams) {
    if (appParams->filenames.empty())	// must provide at least one filename
        usage_exit();
    mainWin->open(appParams->filenames, appParams->idxfilename, appParams->expfilename);
    if (!appParams->cutlist.empty()) {
        if (appParams->cutlist.front()=="AR") {  // obsolete (use 4:3 resp. 16:9 instead)! Or just in case of another AR...
            mainWin->editSuggest();
            QAction *action = mainWin->editconvertpopup->actions().at(0);
            mainWin->editConvert(action);
        } else if (appParams->cutlist.front()=="4:3") {
            mainWin->editSuggest();
            QAction *action = mainWin->editconvertpopup->actions().at(2);
            mainWin->editConvert(action);
        } else if(appParams->cutlist.front()=="16:9") {
            mainWin->editSuggest();
            QAction *action = mainWin->editconvertpopup->actions().at(3);
            mainWin->editConvert(action);
        } else if(appParams->cutlist.front()=="TS" || appParams->cutlist.front()=="TS1") {  // first bookmark is a START
            mainWin->editImport();
            QAction *action = mainWin->editconvertpopup->actions().at(0);
            mainWin->editConvert(action);
        } else if(appParams->cutlist.front()=="TS2") { // 2nd bookmark is a START
            mainWin->editImport();
            QAction *action = mainWin->editconvertpopup->actions().at(1);
            mainWin->editConvert(action);
        } else {
            std::vector<int> piclist, prob_item, prob_pos;
            unsigned int j = 0;
            for (std::vector<QString>::const_iterator iter = appParams->cutlist.begin(); iter != appParams->cutlist.end(); iter++) {
                size_t pos;
                pos = appParams->findFirstOfOrNotFind(*iter, "0123456789:./", false);
                if (pos == appParams->npos) {
                    if (appParams->findFirstOfOrNotFind(*iter, ":./", true) != appParams->npos)
                        piclist.push_back(string2pts(iter->toStdString())/mainWin->getTimePerFrame()); // pts divided by 3600(PAL) or 3003(NTSC)
                    else
                        piclist.push_back(iter->toInt());                       // integers are treated as frame numbers!
                } else {
                    prob_item.push_back(j);
                    prob_pos.push_back(pos);
                }

                if (piclist.size()%2)
                    fprintf(stderr, QObject::tr("*** Cut list contains an odd number of entries! ***\n").toStdString().c_str());
                if (!prob_item.empty()) {
                    fprintf(stderr, QObject::tr("*** Problems parsing parameter provided with option `-cut'! ***\n").toStdString().c_str());
                    for (j=0; j<prob_item.size(); j++) {
                        fprintf(stderr, QObject::tr("\t'%s' ==> discarded!\n").toStdString().c_str(),appParams->cutlist[prob_item[j]].toStdString().c_str());
                        for (int i = 0; i < (5 + prob_pos[j]); i++)
                            fprintf(stderr," ");
                        fprintf(stderr,"^\n");
                    }
                }
                mainWin->addStartStopItems(piclist);
            }
        }
    }
    mainWin->fileExport();
    return;
}

int main(int argc, char *argv[]) {
    int rv = 1;
    tvclipper *mainWin = NULL;

    argv0 = argv[0];

    setlocale(LC_ALL, "");

    AppParams *appParams = readParams(argc, argv);

    printf("%s\n", VERSION_STRING);
    printCopyright();
    printf("\n");

    /* ------------------------ sanity check --------------------------- */
    if (appParams->batchmode && appParams->generateidx)
        usage_exit();

    /* ----------------------- Generate mode --------------------------- */
    if (appParams->generateidx)
        return genIdx(appParams->filenames, appParams->idxfilename);

    /* ---------------------------- GUI and batch mode ------------------------------ */
    QApplication a(argc, argv);
    a.setOrganizationName(TVCLIPPER_QSETTINGS_ORGANIZATION);
    a.setOrganizationDomain(TVCLIPPER_QSETTINGS_DOMAIN);
    a.setApplicationName(TVCLIPPER_QSETTINGS_PRODUCT);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

#ifdef HAVE_LIB_AO
    ao_initialize();
#endif // HAVE_LIB_AO

    av_register_all();

    mainWin = new tvclipper(a.organizationName(), a.applicationName());
    mainWin->batchmode(appParams->batchmode);
    mainWin->exportoptions(appParams->exportformat, appParams->start_bof, appParams->stop_eof);

    if (appParams->batchmode) {
        exportInBatchMode(mainWin, appParams);
        rv = 0;
    } else {
        if (mainWin) {
            mainWin->show();

            if (!appParams->filenames.empty())
                mainWin->open(appParams->filenames, appParams->idxfilename, appParams->expfilename);

            a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
            rv = a.exec();
        }
    }

#ifdef HAVE_LIB_AO
    ao_shutdown();
#endif // HAVE_LIB_AO

    if (mainWin)
        delete mainWin;
    if (appParams)
        delete appParams;

    return rv;
}
