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

defineTest(isRelativePath) {
    path = "$$1"

    contains(path, "^/.*") {
        return(false)
    }

    return(true)
}

defineReplace(getLibOrLib64Str) {
    architecture = $$system("uname -m")
    equals(architecture, "x86_64") {
        return("lib64")
    }

    return("lib")
}

defineTest(existsA52Lib) {
    headerFile = "$$SYSTEM_INCLUDEDIR/a52dec/a52.h"
    libFile = "$$SYSTEM_LIBDIR/liba52.so"


    !exists("$$headerFile") {
        return(false)
    }
    !exists("$$libFile") {
        return(false)
    }

    return(true)
}


defineTest(setLibs) {
    libraries = $$1
    usingA52Lib = false

    for(library, $$list($$libraries)) {
        name = "$$lower($$library)"
        unix: {
            equals(name, "a52") {
                existsA52Lib() {
                    LIBS += -l$$lower($$name)
                    DEFINES += HAVE_LIB_$$upper($$library)
                    usingA52Lib = true
                }
            } else {
                packagesExist("$$name") {
                    PKGCONFIG += $$name
                    DEFINES += HAVE_LIB_$$upper($$library)
                }
            }
        }
    }

    equals(usingA52Lib, true) {
        export(LIBS)
    }
    export(PKGCONFIG)
    export(DEFINES)
}

defineTest(setFFmpegLibs) {
    libraries = $$1

    for(library, $$list($$libraries)) {
        name = "$$lower($$library)"

        unix: {
            packageName = "lib$$name"
            packagesExist("$$packageName") {
                PKGCONFIG += "$$packageName"
                DEFINES += HAVE_LIB_$$upper($$library)
            }
        }

        win32: {
            staticLibPath = "$$shell_path($$clean_path($${_PRO_FILE_PWD_}/../ffmpeg/lib$$name/))"
            staticLibFile = "$$shell_path($$clean_path($${staticLibPath}/lib$${name}.a))"
            sharedLibPath = "$$shell_path($$clean_path($${OUT_PWD}/$$SHAREDIR/ffmpeg/lib$$name/))"
            sharedLibFile = "$$shell_path($$clean_path($$sharedLibPath/lib$${name}.so))"
            libPath = "$$staticLibPath"
            libFile = "$$staticLibFile"
            exists("$$sharedLibFile") {
                libFile = "$$sharedLibFile"
                libPath = "$$sharedLibPath"
            }
            LIBS += -L$$libPath -l$$name
            DEPENDPATH += "$$libPath"
            PRE_TARGETDEPS += "$$libFile"
            DEFINES += HAVE_LIB_$$upper($$library)
        }
    }

    win32: INCLUDEPATH += "$$shell_path($$clean_path($$_PRO_FILE_PWD_/../ffmpeg/))"

    export(DEFINES)
    unix: export(PKGCONFIG)
    win32: export(LIBS)
    win32: export(DEPENDPATH)
    win32: export(PRE_TARGETDEPS)
    win32: export(INCLUDEPATH)
}

defineReplace(getFirstValOrDefVal) {
    value = $$1
    count(ARGS, 2) {
        defaultValue = $$last(ARGS)
    }

    isEmpty(value) {
        !isEmpty(defaultValue) {
            value = "$$defaultValue"
        }
    }

    return($$value)
}

defineReplace(getShareFiles) {
    path = "$$1"

    filePathReExp = $$clean_path("share/$${path}/.*")
    for(file, $$list($$DISTFILES)) {
        contains(file, $$filePathReExp) {
            fileList += "$$file"
        }
    }

    return($$fileList)
}

defineReplace(getFileCopyCommands) {
    sourcePath = $$first(ARGS)
    destPath = $$shell_path($$clean_path($$OUT_PWD/$$last(ARGS)))

    !exists("$$shell_path("$${_PRO_FILE_PWD_}/$$sourcePath")") {
        # sourcePath contains sub-path in share dir from DISTFILES variable
        fileList = $$getShareFiles("$$sourcePath")
    } else {
        # sourcePath is file from DISTFILES variable
        fileList = "$$sourcePath"
    }
    !exists($$destPath) {
        commands += "$$QMAKE_MKDIR $$shell_quote($$destPath)$$escape_expand(\\n\\t)"
    }

    for(file, $$list($$fileList)) {
        sourceFile = "$$shell_path($$clean_path("$${_PRO_FILE_PWD_}/$$file"))"
        commands *= "$$QMAKE_COPY_FILE $$shell_quote($$sourceFile) $$shell_quote($$destPath)$$escape_expand(\\n\\t)"
    }

    return($$commands)
}

defineReplace(getCopiedFilesToBuild) {
    sourcePath = $$first(ARGS)
    destPath = $$shell_path($$clean_path($$last(ARGS)))

    !exists("$$shell_path($${_PRO_FILE_PWD_}/$$sourcePath)") {
        # sourcePath contains sub-path in share dir from DISTFILES variable
        fileList = $$getShareFiles("$$sourcePath")
    } else {
        # sourcePath is file from DISTFILES variable
        fileList = "$$sourcePath"
    }

    for(file, $$list($$fileList)) {
        fileName = "$$basename(file)"
        destFile = "$$shell_path($$clean_path("$${destPath}/$${fileName}"))"
        result *= "$$shell_quote($$destFile)"
    }

    return($$result)
}

defineReplace(getEditDesktopFileCmd) {
    destFile = "$$shell_path($$clean_path("$$OUT_PWD/$$1"))"
    destPath = "$$dirname(destFile)"
    sourceFile = "$$shell_path($$clean_path("$$_PRO_FILE_PWD_/desktopMenuFile/tvclipper.desktop"))"
    fileForEdit = "$$shell_path($$clean_path("$$_PRO_FILE_PWD_/share/desktopMenuFile/tvclipper.desktop"))"

    unix: {
        desktopDocLine = $$shell_quote("$$desktopDocLine")
        desktopExecLine = $$shell_quote("$$desktopExecLine")
        desktopTryExecLine = $$shell_quote("$$desktopTryExecLine")
    }

    commands += "$$QMAKE_MKDIR $$shell_quote("$$destPath")$$escape_expand(\\n\\t)"
    commands += "$$QMAKE_COPY_FILE $$shell_quote("$$sourceFile") $$shell_quote("$$fileForEdit")$$escape_expand(\\n\\t)"
    commands += "echo $$desktopDocLine >> $$shell_quote("$$fileForEdit")$$escape_expand(\\n\\t)"
    commands += "echo $$desktopExecLine >> $$shell_quote("$$fileForEdit")$$escape_expand(\\n\\t)"
    commands += "echo $$desktopTryExecLine >> $$shell_quote("$$fileForEdit")$$escape_expand(\\n\\t)"
    commands += "$$QMAKE_COPY_FILE $$shell_quote("$$fileForEdit") $$shell_quote("$$destFile")$$escape_expand(\\n\\t)"

    return("$$commands")
}
