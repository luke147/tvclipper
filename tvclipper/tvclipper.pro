#  tvclipper
#  Copyright (c) 2015 Lukáš Vlček

#  This file is part of TV Clipper.

#  TV Clipper is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.

#  TV Clipper is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with TV Clipper. If not, see <http://www.gnu.org/licenses/>.


include(tvclipper.pri)
include(variables.pri)

# ***************************************************************************
# *** main settings

CONFIG += c++11
unix: CONFIG += link_pkgconfig
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = tvclipper
TEMPLATE = app
TARGET_x = 0
TARGET_x.y.z = 0.7alpha

SOURCES += main.cpp\
        tvclipper.cpp \
    avframe.cpp \
    buffer.cpp \
    differenceimageprovider.cpp \
    eventlistitem.cpp \
    exception.cpp \
    imageprovider.cpp \
    lavfmuxer.cpp \
    logoutput.cpp \
    mpegmuxer.cpp \
    mpgfile.cpp \
    mpgIndex.cpp \
    playaudio.cpp \
    progressstatusbar.cpp \
    psfile.cpp \
    pts.cpp \
    settings.cpp \
    streamdata.cpp \
    tsfile.cpp \
    xmlprjfile.cpp \
    listitemdelegate.cpp \
    DlgMplayerError.cpp \
    DlgProgress.cpp \
    DlgExport.cpp \
    stream.cpp

HEADERS  += tvclipper.h \
    avframe.h \
    buffer.h \
    busyindicator.h \
    defines.h \
    differenceimageprovider.h \
    eventlistitem.h \
    exception.h \
    imageprovider.h \
    lavfmuxer.h \
    logoutput.h \
    mpegmuxer.h \
    mpgfile.h \
    mpgIndex.h \
    muxer.h \
    playaudio.h \
    port.h \
    progressstatusbar.h \
    psfile.h \
    pts.h \
    settings.h \
    stream.h \
    streamdata.h \
    streamhandle.h \
    tsfile.h \
    types.h \
    version.h \
    xmlprjfile.h \
    listitemdelegate.h \
    DlgMplayerError.h \
    DlgExport.h \
    DlgProgress.h \
    windefines.h

FORMS    += tvclipper.ui \
    DlgExport.ui \
    DlgMplayerError.ui \
    DlgProgress.ui

DISTFILES += \
    share/doc/tvclipper_en.html \
    share/man/tvclipper.1 \
    share/servicemenu/tvclipper_servicemenu.desktop \
    share/doc/COPYING \
    share/doc/CREDITS \
    desktopMenuFile/tvclipper.desktop \
    share/doc/INSTALL \
    share/doc/README \
    share/icons/bookmark.svg \
    share/icons/chapter.svg \
    share/icons/start.svg \
    share/icons/stop.svg \
    share/icons/tvclipper.svg \
    share/icons/video-export.svg \
    share/icons/bookmark.png \
    share/icons/chapter.png \
    share/icons/start.png \
    share/icons/stop.png \
    share/icons/video-export.png

# OTHER_FILES = DISTFILES

# *******************************************************************************************************
# *** variables for installation

# Don't edit value of variables with "def" prefix in the name! Edit value of variables in variables.pri.
defIconType = "png"
defBuildPath = "build"
ICON_TYPE = $$getFirstValOrDefVal("$$lower($$ICON_TYPE)", "$$defIconType")
BUILD_PATH = $$getFirstValOrDefVal("$$BUILD_PATH", "$$defBuildPath")
BUILD_PATH = $$shell_path("$$clean_path("$${BUILD_PATH}")/")

