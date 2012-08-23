# Copyright 2011 Kurtis L. Nusbaum
# 
# This file is part of UDJ.
# 
# UDJ is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
# 
# UDJ is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with UDJ.  If not, see <http://www.gnu.org/licenses/>.


configure_file(
  "${PROJECT_SOURCE_DIR}/src/ConfigDefs.hpp.in"
  "${PROJECT_BINARY_DIR}/src/ConfigDefs.hpp"
)

INCLUDE( AddAppIconMacro )

SET(SOURCES 
  main.cpp 
  PlaybackWidget.cpp
  MetaWindow.cpp 
  MusicFinder.cpp 
  DataStore.cpp
  ActivePlaylistView.cpp
  LibraryView.cpp
  UDJServerConnection.cpp
  JSONHelper.cpp
  qt-json/json.cpp
  LoginWidget.cpp
  ActivityList.cpp
  PlayerCreationWidget.cpp
  WidgetWithLoader.cpp
  MusicModel.cpp
  LoginDialog.cpp
  PlayerCreateDialog.cpp
  simpleCrypt/simplecrypt.cpp
  LibraryWidget.cpp
  ActivePlaylistModel.cpp
  PlayerDashboard.cpp
  Utils.cpp
  Logger.cpp
  LogViewer.cpp
  AboutWidget.cpp
  AddressWidget.cpp
  SetLocationDialog.cpp
  SetPasswordDialog.cpp
  DialogWithLoaderWidget.cpp
  PlaybackErrorMessage.cpp
)

IF(APPLE)
  SET( SOURCES ${SOURCES} mac/UDJApp_Mac.mm)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/mac)
ENDIF(APPLE)

include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src")

if(APPLE AND CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
  set(CMAKE_INSTALL_PREFIX "/Applications")
endif()


if(UNIX AND NOT APPLE)
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
  set(BIN_INSTALL_DIR "bin")
  set(DOC_INSTALL_DIR "share/doc/${PROJECT_NAME_LOWERCASE}/")
else()
  set(BIN_INSTALL_DIR ".")
  set(DOC_INSTALL_DIR ".")
endif()

set(ICONS_DIR "${PROJECT_SOURCE_DIR}/icons")

if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
  add_definitions(-Wall -Wextra)
  if(CMAKE_BUILD_RELEASE)
    add_definitions(-Werror)
  endif()
endif()

set(ICON_SOURCES "")
kde4_add_app_icon( ICON_SOURCES "${CMAKE_SOURCE_DIR}/icons/udjlauncher*.png" )


if(APPLE)
set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_NAME} ${PROJECT_VERSION}")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
set(MACOSX_BUNDLE_COPYRIGHT "${PROJECT_COPYRIGHT_YEAR} ${PROJECT_VENDOR}")
set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")

set(MACOSX_BUNDLE_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources")
set(MACOSX_BUNDLE_ICON "${ICONS_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${MACOSX_BUNDLE_RESOURCES})
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MACOSX_BUNDLE_ICON} ${MACOSX_BUNDLE_RESOURCES})
endif()

set(UDJ_EXTRA_LIBS "")
if(WIN32)
  list(APPEND UDJ_EXTRA_LIBS  ${QTSPARKLE_LIBRARIES})
elseif(APPLE)
  find_library(SPARKLE Sparkle)
  list(APPEND UDJ_EXTRA_LIBS ${SPARKLE})
endif(WIN32)

message(STATUS "EXTRA ${UDJ_EXTRA_LIBS}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${MOC_SOURCES} ${ICON_SOURCES})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${PHONON_LIBS} ${TAGLIB} ${UDJ_EXTRA_LIBS})

if(APPLE)
  set(CMAKE_INSTALL_PREFIX "/Applications")
endif()
install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_INSTALL_DIR})

IF(CUSTOM_CA_CERT)
  INSTALL(FILES ${PROJECT_BINARY_DIR}/serverca.pem DESTINATION ${BIN_INSTALL_DIR})
ENDIF(CUSTOM_CA_CERT)


set(LICENSE_FILE "${PROJECT_SOURCE_DIR}/LICENSE")
#set(README_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(MANPAGE_FILE "${PROJECT_SOURCE_DIR}/doc/UDJ.1")
if(NOT APPLE)
install(FILES "${LICENSE_FILE}"  "${MANPAGE_FILE}" DESTINATION ${DOC_INSTALL_DIR})
endif()


IF(WIN32)
  #We need to look for and include the openssl libs because some asshole made
  #openssl export controlled and isn't installed by default on windows
  FIND_PACKAGE(OpenSSLCross REQUIRED)
#   get_filename_component(OPENSSL_ROOT ${OPENSSL_INCLUDE_DIR} PATH)
#   set(SSL_LIBS_DLLS "${OPENSSL_ROOT}/bin/libeay32.dll" "${OPENSSL_ROOT}/bin/ssleay32.dll")
#
#   INSTALL(FILES ${SSL_LIBS_DLLS} DESTINATION ${BIN_INSTALL_DIR})


  #Do some fancy foot work to get phonon vlc working on widnows, this is so stupid and I hate it.


  #vlc libs
#   get_filename_component(LIBVLC_ROOT ${LIBVLC_LIBRARY} PATH)
#   set(VLC_LIB_DLLS "${LIBVLC_ROOT}/libvlc.dll")
#   INSTALL(FILES ${VLC_LIB_DLLS} DESTINATION ${BIN_INSTALL_DIR})


  #And the friggen PHONON_VLC_DLL
#   find_file(PHONON_VLC_DLL phonon_vlc.dll
#     HINT "$ENV{PHONON_VLC_ROOT_DIR}bin/phonon_backend"
#   )
#   if(NOT PHONON_VLC_DLL)
#     message(FATAL_ERROR "Could not find phonon vlc backend")
#   else(NOT PHONON_VLC_DLL)
#     message(STATUS "Found Phonon VLC Backend DLL: ${PHONON_VLC_DLL}")
#   endif(NOT PHONON_VLC_DLL)
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_VENDOR}")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/${README_FILE}")
SET(CPACK_PACKAGE_DESCRIPTION "UDJ: The Social Music Player")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libtag1c2a, libqt4-network, libqt4-sql-sqlite, libqtcore4, libqtgui4, phonon")
SET(CPACK_PACKAGE_CONTACT "klnusbaum@bazaarsolutions.com")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "UDJ is a social music player that allows you and your friends to controll a single music player from your smart phones")
SET(CPACK_DEBIAN_PACKAGE_SECTION "SOUND")
