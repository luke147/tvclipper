/*  dvbcut settings
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

#include <string>
#include <vector>

#include <qstringlist.h>
#include <QStandardPaths>

#include <assert.h>

#include <QApplication>
#include "defines.h"
#include "settings.h"


#define TVCLIPPER_QSETTINGS_PATH "/" TVCLIPPER_QSETTINGS_DOMAIN "/"

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

#define OPTION_VERSION                  "/version"

#define OPTIONS_WHEEL                   "/wheel"
#define OPTION_WHEEL_NORMAL             "/incr_normal"
#define OPTION_WHEEL_SHIFT              "/incr_shift"
#define OPTION_WHEEL_CTRL               "/incr_ctrl"
#define OPTION_WHEEL_ALT                "/incr_alt"
#define OPTION_WHEEL_THRESHOLD          "/threshold"
#define OPTION_WHEEL_DELTA              "/delta"

#define OPTIONS_SLIDERS                  "/sliders"
#define OPTION_SLIDERS_JOG_MAXIMUM       "/jog_maximum"
#define OPTION_SLIDERS_JOG_THRESHOLD     "/jog_threshold"
#define OPTION_SLIDERS_JOG_OFFSET        "/jog_offset"
#define OPTION_SLIDERS_JOG_INTERVAL      "/jog_interval"
#define OPTION_SLIDERS_LIN_INTERVAL      "/lin_interval"

#define OPTIONS_LASTDIR                  "/lastdir"
#define OPTION_LASTDIR_NAME              "/name"
#define OPTION_LASTDIR_UPDATE            "/update"

#define OPTIONS_FILTERS                  "/filters"
#define OPTION_FILTERS_IDXFILTER         "/idxfilter"
#define OPTION_FILTERS_PRJFILTER         "/prjfilter"
#define OPTION_FILTERS_LOADFILTER        "/loadfilter"

#define OPTIONS_VIEW_SCALE_FACTOR        "/viewscalefactor"
#define OPTION_VIEW_SCALE_FACTOR_CURRENT "/current"

#define OPTION_EXPORT_FORMAT             "/export_format"

#define OPTIONS_RECENTFILES              "/recentfiles"
#define OPTION_RECENTFILES_MAX           "/max"
#define OPTION_RECENTFILES_FILE_IDX         "idx"

#define OPTIONS_LABELS                      "/labels"
#define OPTION_LABELS_START                 "/start"
#define OPTION_LABELS_STOP                 "/stop"
#define OPTION_LABELS_CHAPTER                 "/chapter"
#define OPTION_LABELS_BOOKMARK                 "/bookmark"
#define OPTION_LABELS_CUSTOM_COLORS                 "/CustomColors"
#define OPTION_LABELS_COLOR_START_NORMAL              "/ColorStartNormal"
#define OPTION_LABELS_COLOR_START_HIGHLIGHT              "/ColorStartHighlight"
#define OPTION_LABELS_COLOR_STOP_NORMAL              "/ColorStopNormal"
#define OPTION_LABELS_COLOR_STOP_HIGHLIGHT              "/ColorStopHighlight"
#define OPTION_LABELS_COLOR_CHAPTER_NORMAL              "/ColorchapterNormal"
#define OPTION_LABELS_COLOR_CHAPTER_HIGHLIGHT              "/ColorChapterHighlight"
#define OPTION_LABELS_COLOR_BOOKMARK_NORMAL              "/ColorBookmarkNormal"
#define OPTION_LABELS_COLOR_BOOKMARK_HIGHLIGHT              "/ColorBookmarkHighlight"
#define OPTION_LABELS_COLOR_TEXT_NORMAL              "/ColorTextNormal"
#define OPTION_LABELS_COLOR_TEXT_HIGHLIGHT              "/ColorTextHighlight"

#define OPTIONS_SNAPSHOTS               "/snapshots"
#define OPTION_SNAPSHOTS_TYPE           "/type"
#define OPTION_SNAPSHOTS_QUALITY           "/quality"
#define OPTION_SNAPSHOTS_PREFIX           "/prefix"
#define OPTION_SNAPSHOTS_DELIMITER           "/delimiter"
#define OPTION_SNAPSHOTS_FIRST           "/first"
#define OPTION_SNAPSHOTS_WIDTH           "/width"
#define OPTION_SNAPSHOTS_EXTENSION           "/extension"
#define OPTION_SNAPSHOTS_RANGE           "/range"
#define OPTION_SNAPSHOTS_SAMPLES           "/samples"

#define OPTIONS_PIPE                        "/pipe"
#define OPTION_PIPE_COMMAND                 "/command"
#define OPTION_PIPE_POST                 "/post"
#define OPTION_PIPE_DVDLABEL                 "/dvdlabel"
#define OPTION_PIPE_FORMAT                 "/format"

#define OPTIONS_CHAPTERS                 "/chapters"
#define OPTION_CHAPTERS_INTERVAL                 "/interval"
#define OPTION_CHAPTERS_TOLERANCE                 "/tolerance"
#define OPTION_CHAPTERS_THRESHOLD                 "/threshold"
#define OPTION_CHAPTERS_MINIMUM                 "/minimum"

#define OPTION_START_BOF                    "/start_bof"
#define OPTION_STOP_EOF                    "/stop_eof"

#define OPTION_WHEEL_NORMAL_DEF_VAL     25 * 60
#define OPTION_WHEEL_ALT_DEF_VAL        15 * 25 * 60
#define OPTION_WHEEL_CTRL_DEF_VAL       1
#define OPTION_WHEEL_SHIFT_DEF_VAL      25
#define OPTION_WHEEL_THRESHOLD_DEF_VAL  24
#define OPTION_WHEEL_DELTA_DEF_VAL      50

#define OPTION_SLIDERS_JOG_MAXIMUM_DEF_VAL 180000
#define OPTION_SLIDERS_JOG_THRESHOLD_DEF_VAL     50
#define OPTION_SLIDERS_JOG_OFFSET_DEF_VAL    0.4
#define OPTION_SLIDERS_JOG_INTERVAL_DEF_VAL      1
#define OPTION_SLIDERS_LIN_INTERVAL_DEF_VAL  3600

#define OPTION_LASTDIR_UPDATE_DEF_VAL            true

#define OPTION_FILTERS_LOADFILTER_DEF_VAL \
    "Recognized files (*.tvclipper *.m2t *.mpg *.rec* *.ts *.tts* *.trp *.vdr);;" \
    "tvclipper project files (*.tvclipper);;" \
    "MPEG files (*.m2t *.mpg *.rec* *.ts *.tts* *.trp *.vdr);;" \
    "All files (*)"
#define OPTION_FILTERS_IDXFILTER_DEF_VAL \
    "tvclipper index files (*.idx);;All files (*)"
#define OPTION_FILTERS_PRJFILTER_DEF_VAL \
    "tvclipper project files (*.tvclipper);;All files (*)"

#define OPTION_VIEW_SCALE_FACTOR_CURRENT_DEF_VAL 0.99

#define OPTION_EXPORT_FORMAT_DEF_VAL 0

#define OPTION_RECENTFILES_MAX_DEF_VAL           5

#define OPTION_LABELS_START_DEF_VAL   "<font size=\"+1\" color=\"" START_COLOUR_MASK "\"><b>START</b></font>"
#define OPTION_LABELS_STOP_DEF_VAL    "<font size=\"+1\" color=\"" STOP_COLOUR_MASK "\"><b>STOP</b></font>"
#define OPTION_LABELS_CHAPTER_DEF_VAL "<font color=\"" CHAPTER_COLOUR_MASK "\">CHAPTER</font>"
#define OPTION_LABELS_BOOKMARK_DEF_VAL "<font color=\"" BOOKMARK_COLOUR_MASK "\">BOOKMARK</font>"

#define OPTION_SNAPSHOTS_TYPE_DEF_VAL           "PNG"
#define OPTION_SNAPSHOTS_QUALITY_DEF_VAL           -1
#define OPTION_SNAPSHOTS_PREFIX_DEF_VAL           ""
#define OPTION_SNAPSHOTS_DELIMITER_DEF_VAL           "_"
#define OPTION_SNAPSHOTS_FIRST_DEF_VAL           1
#define OPTION_SNAPSHOTS_WIDTH_DEF_VAL           3
#define OPTION_SNAPSHOTS_EXTENSION_DEF_VAL           "png"
#define OPTION_SNAPSHOTS_RANGE_DEF_VAL           0
#define OPTION_SNAPSHOTS_SAMPLES_DEF_VAL           1

#define OPTION_PIPE_COMMAND_DEF_VAL "|dvdauthor -t -c '%CHAPTERS%' -v mpeg2 -o '%OUTPUT%' -"
#define OPTION_PIPE_POST_DEF_VAL "dvdauthor -o '%OUTPUT%' -T"
#define OPTION_PIPE_DVDLABEL_DEF_VAL "DVD-Video titleset (dvdauthor)"
#define OPTION_PIPE_FORMAT_DEV_VAL 0

#define OPTION_CHAPTERS_INTERVAL_DEF_VAL 600 * 25
#define OPTION_CHAPTERS_TOLERANCE_DEF_VAL 0
#define OPTION_CHAPTERS_THRESHOLD_DEF_VAL  50.
#define OPTION_CHAPTERS_MINIMUM_DEF_VAL 200 * 25

#define OPTION_START_BOF_DEF_VAL                    true
#define OPTION_STOP_EOF_DEF_VAL                    true

#define OPTION_LABELS_CUSTOM_COLORS_DEF_VAL                 false
#define OPTION_LABELS_COLOR_START_NORMAL_DEF_VAL              "#009000"
#define OPTION_LABELS_COLOR_START_HIGHLIGHT_DEF_VAL              "#00DC00"
#define OPTION_LABELS_COLOR_STOP_NORMAL_DEF_VAL              "#900000"
#define OPTION_LABELS_COLOR_STOP_HIGHLIGHT_DEF_VAL              "#DC0000"
#define OPTION_LABELS_COLOR_CHAPTER_NORMAL_DEF_VAL              "#909000"
#define OPTION_LABELS_COLOR_CHAPTER_HIGHLIGHT_DEF_VAL              "#D0D000"
#define OPTION_LABELS_COLOR_BOOKMARK_NORMAL_DEF_VAL              "#0000D0"
#define OPTION_LABELS_COLOR_BOOKMARK_HIGHLIGHT_DEF_VAL              "#000090"
#define OPTION_LABELS_COLOR_TEXT_NORMAL_DEF_VAL              "#000000"
#define OPTION_LABELS_COLOR_TEXT_HIGHLIGHT_DEF_VAL              "#FFFFFF"


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
    endGroup(); // TVCLIPPER_QSETTINGS_PATH
}

void tvclipper_settings::load_settings()
{
    //int version = value(OPTION_VERSION, 0).toInt();

    beginGroup(OPTIONS_WHEEL);
    wheel_increments[WHEEL_INCR_NORMAL] = value(OPTION_WHEEL_NORMAL, OPTION_WHEEL_NORMAL_DEF_VAL).toInt();
    wheel_increments[WHEEL_INCR_SHIFT] = value(OPTION_WHEEL_SHIFT, OPTION_WHEEL_SHIFT_DEF_VAL).toInt();
    wheel_increments[WHEEL_INCR_CTRL] = value(OPTION_WHEEL_CTRL, OPTION_WHEEL_CTRL_DEF_VAL).toInt();
    wheel_increments[WHEEL_INCR_ALT] = value(OPTION_WHEEL_ALT, OPTION_WHEEL_ALT_DEF_VAL).toInt();
    wheel_threshold = value(OPTION_WHEEL_THRESHOLD, OPTION_WHEEL_THRESHOLD_DEF_VAL).toInt();
    // Note: delta is a multiple of 120 (see Qt documentation)
    wheel_delta = value(OPTION_WHEEL_DELTA, OPTION_WHEEL_DELTA_DEF_VAL).toInt();
    if (wheel_delta == 0)
        wheel_delta = 1;	// avoid devide by zero
    endGroup();	// wheel

    beginGroup(OPTIONS_SLIDERS);
    jog_maximum = value(OPTION_SLIDERS_JOG_MAXIMUM, OPTION_SLIDERS_JOG_MAXIMUM_DEF_VAL).toInt();
    jog_threshold = value(OPTION_SLIDERS_JOG_THRESHOLD, OPTION_SLIDERS_JOG_THRESHOLD_DEF_VAL).toInt();
    // to increase the "zero frames"-region of the jog-slider
    jog_offset = value(OPTION_SLIDERS_JOG_OFFSET, OPTION_SLIDERS_JOG_OFFSET_DEF_VAL).toDouble();
    // sub-intervals of jog_maximum
    jog_interval = value(OPTION_SLIDERS_JOG_INTERVAL, OPTION_SLIDERS_JOG_INTERVAL_DEF_VAL).toInt();
    if (jog_interval < 0)
        jog_interval = 0;
    lin_interval = value(OPTION_SLIDERS_LIN_INTERVAL, OPTION_SLIDERS_LIN_INTERVAL_DEF_VAL).toInt();
    if (lin_interval < 0)
        lin_interval = 0;
    endGroup();	// sliders

    beginGroup(OPTIONS_LASTDIR);
    lastdir = value(OPTION_LASTDIR_NAME, QStandardPaths::displayName(QStandardPaths::HomeLocation)).toString();
    lastdir_update = value(OPTION_LASTDIR_UPDATE, OPTION_LASTDIR_UPDATE_DEF_VAL).toBool();
    endGroup(); // lastdir

    beginGroup(OPTIONS_FILTERS);
    idxfilter = value(OPTION_FILTERS_IDXFILTER, OPTION_FILTERS_IDXFILTER_DEF_VAL).toString();
    prjfilter = value(OPTION_FILTERS_PRJFILTER, OPTION_FILTERS_PRJFILTER_DEF_VAL).toString();
    loadfilter = value(OPTION_FILTERS_LOADFILTER, OPTION_FILTERS_LOADFILTER_DEF_VAL).toString();
    endGroup();	// filters

    /* view scale factor */
    beginGroup(OPTIONS_VIEW_SCALE_FACTOR);
    viewscalefactor = value(OPTION_VIEW_SCALE_FACTOR_CURRENT, OPTION_VIEW_SCALE_FACTOR_CURRENT_DEF_VAL).toDouble();
    endGroup(); // viewscalefactor

    export_format = value(OPTION_EXPORT_FORMAT, 0).toInt();

    beginGroup(QStringLiteral(OPTIONS_RECENTFILES));
    recentfiles_max = value(QStringLiteral(OPTION_RECENTFILES_MAX), OPTION_RECENTFILES_MAX_DEF_VAL).toInt();

    recentfiles.clear();
    std::list<std::string> videoFilenames;

    {
        QStringList recentFileNumbers = childGroups();
        unsigned int currentNumber = 0;
        for (QStringList::const_iterator itNumber = recentFileNumbers.begin();
             itNumber != recentFileNumbers.end() && currentNumber < recentfiles_max;
             itNumber++, currentNumber++)
        {
            // format with subkeys and multiple files!
            beginGroup(*itNumber);

            if (!contains(QStringLiteral(OPTION_RECENTFILES_FILE_IDX))) {
                endGroup();
                continue;
            }
            videoFilenames.clear();
            QStringList fileKeys = childKeys();
            QString idxfilename;
            for (QStringList::const_iterator iter = fileKeys.begin(); iter != fileKeys.end(); iter++) {
                if (*iter == QStringLiteral(OPTION_RECENTFILES_FILE_IDX)) {
                    idxfilename = value(*iter).toString();
                    continue;
                }
                QString videoFilename = value(*iter).toString();
                if (videoFilename.isEmpty()) {
                    endGroup();	// *itNumber
                    continue;
                }
                videoFilenames.push_back( videoFilename.toStdString() );
            }

            if (videoFilenames.empty()) {
                endGroup();	// *itNumber
                continue;
            }

            // adding one item of recent file list
            recentfiles.push_back(std::pair<std::list<std::string>,std::string>(videoFilenames, idxfilename.toStdString()));

            endGroup();	// *itNumber
        }
    }

    endGroup();	// recentfiles


    beginGroup(OPTIONS_LABELS);
    start_label = value(OPTION_LABELS_START, OPTION_LABELS_START_DEF_VAL).toString();
    stop_label = value(OPTION_LABELS_STOP, OPTION_LABELS_STOP_DEF_VAL).toString();
    chapter_label = value(OPTION_LABELS_CHAPTER, OPTION_LABELS_CHAPTER_DEF_VAL).toString();
    bookmark_label = value(OPTION_LABELS_BOOKMARK, OPTION_LABELS_BOOKMARK_DEF_VAL).toString();
    customColors = value(OPTION_LABELS_CUSTOM_COLORS, OPTION_LABELS_CUSTOM_COLORS_DEF_VAL).toBool();
    colorStartNormal = value(OPTION_LABELS_COLOR_START_NORMAL, OPTION_LABELS_COLOR_START_NORMAL_DEF_VAL).toString();
    colorStartHighlight = value(OPTION_LABELS_COLOR_START_HIGHLIGHT, OPTION_LABELS_COLOR_START_HIGHLIGHT_DEF_VAL).toString();
    colorStopNormal = value(OPTION_LABELS_COLOR_STOP_NORMAL, OPTION_LABELS_COLOR_STOP_NORMAL_DEF_VAL).toString();
    colorStopHighlight = value(OPTION_LABELS_COLOR_STOP_HIGHLIGHT, OPTION_LABELS_COLOR_STOP_HIGHLIGHT_DEF_VAL).toString();
    colorChapterNormal = value(OPTION_LABELS_COLOR_CHAPTER_NORMAL, OPTION_LABELS_COLOR_CHAPTER_NORMAL_DEF_VAL).toString();
    colorChapterHighlight = value(OPTION_LABELS_COLOR_CHAPTER_HIGHLIGHT, OPTION_LABELS_COLOR_CHAPTER_HIGHLIGHT_DEF_VAL).toString();
    colorBookmarkNormal = value(OPTION_LABELS_COLOR_BOOKMARK_NORMAL, OPTION_LABELS_COLOR_BOOKMARK_NORMAL_DEF_VAL).toString();
    colorBookmarkHighlight = value(OPTION_LABELS_COLOR_BOOKMARK_HIGHLIGHT, OPTION_LABELS_COLOR_BOOKMARK_HIGHLIGHT_DEF_VAL).toString();
    colorTextNormal = value(OPTION_LABELS_COLOR_TEXT_NORMAL, OPTION_LABELS_COLOR_TEXT_NORMAL_DEF_VAL).toString();
    colorTextHighlight = value(OPTION_LABELS_COLOR_TEXT_HIGHLIGHT, OPTION_LABELS_COLOR_TEXT_HIGHLIGHT_DEF_VAL).toString();
    endGroup();	// labels

    start_bof = value(QStringLiteral(OPTION_START_BOF), true).toBool();
    stop_eof = value(QStringLiteral(OPTION_STOP_EOF), true).toBool();

    beginGroup(OPTIONS_SNAPSHOTS);
    snapshot_type = value(OPTION_SNAPSHOTS_TYPE, OPTION_SNAPSHOTS_TYPE_DEF_VAL).toString();
    snapshot_quality = value(OPTION_SNAPSHOTS_QUALITY, OPTION_SNAPSHOTS_QUALITY_DEF_VAL).toInt();
    snapshot_prefix = value(OPTION_SNAPSHOTS_PREFIX, OPTION_SNAPSHOTS_PREFIX_DEF_VAL).toString();
    snapshot_delimiter = value(OPTION_SNAPSHOTS_DELIMITER, OPTION_SNAPSHOTS_DELIMITER_DEF_VAL).toString();
    snapshot_first = value(OPTION_SNAPSHOTS_FIRST, OPTION_SNAPSHOTS_FIRST_DEF_VAL).toInt();
    snapshot_width = value(OPTION_SNAPSHOTS_WIDTH, OPTION_SNAPSHOTS_WIDTH_DEF_VAL).toInt();
    snapshot_extension = value(OPTION_SNAPSHOTS_EXTENSION, OPTION_SNAPSHOTS_EXTENSION_DEF_VAL).toString();
    snapshot_range = value(OPTION_SNAPSHOTS_RANGE, OPTION_SNAPSHOTS_RANGE_DEF_VAL).toInt();
    snapshot_samples = value(OPTION_SNAPSHOTS_SAMPLES, OPTION_SNAPSHOTS_SAMPLES_DEF_VAL).toInt();
    endGroup();	// snapshots

    beginGroup(OPTIONS_PIPE);
    pipe_command.clear();
    pipe_post.clear();
    pipe_label.clear();
    pipe_format.clear();

    QStringList pipeList = childGroups();
    for (QStringList::const_iterator it = pipeList.begin(); it != pipeList.end(); it++) {
        QString command = value(*it + OPTION_PIPE_COMMAND, OPTION_PIPE_COMMAND_DEF_VAL).toString();
        QString label = value(*it + OPTION_PIPE_DVDLABEL, OPTION_PIPE_DVDLABEL_DEF_VAL).toString();
        QString post = value(*it + OPTION_PIPE_POST, OPTION_PIPE_POST_DEF_VAL).toString();
        int format = value(*it + OPTION_PIPE_FORMAT, OPTION_PIPE_FORMAT_DEV_VAL).toInt();
        if (format < 0 || format > 3) {
            format = 0;
        }
        if (command.isEmpty() || label.isEmpty()) {
            continue;
        }

        pipe_command.push_back(command);
        pipe_post.push_back(post);
        pipe_label.push_back(label);
        pipe_format.push_back(format);
    }
    endGroup();	// pipe

    beginGroup(QStringLiteral(OPTIONS_CHAPTERS));
    // length (>0) or number (<0) of chapter(s)
    chapter_interval = value(QStringLiteral(OPTION_CHAPTERS_INTERVAL), OPTION_CHAPTERS_INTERVAL_DEF_VAL).toInt();
    // detection of scene changes is rather time comsuming...
    chapter_tolerance = value(QStringLiteral(OPTION_CHAPTERS_TOLERANCE), OPTION_CHAPTERS_TOLERANCE_DEF_VAL).toInt();
    // average color distance needed for a scene change
    chapter_threshold = value(QStringLiteral(OPTION_CHAPTERS_THRESHOLD), OPTION_CHAPTERS_THRESHOLD_DEF_VAL).toDouble();
    // minimal length of a chapter
    chapter_minimum = value(QStringLiteral(OPTION_CHAPTERS_MINIMUM), OPTION_CHAPTERS_MINIMUM_DEF_VAL).toInt();
    endGroup();	// auto chapters
}