unix: {
    defBinDir = "$$TARGET"
    defShareDir = "$$TARGET"
    defSysShareDir = "share"
    defPrefix = "/opt"
    defSysSharePrefix = "/usr"
    defDocDir = "doc/tvclipper"
    defIconDir = "icons"
    defManDir = "man/man1"
    defApplicationsDir = "applications"
    defServicemenuDir = "kde4/services/ServiceMenus"
    defPathToAppScalableIcons = "hicolor/scalable/apps"

    defSystemIncludeDir = "/usr/include"
    defSystemLibDir = "/usr/$$getLibOrLib64Str()"

    SYSTEM_INCLUDEDIR = "$$getFirstValOrDefVal("$$SYSTEM_INCLUDEDIR", "$$defSystemIncludeDir")"
    SYSTEM_LIBDIR = "$$getFirstValOrDefVal("$$SYSTEM_LIBDIR", "$$defSystemLibDir")"

    BINDIR = $$shell_path($$getFirstValOrDefVal("$$BINDIR", "$$defBinDir"))
    DOCDIR = $$shell_path($$getFirstValOrDefVal("$$DOCDIR", "$$defDocDir"))
    ICONDIR = $$shell_path($$getFirstValOrDefVal("$$ICONS", "$$defIconDir"))
    MANDIR = $$shell_path($$getFirstValOrDefVal("$$MANDIR", "$$defManDir"))
    APPLICATIONSDIR = $$shell_path($$getFirstValOrDefVal("$$APPLICATIONSDIR", "$$defApplicationsDir"))
    SERVICEMENUDIR = $$shell_path($$getFirstValOrDefVal("$$SERVICEMENUDIR", "$$defServicemenuDir"))
    PATH_TO_APP_SCALABLE_ICONS = $$shell_path($$getFirstValOrDefVal("$$PATH_TO_APP_SCALABLE_ICONS", "$$defPathToAppScalableIcons"))
    SHAREDIR = $$shell_path($$getFirstValOrDefVal("$$SHAREDIR", "$$defShareDir"))
    SYSSHARE_PREFIX = $$shell_path($$getFirstValOrDefVal("$$SYSSHARE_PREFIX", "$$defSysSharePrefix"))
    SYSSHAREDIR = $$shell_path($$getFirstValOrDefVal("$$SYSSHAREDIR", "$$defSysShareDir"))
    PREFIX = $$shell_path($$getFirstValOrDefVal("$$PREFIX", "$$defPrefix"))

    BUILD_PREFIX = "$$PREFIX"
    BUILD_SYSSHARE_PREFIX = "$$SYSSHARE_PREFIX"

    INST_PREFIX = $$shell_path($$clean_path("$$PREFIX"))
    INST_SYSSHARE_PREFIX = $$shell_path($$clean_path("$$SYSSHARE_PREFIX"))

    isRelativePath("$$SHAREDIR") {
        BUILD_SHAREDIR = $$shell_path($$clean_path("$${BUILD_PATH}$${BUILD_PREFIX}/$${SHAREDIR}"))
        INST_SHAREDIR = $$shell_path($$clean_path("$$INST_PREFIX/$$SHAREDIR"))
    } else {
        BUILD_SHAREDIR = "$$shell_path($$clean_path("$${BUILD_PATH}$${SHAREDIR}"))"
        INST_SHAREDIR = $$shell_path($$clean_path("$$SHAREDIR"))
    }
    isRelativePath("$$SYSSHAREDIR") {
        BUILD_SYSSHAREDIR = $$shell_path($$clean_path("$${BUILD_PATH}$${BUILD_SYSSHARE_PREFIX}/$$SYSSHAREDIR"))
        INST_SYSSHAREDIR = $$shell_path($$clean_path("$$INST_SYSSHARE_PREFIX/$$SYSSHAREDIR"))
    } else {
        BUILD_SYSSHAREDIR = $$shell_path($$clean_path("$${BUILD_PATH}$$SYSSHAREDIR"))
        INST_SYSSHAREDIR = $$shell_path($$clean_path("$$SYSSHAREDIR"))
    }
    isRelativePath("$$APPLICATIONSDIR") {
        BUILD_APPLICATIONSDIR = $$shell_path($$clean_path("$$BUILD_SYSSHAREDIR/$$APPLICATIONSDIR"))
        INST_APPLICATIONSDIR = $$shell_path($$clean_path("$$INST_SYSSHAREDIR/$$APPLICATIONSDIR"))
    } else {
        BUILD_APPLICATIONSDIR = $$shell_path($$clean_path("$${BUILD_PATH}$$APPLICATIONSDIR"))
        INST_APPLICATIONSDIR = $$shell_path($$clean_path("$$APPLICATIONSDIR"))
    }
    isRelativePath("$$DOCDIR") {
        BUILD_DOCDIR = $$shell_path($$clean_path("$$BUILD_SYSSHAREDIR/$$DOCDIR"))
        INST_DOCDIR = $$shell_path($$clean_path("$$INST_SYSSHAREDIR/$$DOCDIR"))
    } else {
        BUILD_DOCDIR = $$shell_path($$clean_path("$${BUILD_PATH}$$DOCDIR"))
        INST_DOCDIR = $$shell_path($$clean_path("$$DOCDIR"))
    }
    isRelativePath("$$MANDIR") {
        BUILD_MANDIR = $$shell_path($$clean_path("$$BUILD_SYSSHAREDIR/$$MANDIR"))
        INST_MANDIR = $$shell_path($$clean_path("$$INST_SYSSHAREDIR/$$MANDIR"))
    } else {
        BUILD_MANDIR = $$shell_path($$clean_path("$${BUILD_PATH}$$MANDIR"))
        INST_MANDIR = $$shell_path($$clean_path("$$MANDIR"))
    }
    isRelativePath("$$SERVICEMENUDIR") {
        BUILD_SERVICEMENUDIR = $$shell_path($$clean_path("$$BUILD_SYSSHAREDIR/$$SERVICEMENUDIR"))
        INST_SERVICEMENUDIR = $$shell_path($$clean_path("$$INST_SYSSHAREDIR/$$SERVICEMENUDIR"))
    } else {
        BUILD_SERVICEMENUDIR = $$shell_path($$clean_path("$${BUILD_PATH}$$SERVICEMENUDIR"))
        INST_SERVICEMENUDIR = $$shell_path($$clean_path("$$SERVICEMENUDIR"))
    }
    isRelativePath("$$ICONDIR") {
        BUILD_ICONDIR = $$shell_path($$clean_path("$$BUILD_SYSSHAREDIR/$$ICONDIR"))
        INST_ICONDIR = $$shell_path($$clean_path("$$INST_SYSSHAREDIR/$$ICONDIR"))
    } else {
        BUILD_ICONDIR = $$shell_path($$clean_path("$${BUILD_PATH}$$ICONDIR"))
        INST_ICONDIR = $$shell_path($$clean_path("$$ICONDIR"))
    }
    isRelativePath("$$PATH_TO_APP_SCALABLE_ICONS") {
        BUILD_PATH_TO_APP_SCALABLE_ICONS = $$shell_path($$clean_path("$$BUILD_SYSSHAREDIR/$$ICONDIR/$$PATH_TO_APP_SCALABLE_ICONS"))
        INST_PATH_TO_APP_SCALABLE_ICONS = $$shell_path($$clean_path("$$INST_SYSSHAREDIR/$$ICONDIR/$$PATH_TO_APP_SCALABLE_ICONS"))
    } else {
        BUILD_PATH_TO_APP_SCALABLE_ICONS = $$shell_path($$clean_path("$${BUILD_PATH}$$PATH_TO_APP_SCALABLE_ICONS"))
        INST_PATH_TO_APP_SCALABLE_ICONS = $$shell_path($$clean_path("$$PATH_TO_APP_SCALABLE_ICONS"))
    }

    isRelativePath("$$BINDIR") {
        DESTDIR = $$shell_path($$clean_path("$${BUILD_PATH}$${BUILD_PREFIX}/$$BINDIR"))
        INST_BINDIR = $$shell_path($$clean_path("$$INST_PREFIX/$$BINDIR"))
    } else {
        DESTDIR = $$shell_path($$clean_path("$${BUILD_PATH}$$BINDIR"))
        INST_BINDIR = $$shell_path($$clean_path("$$BINDIR"))
    }
}

