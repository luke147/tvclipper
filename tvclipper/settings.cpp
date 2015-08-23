/*  tvclipper settings
    Copyright (c) 2006 - 2009 Michael Riepe
 
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

/* $Id: settings.cpp 172 2011-04-22 21:57:55Z too-tired $ */

#include <string>
#include <vector>

#include <qstringlist.h>

#include <assert.h>

#include <QApplication>
#include "defines.h"
#include "settings.h"

#warning zkontrolovat funkčnost celého načítání a ukládání nastavení

#define TVCLIPPER_QSETTINGS_PATH "/" TVCLIPPER_QSETTINGS_DOMAIN "/"

#define TVCLIPPER_DEFAULT_LOADFILTER \
	"Recognized files (*.tvclipper *.m2t *.mpg *.rec* *.ts *.tts* *.trp *.vdr);;" \
	"tvclipper project files (*.tvclipper);;" \
	"MPEG files (*.m2t *.mpg *.rec* *.ts *.tts* *.trp *.vdr);;" \
	"All files (*)"
#define TVCLIPPER_DEFAULT_IDXFILTER \
	"tvclipper index files (*.idx);;All files (*)"
#define TVCLIPPER_DEFAULT_PRJFILTER \
	"tvclipper project files (*.tvclipper);;All files (*)"

#define TVCLIPPER_DEFAULT_START_LABEL   "<font size=\"+1\" color=\"" START_COLOUR_MASK "\"><b>START</b></font>"
#define TVCLIPPER_DEFAULT_STOP_LABEL    "<font size=\"+1\" color=\"" STOP_COLOUR_MASK "\"><b>STOP</b></font>"
#define TVCLIPPER_DEFAULT_CHAPTER_LABEL "<font color=\"" CHAPTER_COLOUR_MASK "\">CHAPTER</font>"
#define TVCLIPPER_DEFAULT_BOOKMARK_LABEL "<font color=\"" BOOKMARK_COLOUR_MASK "\">BOOKMARK</font>"

#define TVCLIPPER_DEFAULT_PIPE_COMMAND "|dvdauthor -t -c '%CHAPTERS%' -v mpeg2 -o '%OUTPUT%' -"
#define TVCLIPPER_DEFAULT_PIPE_POST "dvdauthor -o '%OUTPUT%' -T"
#define TVCLIPPER_DEFAULT_PIPE_LABEL "DVD-Video titleset (dvdauthor)"
#define TVCLIPPER_DEFAULT_PIPE_FORMAT (0)
/* 
// SOME OTHER EXAMPLES for the settings file ~/.qt/tvclipper.sf.netrc 
// (ok, for time consuming conversions one does not save any time, but it may be convenient...) 
// 1. Conversion to mpeg4 avi-file with ffmpeg:
//    (to recode to a smaller MPEG2 File use "--target dvd -acodec copy"?)!
pipe/1/command=|ffmpeg -f mpeg2video -i - -f avi -vcodec mpeg4 -b 1200k -g 250 -bf 2 -acodec libmp3lame -ab 128k -ar 44100 '%OUTPUT%'
pipe/1/format=1
pipe/1/label=MPEG-4/ASP (ffmpeg)
pipe/1/post=
// 2. Shrinking with vamps by 20%, before piping to dvdauthor:
pipe/2/command=| vamps -E 1.2 -S 10000000000 -a 1,2,3 | dvdauthor -t -c '%CHAPTERS%' -v mpeg2 -o '%OUTPUT%' -
pipe/2/format=0
pipe/2/label=20% shrinked DVD-Video titleset (vamps & dvdauthor)
pipe/2/post=dvdauthor -o '%OUTPUT%' -T
// 3. recoding to a (smaller?) MPEG2 file with DVD compliant resolution (ca. 3000kbps):
pipe/3/command=|ffmpeg -f mpeg2video -i - --target dvd -qscale 3.0 -bf 2 -acodec copy '%OUTPUT%'"
pipe/3/format=1
pipe/3/label=recoded DVD compliant video (ffmpeg)
pipe/3/post=
*/

tvclipper_settings::tvclipper_settings(QString organization, QString application)
    : QSettings(organization, application)
{
    setPath(QSettings::NativeFormat, QSettings::UserScope, application);
    beginGroup(TVCLIPPER_QSETTINGS_PATH);
    loaded = false;
}

tvclipper_settings::~tvclipper_settings()
{
    if (loaded) {
        save_settings();
    }
    endGroup();
}

