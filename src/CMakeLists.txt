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
  ErrorMessage.cpp
)


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

set(RESOURCES_SOURCES "")
if(MSVC)
  list(APPEND RESOURCES_SOURCES ${PROJECT_SOURCE_DIR}/src/udj.rc)
endif()

if(APPLE)
set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_NAME} ${PROJECT_VERSION}")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
set(MACOSX_BUNDLE_COPYRIGHT "${PROJECT_COPYRIGHT_YEAR} ${PROJECT_VENDOR}")
set(MACOSX_BUNDLE_ICON_FILE "udjlauncher.icns")
set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")

set(MACOSX_BUNDLE_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources")
set(MACOSX_BUNDLE_ICON "${ICONS_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${MACOSX_BUNDLE_RESOURCES})
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MACOSX_BUNDLE_ICON} ${MACOSX_BUNDLE_RESOURCES})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${MOC_SOURCES} ${RESOURCES_SOURCES})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${TAGLIB})

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
  FIND_PACKAGE(OpenSSL)
  if(NOT OPENSSL_FOUND)
    message(fatal_error "Cannot find OpenSSL. This is required on windows")
  endif()
  get_filename_component(OPENSSL_ROOT ${OPENSSL_INCLUDE_DIR} PATH)
  set(SSL_LIBS "${OPENSSL_ROOT}/bin/libeay32.dll" "${OPENSSL_ROOT}/bin/ssleay32.dll")

  INSTALL(FILES ${SSL_LIBS} DESTINATION ${BIN_INSTALL_DIR})
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
if(WIN32)
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "${PROJECT_NAME}")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
  set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME} ${PROJECT_VERSION}")
  set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY "${BIN_INSTALL_DIR}")
  #set(CPACK_NSIS_MUI_ICON "${PROJECT_ICONS_DIRECTORY}/NSIS.ico")
  #set(CPACK_PACKAGE_ICON "${PROJECT_ICONS_DIRECTORY}\\\\NSISHeader.bmp")
  set(CPACK_NSIS_URL_INFO_ABOUT "http://${PROJECT_DOMAIN}")
  #set(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
  #set(CPACK_NSIS_MENU_LINKS "${LICENSE_FILE}" "License" "${README_FILE}" "Readme")
  #set(CPACK_NSIS_MUI_FINISHPAGE_RUN "${CPACK_NSIS_INSTALLED_ICON_NAME}")
endif()
if(APPLE)
  set(CMAKE_INSTALL_PREFIX "/Applications")
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_DMG_FORMAT "UDBZ")
  set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
  set(CPACK_SYSTEM_NAME "OSX")
  set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}")
  set(CPACK_PACKAGE_ICON "${ICONS_DIR}/udjlauncher.icns")
  #set(CPACK_DMG_DS_STORE "${ICONS_DIR}/DMGDSStore")
  #set(CPACK_DMG_BACKGROUND_IMAGE "${ICONS_DIR}/DMGBackground.png")
endif()

include(CPack)


set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BIN_INSTALL_DIR}")
include(InstallRequiredSystemLibraries)
if(APPLE)
  set(EXECUTABLE "${PROJECT_NAME}.app")
elseif(WIN32)
  set(EXECUTABLE "${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
else()
  set(EXECUTABLE "${BIN_INSTALL_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
endif()

set(UDJ_QT_PLUGINS "qsqlite")
if(APPLE)
  list(APPEND UDJ_QT_PLUGINS phonon_qt7)
elseif(WIN32)
  list(APPEND UDJ_QT_PLUGINS phonon_ds9)
endif()

if(APPLE OR WIN32)
  include(DeployQt4)
  install_qt4_executable("${EXECUTABLE}" "${UDJ_QT_PLUGINS}")
endif()
