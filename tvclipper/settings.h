/*  dvbcut settings
    Copyright (c) 2006 Michael Riepe

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

#ifndef _TVCLIPPER_SETTINGS_H
#define _TVCLIPPER_SETTINGS_H

#include <QString>
#include <string>
#include <vector>

#include <QSettings>
#include <QPalette>


#ifndef SHAREDIR
#define SHAREDIR "."
#endif

#ifndef DOCDIR
#define DOCDIR "doc"
#endif

#ifndef ICON_TYPE
#define ICON_TYPE "png"
#endif

#define TVCLIPPER_QSETTINGS_DOMAIN "tvclipper"
#define TVCLIPPER_QSETTINGS_PRODUCT "tvclipper"
#define TVCLIPPER_QSETTINGS_ORGANIZATION "tvclipper"

#define EVENT_ITEM_TEST_LABEL "TEST TEST"
#define EVENT_ITEM_RIGHT_MARGIN 25
#define EVENT_ITEM_ICON_MARGIN 3
#define EVENT_ITEM_TEXT_MARGIN 3
#define EVENT_ITEM_ICON_WIDTH 130
#define EVENT_ITEM_ICON_HEIGHT 90

#define EVENT_ITEM_SPACE_BETWEEN_ELEMENTS    EVENT_ITEM_TEXT_MARGIN + (2 * EVENT_ITEM_ICON_MARGIN) + EVENT_ITEM_RIGHT_MARGIN

#define DEFAULT_NORMAL_START_COLOUR       Qt::darkGreen // "#009000"
#define DEFAULT_HIGHLIGHT_START_COLOUR    Qt::green // "#00DC00"
#define DEFAULT_NORMAL_STOP_COLOUR        Qt::darkRed // "#900000"
#define DEFAULT_HIGHLIGHT_STOP_COLOUR     Qt::red // "#DC0000"
#define DEFAULT_NORMAL_CHAPTER_COLOUR     Qt::darkYellow // "#909000"
#define DEFAULT_HIGHLIGHT_CHAPTER_COLOUR  Qt::yellow // "#D0D000"
#define DEFAULT_NORMAL_BOOKMARK_COLOUR    Qt::darkBlue // "#0000D0"
#define DEFAULT_HIGHLIGHT_BOOKMARK_COLOUR Qt::darkBlue // "#000090"
#define DEFAULT_NORMAL_TEXT_COLOUR        QPalette::Text // "#000000"
#define DEFAULT_HIGHLIGHT_TEXT_COLOUR     QPalette::HighlightedText // "#FFFFFF"

#define START_COLOUR_MASK "%start-colour%"
#define STOP_COLOUR_MASK "%stop-colour%"
#define CHAPTER_COLOUR_MASK "%chapter-colour%"
#define BOOKMARK_COLOUR_MASK "%bookmark-colour%"
#define NORMAL_TEXT_COLOUR_MASK "%normal-text-colour%"

enum WHEEL_INCR {
    WHEEL_INCR_NORMAL,
    WHEEL_INCR_SHIFT,
    WHEEL_INCR_CTRL,
    WHEEL_INCR_ALT,
};

class tvclipper_settings : QSettings {
public:
    tvclipper_settings(QString organization, QString application);
    ~tvclipper_settings();

    void load_settings();
    void save_settings();

    bool loaded;

    QString lastdir;
    bool lastdir_update;
    QString idxfilter;
    QString prjfilter;
    QString loadfilter;
    std::vector< std::pair<std::list<std::string>, std::string> > recentfiles;
    unsigned int recentfiles_max;
    double viewscalefactor;
    int wheel_increments[sizeof(WHEEL_INCR)];
    int wheel_threshold;
    int wheel_delta;
    int jog_maximum;
    int jog_threshold;
    double jog_offset;
    int jog_interval;
    int lin_interval;
    int export_format;
    QString start_label;
    QString stop_label;
    QString chapter_label;
    QString bookmark_label;
    bool customColors;
    QString colorStartNormal;
    QString colorStartHighlight;
    QString colorStopNormal;
    QString colorStopHighlight;
    QString colorChapterNormal;
    QString colorChapterHighlight;
    QString colorBookmarkNormal;
    QString colorBookmarkHighlight;
    QString colorTextNormal;
    QString colorTextHighlight;
    bool start_bof;
    bool stop_eof;
    QString snapshot_type;
    QString snapshot_prefix;
    QString snapshot_delimiter;
    QString snapshot_extension;
    int snapshot_quality;
    int snapshot_first;
    int snapshot_width;
    int snapshot_range;
    int snapshot_samples;
    std::vector<QString> pipe_command;
    std::vector<QString> pipe_post;
    std::vector<QString> pipe_label;
    std::vector<int> pipe_format;
    int chapter_interval;
    int chapter_tolerance;
    double chapter_threshold;
    int chapter_minimum;
    bool maximizeWindow;
};

// access function
extern void constructSettings(QString orgName = QString(), QString productName = QString());
extern tvclipper_settings* settings();
extern void destructSettings();

#endif // _TVCLIPPER_SETTINGS_H