void tvclipper_settings::load_settings()
{
    int version = value("/version", 0).toInt();

    if (version >= 1) {
        // config format version 1 or later
        beginGroup("/wheel");
        wheel_increments[WHEEL_INCR_NORMAL] = value("/incr_normal", 25*60).toInt();
        wheel_increments[WHEEL_INCR_SHIFT] = value("/incr_shift", 25).toInt();
        wheel_increments[WHEEL_INCR_CTRL] = value("/incr_ctrl", 1).toInt();
        wheel_increments[WHEEL_INCR_ALT] = value("/incr_alt", 15*25*60).toInt();
        wheel_threshold = value("/threshold", 24).toInt();
        // Note: delta is a multiple of 120 (see Qt documentation)
        wheel_delta = value("/delta", 120).toInt();
        if (wheel_delta == 0)
            wheel_delta = 1;	// avoid devide by zero
        endGroup();	// wheel
        beginGroup("/slider");
        jog_maximum = value("/jog_maximum", 180000).toInt();
        jog_threshold = value("/jog_threshold", 50).toInt();
        // to increase the "zero frames"-region of the jog-slider
        jog_offset = value("/jog_offset", 0.4).toDouble();
        // sub-intervals of jog_maximum
        jog_interval = value("/jog_interval", 1).toInt();
        if (jog_interval < 0)
            jog_interval = 0;
        lin_interval = value("/lin_interval", 3600).toInt();
        if (lin_interval < 0)
            lin_interval = 0;
        endGroup();	// slider
        beginGroup("/lastdir");
        lastdir = value("/name", ".").toString();
        lastdir_update = value("/update", true).toBool();
        endGroup(); // lastdir
        beginGroup("/filter");
        idxfilter = value("/idxfilter", TVCLIPPER_DEFAULT_IDXFILTER).toString();
        prjfilter = value("/prjfilter", TVCLIPPER_DEFAULT_PRJFILTER).toString();
        loadfilter = value("/loadfilter", TVCLIPPER_DEFAULT_LOADFILTER).toString();
        endGroup();	// filter
    }
    else {
        // old (unnumbered) config format
        wheel_increments[WHEEL_INCR_NORMAL] = value("/wheel_incr_normal", 25*60).toInt();
        wheel_increments[WHEEL_INCR_SHIFT] = value("/wheel_incr_shift", 25).toInt();
        wheel_increments[WHEEL_INCR_CTRL] = value("/wheel_incr_ctrl", 1).toInt();
        wheel_increments[WHEEL_INCR_ALT] = value("/wheel_incr_alt", 15*25*60).toInt();
        wheel_threshold = value("/wheel_threshold", 24).toInt();
        // Note: delta is a multiple of 120 (see Qt documentation)
        wheel_delta = value("/wheel_delta", 120).toInt();
        if (wheel_delta == 0)
            wheel_delta = 1;	// avoid devide by zero
        jog_maximum = value("/jog_maximum", 180000).toInt();
        jog_threshold = value("/jog_threshold", 50).toInt();
        // to increase the "zero frames"-region of the jog-slider
        jog_offset = value("/jog_offset", 0.4).toDouble();
        // sub-intervals of jog_maximum
        jog_interval = value("/jog_interval", 1).toInt();
        if (jog_interval < 0)
            jog_interval = 0;
        lin_interval = value("/lin_interval", 3600).toInt();
        if (lin_interval < 0)
            lin_interval = 0;
        lastdir = value("/lastdir", ".").toString();
        lastdir_update = true;
        idxfilter = value("/idxfilter", TVCLIPPER_DEFAULT_IDXFILTER).toString();
        prjfilter = value("/prjfilter", TVCLIPPER_DEFAULT_PRJFILTER).toString();
        loadfilter = value("/loadfilter", TVCLIPPER_DEFAULT_LOADFILTER).toString();
        // remove old-style entries
        remove("/wheel_incr_normal");
        remove("/wheel_incr_shift");
        remove("/wheel_incr_ctrl");
        remove("/wheel_incr_alt");
        remove("/wheel_threshold");
        remove("/wheel_delta");
        remove("/jog_maximum");
        remove("/jog_threshold");
        remove("/jog_offset");
        remove("/jog_interval");
        remove("/lin_interval");
        remove("/lastdir");
        remove("/idxfilter");
        remove("/prjfilter");
        remove("/loadfilter");
    }
    if (version >= 2) {
        /* float view scale factor */
        beginGroup("/viewscalefactor");
        viewscalefactor = value("/current", 1.0).toDouble();
        viewscalefactor_custom = value("/custom", 3.0).toDouble();
        endGroup(); // viewscalefactor
    }
    else {
        viewscalefactor = (double)value("/viewscalefactor", 1).toInt();
        viewscalefactor_custom = 3.0;
        remove("/viewscalefactor");
    }
    export_format = value("/export_format", 0).toInt();
    beginGroup("/recentfiles");
    recentfiles_max = value("/max", 5).toInt();
    recentfiles.clear();
    std::list<std::string> filenames;
    QStringList keys = this->childKeys();
    for (unsigned int i = 0; i < recentfiles_max; ++i) {
        QString key = "/" + QString::number(i);
        if (version < 1 && keys.size()>1) {
            // OLD format (2 keys per input file, NO subkeys!)
            if (! this->contains(key))
                continue;
            QString filename = value(key).toString();
            if (filename.isEmpty())
                continue;
            filenames.clear();
            filenames.push_back(filename.toStdString());
            if (! this->contains(key + "-idx"))
                continue;
            QString idxfilename = value(key + "-idx").toString();
            recentfiles.push_back(
                        std::pair<std::list<std::string>,std::string>(filenames, idxfilename.toStdString()));
        }
        else {
            // NEW format with subkeys and multiple files!
            beginGroup(key);
            QString filename = value("/0").toString();
            if (!filename.isEmpty()) {
                // multiple input files?
                int j=0;
                filenames.clear();
                while(!filename.isEmpty()) {
                    filenames.push_back(filename.toStdString());
                    filename = value("/" + QString::number(++j), "").toString();
                }
                if (! this->contains("/idx"))
                    continue;
                QString idxfilename = value("/idx").toString();
                recentfiles.push_back(
                            std::pair<std::list<std::string>,std::string>(filenames, idxfilename.toStdString()));
            }
            endGroup();	// key
        }
    }
    endGroup();	// recentfiles
    beginGroup("/labels");
    start_label = value("/start", TVCLIPPER_DEFAULT_START_LABEL).toString();
    stop_label = value("/stop", TVCLIPPER_DEFAULT_STOP_LABEL).toString();
    chapter_label = value("/chapter", TVCLIPPER_DEFAULT_CHAPTER_LABEL).toString();
    bookmark_label = value("/bookmark", TVCLIPPER_DEFAULT_BOOKMARK_LABEL).toString();
    endGroup();	// labels
    start_bof = value("/start_bof", true).toBool();
    stop_eof = value("/stop_eof", true).toBool();
    beginGroup("/snapshots");
    snapshot_type = value("/type", "PNG").toString();
    snapshot_quality = value("/quality", -1).toInt();
    snapshot_prefix = value("/prefix", "").toString();
    snapshot_delimiter = value("/delimiter", "_").toString();
    snapshot_first = value("/first", 1).toInt();
    snapshot_width = value("/width", 3).toInt();
    snapshot_extension = value("/extension", "png").toString();
    snapshot_range = value("/range", 0).toInt();
    snapshot_samples = value("/samples", 1).toInt();
    endGroup();	// snapshots
    beginGroup("/pipe");
    pipe_command.clear();
    pipe_post.clear();
    pipe_label.clear();
    pipe_format.clear();
    beginGroup("/0");
    QString command = value("/command", TVCLIPPER_DEFAULT_PIPE_COMMAND).toString();
    QString post = value("/post", TVCLIPPER_DEFAULT_PIPE_POST).toString();
    QString label = value("/label", TVCLIPPER_DEFAULT_PIPE_LABEL).toString();
    int format = value("/format", TVCLIPPER_DEFAULT_PIPE_FORMAT).toInt();
    endGroup();	// 0
    unsigned int i = 0;

    while(!command.isEmpty() && !label.isEmpty()) {
        if(format<0 || format>3) format = 0;
        pipe_command.push_back(command);
        pipe_post.push_back(post);
        pipe_label.push_back(label);
        pipe_format.push_back(format);
        QString key = "/" + QString::number(++i);
        beginGroup(key);
        command = value("/command", QVariant("")).toString();
        post = value("/post", QVariant("")).toString();
        label = this->value("/label", QVariant("")).toString();
        format = value("/format", QVariant(0)).toInt();
        endGroup();	// key
    }
    endGroup();	// pipe
    beginGroup("/chapters");
    // length (>0) or number (<0) of chapter(s)
    chapter_interval = value("/interval", 600*25).toInt();
    // detection of scene changes is rather time comsuming...
    //chapter_tolerance = readNumEntry("/tolerance", 10*25);
    //... better switch it off per default!
    chapter_tolerance = value("/tolerance", 0).toInt();
    // average color distance needed for a scene change
    chapter_threshold = value("/threshold", 50.).toDouble();
    // minimal length of a chapter
    chapter_minimum = value("/minimum", 200*25).toInt();
    endGroup();	// auto chapters
}

