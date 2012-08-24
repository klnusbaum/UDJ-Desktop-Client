# CMake module to search for the Phonon VLC plugin
# Authors: Kurtis Nusbaum <klnusbaum@bazaarsolutions.com>
# 
# Copyright 2012 Kurtis Nusbaum.
# LICENSE: GPLv2
#
# If it's found it sets PHONON_VLC_FOUND to TRUE
# and following variables are set:
#    PHONON_VLC_DLL
# Please note this has only been tested on windows. Sorry.


find_path(PHONON_VLC_DLL bin/phonon_backend/phonon_vlc.dll
HINTS "$ENV{PHONON_VLC_ROOT_DIR}"
)
if (PHONON_VLC_DLL)
set(PHONON_VLC_FOUND TRUE)
endif (PHONON_VLC_DLL)


if (PHONON_VLC_FOUND)
    if (NOT PhononVLC_FIND_QUIETLY)
        message(STATUS "Found VLC DLL: ${PHONON_VLC_DLL}")
    endif (NOT PhononVLC_FIND_QUIETLY)
else (PHONON_VLC_FOUND)
    if (PhononVLC_REQUIRED)
        message(FATAL_ERROR "Couldn't Phonon VLC")
    endif (PhononVLC_REQUIRED)
endif (PHONON_VLC_FOUND)