win32: {
    defBinDir = "$$TARGET"
    defShareDir = "$$TARGET"
    defPrefix = "C:/Program Files"
    defDocDir = "doc"

    BINDIR = $$shell_path($$getFirstValOrDefVal("$$BINDIR", "$$defBinDir"))
    DESTDIR = $$shell_path($$clean_path("$${BUILD_PATH}$$BINDIR"))
    DOCDIR = $$shell_path($$getFirstValOrDefVal("$$DOCDIR", "$$defDocDir"))
    ICONDIR = $$shell_path($$getFirstValOrDefVal("$$ICONS", "$$defIconDir"))
    MANDIR = $$shell_path($$getFirstValOrDefVal("$$MANDIR", "$$defManDir"))
    APPLICATIONSDIR = $$shell_path($$getFirstValOrDefVal("$$APPLICATIONSDIR", "$$defApplicationsDir"))
    SERVICEMENUDIR = $$shell_path($$getFirstValOrDefVal("$$SERVICEMENUDIR", "$$defServicemenuDir"))
    PATH_TO_APP_SCALABLE_ICONS = $$shell_path($$getFirstValOrDefVal("$$PATH_TO_APP_SCALABLE_ICONS", "$$defPathToAppScalableIcons"))
    SHAREDIR = $$shell_path($$getFirstValOrDefVal("$$SHAREDIR", "$$defShareDir"))
    PREFIX = $$shell_path($$getFirstValOrDefVal("$$PREFIX", "$$defPrefix"))

    INST_PREFIX = $$shell_path($$clean_path("$$PREFIX"))
    INST_BINDIR = $$shell_path($$clean_path("$$INST_PREFIX/$$BINDIR"))
    INST_SHAREDIR = $$shell_path($$clean_path("$$INST_PREFIX/$$SHAREDIR"))
    INST_DOCDIR = $$shell_path($$clean_path("$$INST_PREFIX/$$SHAREDIR/$$DOCDIR"))
}

