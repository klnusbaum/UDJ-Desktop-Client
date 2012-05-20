#
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

cmake_minimum_required(VERSION 2.8.7)
PROJECT(UDJ)
SET(PROJECT_VERSION_MAJOR 0)
SET(PROJECT_VERSION_MINOR 5)
SET(PROJECT_VERSION_PATCH 0)
SET(PROJECT_COPYRIGHT_YEAR 2011)
SET(PROJECT_VERNDOR "Bazaar Solutions")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_COPYRIGHT_YEAR "2011")
set(PROJECT_DOMAIN_FIRST "klnusbaum")
set(PROJECT_DOMAIN_SECOND "org")
set(PROJECT_DOMAIN "${PROJECT_DOMAIN_FIRST}.${PROJECT_DOMAIN_SECOND}")
FIND_PACKAGE(Qt4 4.4.3 
  COMPONENTS QtCore QtGui Phonon QtSql QtNetwork REQUIRED)

FIND_LIBRARY(TAGLIB "tag" PATH)
IF(NOT TAGLIB)
  MESSAGE(fatal_error "Could not find taglib")
ENDIF(NOT TAGLIB)

FIND_PATH(TAGLIB_HEADER_PATH "taglib/taglib.h" PATH)
IF(NOT TAGLIB_HEADER_PATH)
  message(FATAL_ERROR "Couldn't find path to taglib header")
ENDIF(NOT TAGLIB_HEADER_PATH)
include_directories("${TAGLIB_HEADER_PATH}/taglib")

set(UDJ_DEBUG_BUILD FALSE CACHE BOOL "Enables/Disables a debug build of UDJ")

IF(UDJ_DEBUG_BUILD)
  ADD_DEFINITIONS(-DUDJ_DEBUG_BUILD)
ENDIF(UDJ_DEBUG_BUILD)


#CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/res/udjlauncher.svg
#  ${PROJECT_BINARY_DIR}/udjlauncher.svg COPYONLY)

set(HAS_CUSTOM_CA_CERT 0)
IF(CUSTOM_CA_CERT)
CONFIGURE_FILE(${CUSTOM_CA_CERT}
  ${PROJECT_BINARY_DIR}/serverca.pem COPYONLY)
set(HAS_CUSTOM_CA_CERT 1)
ENDIF(CUSTOM_CA_CERT)

set(IS_APPLE_BUILD 0)
set(IS_WINDOWS_BUILD 0)
IF(APPLE)
  set(IS_APPLE_BUILD 1)
ELSEIF(WIN32)
 set(IS_WINDOWS_BUILD 1)
ENDIF()


ADD_SUBDIRECTORY(src)