void
tvclipper_settings::save_settings()
{
    setValue(OPTION_VERSION, 0);	// latest config version

    beginGroup(QStringLiteral(OPTIONS_WHEEL));
    setValue(QStringLiteral(OPTION_WHEEL_NORMAL), wheel_increments[WHEEL_INCR_NORMAL]);
    setValue(QStringLiteral(OPTION_WHEEL_SHIFT), wheel_increments[WHEEL_INCR_SHIFT]);
    setValue(QStringLiteral(OPTION_WHEEL_CTRL), wheel_increments[WHEEL_INCR_CTRL]);
    setValue(QStringLiteral(OPTION_WHEEL_ALT), wheel_increments[WHEEL_INCR_ALT]);
    setValue(QStringLiteral(OPTION_WHEEL_THRESHOLD), wheel_threshold);
    setValue(QStringLiteral(OPTION_WHEEL_DELTA), wheel_delta);
    endGroup();	// wheel

    beginGroup(QStringLiteral(OPTIONS_SLIDERS));
    setValue(QStringLiteral(OPTION_SLIDERS_JOG_MAXIMUM), jog_maximum);
    setValue(QStringLiteral(OPTION_SLIDERS_JOG_THRESHOLD), jog_threshold);
    setValue(QStringLiteral(OPTION_SLIDERS_JOG_OFFSET), jog_offset);
    setValue(QStringLiteral(OPTION_SLIDERS_JOG_INTERVAL), jog_interval);
    setValue(QStringLiteral(OPTION_SLIDERS_LIN_INTERVAL), lin_interval);
    endGroup();	// sliders

    beginGroup(QStringLiteral(OPTIONS_LASTDIR));
    setValue(QStringLiteral(OPTION_LASTDIR_NAME), lastdir);
    setValue(QStringLiteral(OPTION_LASTDIR_UPDATE), lastdir_update);
    endGroup();	// lastdir

    beginGroup(QStringLiteral(OPTIONS_FILTERS));
    setValue(QStringLiteral(OPTION_FILTERS_IDXFILTER), idxfilter);
    setValue(QStringLiteral(OPTION_FILTERS_PRJFILTER), prjfilter);
    setValue(QStringLiteral(OPTION_FILTERS_LOADFILTER), loadfilter);
    endGroup();	// filters

    beginGroup(QStringLiteral(OPTIONS_VIEW_SCALE_FACTOR));
    setValue(QStringLiteral(OPTION_VIEW_SCALE_FACTOR_CURRENT), viewscalefactor);
    endGroup();	// viewscalefactor

    setValue(QStringLiteral(OPTION_EXPORT_FORMAT), export_format);

    beginGroup(QStringLiteral(OPTIONS_RECENTFILES));
    /* --- removing old recentfile list --- */
    QStringList keys = allKeys();
    for (QStringList::const_iterator it = keys.begin(); it != keys.end(); it++) {
        remove(*it);
    }

    /* --- saving --- */
    // saving maximal number of recent files
    setValue(QStringLiteral(OPTION_RECENTFILES_MAX), int(recentfiles_max));

    // saving the updated list of recent files
    for (unsigned int i = 0; i < recentfiles.size(); ++i) {
        QString key = "/" + QString::number(i);
        beginGroup(key);
        int j = 0;
        for(std::list<std::string>::iterator it = settings()->recentfiles[i].first.begin(); it!=settings()->recentfiles[i].first.end(); it++, j++) {
            setValue("/" + QString::number(j), QString::fromStdString(*it));
        }
        setValue(QStringLiteral(OPTION_RECENTFILES_FILE_IDX), QString::fromStdString(recentfiles[i].second));
        endGroup();	// key
    }
    endGroup();	// recentfiles

    beginGroup(QStringLiteral(OPTIONS_LABELS));
    setValue(QStringLiteral(OPTION_LABELS_START), start_label);
    setValue(QStringLiteral(OPTION_LABELS_STOP), stop_label);
    setValue(QStringLiteral(OPTION_LABELS_CHAPTER), chapter_label);
    setValue(QStringLiteral(OPTION_LABELS_BOOKMARK), bookmark_label);
    setValue(QStringLiteral(OPTION_LABELS_CUSTOM_COLORS), customColors);
    setValue(OPTION_LABELS_COLOR_START_NORMAL, colorStartNormal);
    setValue(OPTION_LABELS_COLOR_START_HIGHLIGHT, colorStartHighlight);
    setValue(OPTION_LABELS_COLOR_STOP_NORMAL, colorStopNormal);
    setValue(OPTION_LABELS_COLOR_STOP_HIGHLIGHT, colorStopHighlight);
    setValue(OPTION_LABELS_COLOR_CHAPTER_NORMAL, colorChapterNormal);
    setValue(OPTION_LABELS_COLOR_CHAPTER_HIGHLIGHT, colorChapterHighlight);
    setValue(OPTION_LABELS_COLOR_BOOKMARK_NORMAL, colorBookmarkNormal);
    setValue(OPTION_LABELS_COLOR_BOOKMARK_HIGHLIGHT, colorBookmarkHighlight);
    setValue(OPTION_LABELS_COLOR_TEXT_NORMAL, colorTextNormal);
    setValue(OPTION_LABELS_COLOR_TEXT_HIGHLIGHT, colorTextHighlight);
    endGroup();	// labels

    setValue(QStringLiteral(OPTION_START_BOF), start_bof);
    setValue(QStringLiteral(OPTION_STOP_EOF), stop_eof);

    beginGroup(QStringLiteral(OPTIONS_SNAPSHOTS));
    setValue(QStringLiteral(OPTION_SNAPSHOTS_TYPE), snapshot_type);
    setValue(QStringLiteral(OPTION_SNAPSHOTS_QUALITY), snapshot_quality);
    setValue(QStringLiteral(OPTION_SNAPSHOTS_PREFIX), snapshot_prefix);
    setValue(QStringLiteral(OPTION_SNAPSHOTS_DELIMITER), snapshot_delimiter);
    setValue(QStringLiteral(OPTION_SNAPSHOTS_FIRST), snapshot_first);
    setValue(QStringLiteral(OPTION_SNAPSHOTS_WIDTH), snapshot_width);
    setValue(QStringLiteral(OPTION_SNAPSHOTS_EXTENSION), snapshot_extension);
    setValue(QStringLiteral(OPTION_SNAPSHOTS_RANGE), snapshot_range);
    setValue(QStringLiteral(OPTION_SNAPSHOTS_SAMPLES), snapshot_samples);
    endGroup();	// snapshots

    beginGroup(QStringLiteral(OPTIONS_PIPE));
    for (unsigned int i = 0; i < pipe_command.size(); ++i) {
        QString key = "/" + QString::number(i);
        beginGroup(key);
        setValue(QStringLiteral(OPTION_PIPE_COMMAND), pipe_command[i]);
        setValue(QStringLiteral(OPTION_PIPE_POST), pipe_post[i]);
        setValue(QStringLiteral(OPTION_PIPE_DVDLABEL), pipe_label[i]);
        setValue(QStringLiteral(OPTION_PIPE_FORMAT), pipe_format[i]);
        endGroup();	// key
    }
    endGroup();	// pipe

    beginGroup(QStringLiteral(OPTIONS_CHAPTERS));
    setValue(QStringLiteral(OPTION_CHAPTERS_INTERVAL), chapter_interval);
    setValue(QStringLiteral(OPTION_CHAPTERS_TOLERANCE), chapter_tolerance);
    setValue(QStringLiteral(OPTION_CHAPTERS_THRESHOLD), chapter_threshold);
    setValue(QStringLiteral(OPTION_CHAPTERS_MINIMUM), chapter_minimum);
    endGroup();	// chapters
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