message("application directory: $$INST_BINDIR")
message("share directory: $$INST_SHAREDIR")
unix: {
    message("system share directory: $$INST_SYSSHAREDIR")
}

# *******************************************************************************************************
# *** dependentions
# FFmpeg libraries are required if OS is Windows

FFmpegLibs = avcodec avformat avdevice avutil swscale
setFFmpegLibs($$FFmpegLibs)
unix: {
    libs =  mad ao a52
    setLibs($$libs)
}
message("library dependentions: $$LIBS")
message("package dependentions (pkg-config): $$PKGCONFIG")


# **********************************************************************************************
# *** additional settings

relativeDocPath = $$relative_path("$$INST_DOCDIR", "$$INST_BINDIR")
relativeDocPath = $$clean_path($$relativeDocPath)
relativeSharePath = $$relative_path("$$INST_PREFIX/$$SHAREDIR", "$$INST_PREFIX/$$BINDIR")
relativeSharePath = $$clean_path($$relativeSharePath)
isEmpty(relativeSharePath) {
    relativeSharePath = "."
}

unix: {
# These macros are defined with qmake in Makefile before compilation.
# These definitions aren't included during source code editing.
# expected shell is BASH
# "'DOCDIR=\"value\"'" is expand like 'DOCDIR="value"' parameter
# in BASH: parameter 'DOCDIR="value"' is expand like DOCDIR="value"
    PATH_DEFINES += "$$shell_quote(DOCDIR=\"$$relativeDocPath\")"
    PATH_DEFINES += "$$shell_quote(SHAREDIR=\"$$relativeSharePath\")"
    ICON_TYPE_DEFINE = "$$shell_quote(ICON_TYPE=\"$$ICON_TYPE\")"
}
win32: {
    PATH_DEFINES += "DOCDIR=\"$$relativeDocPath\""
    PATH_DEFINES += "SHAREDIR=\"$$relativeSharePath\""
    ICON_TYPE_DEFINE = "ICON_TYPE=\"$$ICON_TYPE\""
}
DEFINES += $$PATH_DEFINES $$ICON_TYPE_DEFINE
message("DEFINES: $$DEFINES")