void
tvclipper_settings::save_settings() {
    setValue("/version", 2);	// latest config version
    beginGroup("/wheel");
    setValue("/incr_normal", wheel_increments[WHEEL_INCR_NORMAL]);
    setValue("/incr_shift", wheel_increments[WHEEL_INCR_SHIFT]);
    setValue("/incr_ctrl", wheel_increments[WHEEL_INCR_CTRL]);
    setValue("/incr_alt", wheel_increments[WHEEL_INCR_ALT]);
    setValue("/threshold", wheel_threshold);
    setValue("/delta", wheel_delta);
    endGroup();	// wheel
    beginGroup("/slider");
    setValue("/jog_maximum", jog_maximum);
    setValue("/jog_threshold", jog_threshold);
    setValue("/jog_offset", jog_offset);
    setValue("/jog_interval", jog_interval);
    setValue("/lin_interval", lin_interval);
    endGroup();	// slider
    beginGroup("/lastdir");
    setValue("/name", lastdir);
    setValue("/update", lastdir_update);
    endGroup();	// lastdir
    beginGroup("/filter");
    setValue("/idxfilter", idxfilter);
    setValue("/prjfilter", prjfilter);
    setValue("/loadfilter", loadfilter);
    endGroup();	// filter
    beginGroup("/viewscalefactor");
    setValue("/current", viewscalefactor);
    setValue("/custom", viewscalefactor_custom);
    endGroup();	// viewscalefactor
    setValue("/export_format", export_format);
    beginGroup("/recentfiles");
    // first remove any OLD recentfiles entries to clean the settings file (<revision 108)!!!
    QStringList keys = this->childKeys();
    for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it )
        remove("/" + *it);
    // then remove ALL new recentfiles entries!!!
    // (otherwise it would be a mess with erased&inserted muliple file entries of different size)
    this->beginGroup("/");
    QStringList subkeys = this->childKeys();
    this->endGroup(); // /
    for ( QStringList::Iterator its = subkeys.begin(); its != subkeys.end(); ++its ) {
        this->beginGroup("/" + *its);
        QStringList keys = childKeys();
        this->endGroup();
        for ( QStringList::Iterator itk = keys.begin(); itk != keys.end(); ++itk )
            remove("/"  + *its + "/" + *itk);
    }
    setValue("/max", int(recentfiles_max));
    // and NOW write the updated list from scratch!!!
    for (unsigned int i = 0; i < recentfiles.size(); ++i) {
        QString key = "/" + QString::number(i);
        beginGroup(key);
        int j=0;
        for(std::list<std::string>::iterator it=settings()->recentfiles[i].first.begin();
            it!=settings()->recentfiles[i].first.end(); it++, j++)
            setValue("/" + QString::number(j), QString::fromStdString(*it));
        setValue("/idx", QString::fromStdString(recentfiles[i].second));
        endGroup();	// key
    }
    endGroup();	// recentfiles
    beginGroup("/labels");
    setValue("/start", start_label);
    setValue("/stop", stop_label);
    setValue("/chapter", chapter_label);
    setValue("/bookmark", bookmark_label);
    endGroup();	// labels
    setValue("/start_bof", start_bof);
    setValue("/stop_eof", stop_eof);
    beginGroup("/snapshots");
    setValue("/type", snapshot_type);
    setValue("/quality", snapshot_quality);
    setValue("/prefix", snapshot_prefix);
    setValue("/delimiter", snapshot_delimiter);
    setValue("/first", snapshot_first);
    setValue("/width", snapshot_width);
    setValue("/extension", snapshot_extension);
    setValue("/range", snapshot_range);
    setValue("/samples", snapshot_samples);
    endGroup();	// snapshots
    beginGroup("/pipe");
    for (unsigned int i = 0; i < pipe_command.size(); ++i) {
        QString key = "/" + QString::number(i);
        beginGroup(key);
        setValue("/command", pipe_command[i]);
        setValue("/post", pipe_post[i]);
        setValue("/label", pipe_label[i]);
        setValue("/format", pipe_format[i]);
        endGroup();	// key
    }
    endGroup();	// pipe
    beginGroup("/chapters");
    setValue("/interval", chapter_interval);
    setValue("/tolerance", chapter_tolerance);
    setValue("/threshold", chapter_threshold);
    setValue("/minimum", chapter_minimum);
    endGroup();	// auto chapters
}

// private settings variable
static tvclipper_settings *mysettings = NULL;

void constructSettings(QString orgName, QString productName)
{
    if (!mysettings)
    {
        mysettings = new tvclipper_settings(orgName, productName);
    }

    return;
}

// access function (includes delayed loading)
tvclipper_settings* settings() {
    if (!mysettings->loaded) {
        mysettings->load_settings();
        mysettings->loaded = true;
    }

    return mysettings;
}

void destructSettings()
{
    if (mysettings)
        delete mysettings;
}
