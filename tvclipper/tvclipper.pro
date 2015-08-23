#-------------------------------------------------
#
# Project created by QtCreator 2005-07-05T10:52:45
#
#-------------------------------------------------

CONFIG += c++11 # using macro PRId64

# modul Qt XML je zastaralý; use the QXmlStreamReader and QXmlStreamWriter classes in Qt Core instead.
QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tvclipper
TEMPLATE = app


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
    stream.cpp \
    mpgIndex.cpp

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
    mpgIndex.h

FORMS    += tvclipper.ui \
    DlgExport.ui \
    DlgMplayerError.ui \
    DlgProgress.ui

OTHER_FILES += \
    tvclipper_en.html \
    icons/bookmark.svgz \
    icons/chapter.svgz \
    icons/play.svgz \
    icons/stop.svgz \
    icons/bookmark.png \
    icons/chapter.png \
    icons/play.png \
    icons/stop.png \
    TODO

CONFIG(debug, debug|release) {
    unix: {
        PREVIOUS_DIR = "xyz"
        for(FILE, $$list($$OTHER_FILES)){
            DIR = "$${dirname(FILE)}"
            DEST_FILE = "$${OUT_PWD}/$${FILE}"

            !equals(DIR, "$$PREVIOUS_DIR") {
                !exists(DEST_FILE) {
                    !isEmpty(DIR) {
                            COMMAND = "$$QMAKE_COPY_DIR $$quote($${PWD}/$${dirname(FILE)}) $$quote($${OUT_PWD}/$${DIR})"
                            QMAKE_POST_LINK += "$$COMMAND $$escape_expand(\\n\\t)"
                            message("$${COMMAND}")
                    } else {
                            COMMAND = "$$QMAKE_COPY_FILE $$quote($${PWD}/$${FILE}) $$quote($${DEST_FILE})"
                            QMAKE_POST_LINK += "$$COMMAND $$escape_expand(\\n\\t)"
                            message("$$COMMAND")
                    }
                }
            }

            PREVIOUS_DIR = "$$DIR"
        }
    }
}


libraries += swscale avcodec avformat avdevice avutil a52 mad ao

INCLUDEPATH += "/usr/include"
# INCLUDEPATH += "$$_{PRO_FILE_PWD_}/"

for (library, $$list($$libraries)) {
    LIBS += -l$$lower($$library)
    DEFINES += HAVE_LIB_$$upper($$library)
}

RESOURCES +=

QMAKE_DISTCLEAN += "Makefile"