include("desktopMenuFile.pri")
destDesktopMenuFile = $$shell_path("$$BUILD_APPLICATIONSDIR/tvclipper.desktop")
QMAKE_POST_LINK += $$getEditDesktopFileCmd("$$destDesktopMenuFile")
QMAKE_POST_LINK += $$getFileCopyCommands($$shell_path("doc"), $$shell_path("$$BUILD_DOCDIR"))
QMAKE_POST_LINK += $$getFileCopyCommands($$shell_path("man"), $$shell_path("$$BUILD_MANDIR"))
QMAKE_POST_LINK += $$getFileCopyCommands($$shell_path("servicemenu"), $$shell_path("$$BUILD_SERVICEMENUDIR"))
QMAKE_POST_LINK += $$getFileCopyCommands($$shell_path("share/icons/tvclipper.svg"), $$shell_path("$$BUILD_PATH_TO_APP_SCALABLE_ICONS"))
QMAKE_POST_LINK += $$getFileCopyCommands($$shell_path("icons"), $$shell_path("$$BUILD_SHAREDIR/icons"))

QMAKE_DISTCLEAN += "$$destDesktopMenuFile"
QMAKE_DISTCLEAN += "$$getCopiedFilesToBuild($$shell_path("doc"), $$shell_path("$$BUILD_DOCDIR"))"
QMAKE_DISTCLEAN += "$$getCopiedFilesToBuild($$shell_path("man"), $$shell_path("$$BUILD_MANDIR"))"
QMAKE_DISTCLEAN += "$$getCopiedFilesToBuild($$shell_path("servicemenu"), $$shell_path("$$BUILD_SERVICEMENUDIR"))"
QMAKE_DISTCLEAN += "$$getCopiedFilesToBuild($$shell_path("share/icons/tvclipper.svg"), $$shell_path("$$BUILD_PATH_TO_APP_SCALABLE_ICONS"))"
QMAKE_DISTCLEAN += "$$getCopiedFilesToBuild($$shell_path("icons"), $$shell_path("$$BUILD_SHAREDIR/icons"))"

message("QMAKE_DISTCLEAN: $$QMAKE_DISTCLEAN")

# ***********************************************************************************************
# *** installation settings

CONFIG(release, debug|release) {
    docFiles.path = $$shell_path("$$INST_DOCDIR")
    docFiles.files = $$getShareFiles("doc")
    iconFiles.path = $$shell_path("$$INST_SHAREDIR/icons")
    iconFiles.files = $$getShareFiles("icons")
    INSTALLS += docFiles iconFiles

    unix: {
        manFiles.path = $$shell_path("$$INST_MANDIR")
        manFiles.files = $$getShareFiles("man")
        desktopMenuFile.path = $$shell_path("$$INST_APPLICATIONSDIR")
        desktopMenuFile.files = $$getShareFiles("desktopMenuFile")
        servicemenuFiles.path = $$shell_path("$$INST_SERVICEMENUDIR")
        servicemenuFiles.files = $$getShareFiles("servicemenu")
        appIconFile.path = $$shell_path("$$INST_PATH_TO_APP_SCALABLE_ICONS")
        appIconFile.files = $$shell_path("share/icons/tvclipper.svg")

        INSTALLS += manFiles desktopMenuFile servicemenuFiles appIconFile
    }

    target.path = $$shell_path("$$INST_BINDIR")
    INSTALLS += target
}

message("commands before linking: $$escape_expand(\\n\\t)$$QMAKE_PRE_LINK")
message("commands after linking: $$escape_expand(\\n\\t)$$QMAKE_POST_LINK")
message("INSTALLS: $$INSTALLS")
