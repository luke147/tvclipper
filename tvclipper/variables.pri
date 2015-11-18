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


# if variables are empty:
# it means that install directory is: /opt/<app-name>
# necessary system files will be installed to system subdirectories to directory: [SYSSHAREDIR]/[SYSSHARE_PREFIX] (default is: /usr/share)

ICON_TYPE = "png" # types: svg, png

unix: {
    # Empty variables means install to directory: /opt/<app-name>
    # You can specify absolute or relative paths. Examples for relative path are below.

    # for finding liba52 library
    # SYSTEM_LIBDIR = "/usr/lib/x86_64-linux-gnu" # [SYSTEM_LIBDIR]/liba52.so  =>  in Debian x86_64 (in Mageia /usr/lib or /usr/lib64)
    # SYSTEM_INCLUDEDIR = "/usr/include"          # [SYSTEM_INCLUDEDIR]/a52dec/a52.h

    # PREFIX = "/usr"
    # BINDIR = "bin"               # [PREFIX]/[BINDIR]
    # SHAREDIR = "share/tvclipper" # [PREFIX]/[SHAREDIR]  => The directory containts tvclipper files. It isn't /usr/share.

    # SYSSHARE_PREFIX = "/usr"
    # SYSSHAREDIR = "share"       # [SYSSHARE_PREFIX]/[SYSSHAREDIR]
    # DOCDIR = "doc/tvclipper"    # [SYSSHARE_PREFIX]/[SYSSHAREDIR]/[DOCDIR]
    # ICONDIR = "icons"           # [SYSSHARE_PREFIX]/[SYSSHAREDIR]/[ICONDIR]
    # MANDIR = "man/man1"         # [SYSSHARE_PREFIX]/[SYSSHAREDIR]/[MANDIR]
    # APPLICATIONSDIR = "applications" # [SYSSHARE_PREFIX]/[SYSSHAREDIR]/[APPLICATIONSDIRDIR]
    # SERVICEMENUDIR = "kde4/services/ServiceMenus" # [SYSSHARE_PREFIX]/[SYSSHAREDIR]/[SERVICEMENUDIR]
    # PATH_TO_APP_SCALABLE_ICONS = "hicolor/scalable/apps" # [SYSSHARE_PREFIX]/[SYSSHAREDIR]/[ICONDIR]/[PATH_TO_APP_SCALABLE_ICONS]

    # BUILD_PATH = "build"      # build-tvclipper-Release/[BUILD_PATH] or build-tvclipper-Debug/[BUILD_PATH]
}

# TV Clipper doesn't have support for Windows.
# win32: {
#     PREFIX = "C:/Program Files"
#     BINDIR = "tvclipper"        # [PREFIX]/[BINDIR]
#     SHAREDIR = "tvclipper"      # [PREFIX]/[SHAREDIR]
#     DOCDIR = "doc"              # [PREFIX]/[SHAREDIR]/[DOCDIR]

#     BUILD_PATH = "build"      # build-tvclipper-Release/[BUILD_PATH] or build-tvclipper-Debug/[BUILD_PATH]
# }
