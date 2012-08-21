/**
 * Copyright 2011 Kurtis L. Nusbaum
 * 
 * This file is part of UDJ.
 * 
 * UDJ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * UDJ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with UDJ.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "MusicFinder.hpp"
#include "Logger.hpp"
#include "ConfigDefs.hpp"
#include "DataStore.hpp"
#include <QRegExp>
#include <QXmlSimpleReader>
#include <QXmlDefaultHandler>
#include <phonon/backendcapabilities.h>

namespace UDJ{

class iTunesHandler : public QXmlDefaultHandler{

public:
  bool characters(const QString& ch){
    if(ch.startsWith("file://")){
      QUrl songUrl(ch);
      QString file = songUrl.path();
      #if IS_WINDOWS_BUILD
      Logger::instance()->log("Windows build, removing leading slash");
      file = file.remove(0,1);
      #endif
      Logger::instance()->log("Checking path: " + file);
      QFileInfo info(file);
      if(info.isFile()){
        foundFiles.append(Phonon::MediaSource(file));
      }
    }
    return true;
  }

  QList<Phonon::MediaSource> foundFiles;

};

QList<Phonon::MediaSource> MusicFinder::filterDuplicateSongs(
  const QList<Phonon::MediaSource>& songsToFilter, const DataStore* dataStore)
{
  QList<Phonon::MediaSource> toReturn;
  Phonon::MediaSource song;
  Q_FOREACH(song, songsToFilter){
    if(!dataStore->alreadyHaveSongInLibrary(song.fileName())){
      toReturn.append(song);
    }
  }
  return toReturn;
}

QList<Phonon::MediaSource> MusicFinder::findItunesMusic(const QString& itunesLibFileName, const DataStore* dataStore){
  iTunesHandler handler;
  QFile itunesLibFile(itunesLibFileName);
  QXmlSimpleReader itunesReader;
  QXmlInputSource *source = new QXmlInputSource(&itunesLibFile);
  itunesReader.setContentHandler(&handler);
  itunesReader.setErrorHandler(&handler);
  itunesReader.parse(source);
  return filterDuplicateSongs(handler.foundFiles, dataStore);
}

QList<Phonon::MediaSource> MusicFinder::findMusicInDir(const QString& musicDir, const DataStore* dataStore){
  QRegExp fileMatcher = getMusicFileMatcher();
  return filterDuplicateSongs(findMusicInDirWithMatcher(musicDir, fileMatcher), dataStore);
}

QList<Phonon::MediaSource> MusicFinder::findMusicInDirWithMatcher(
    const QString& musicDir, const QRegExp& fileMatcher)
{
  QList<Phonon::MediaSource> toReturn;
  QDir dir(musicDir);
  QFileInfoList potentialFiles = dir.entryInfoList(QDir::Dirs| QDir::Files | QDir::NoDotAndDotDot);
  QFileInfo currentFile;
  for(int i =0; i < potentialFiles.size(); ++i){
    currentFile = potentialFiles[i];
    if(currentFile.isFile() && fileMatcher.exactMatch(currentFile.fileName())){
      toReturn.append(Phonon::MediaSource(currentFile.absoluteFilePath()));
    }
    else if(currentFile.isDir()){
      toReturn.append(findMusicInDirWithMatcher(
            dir.absoluteFilePath(currentFile.absoluteFilePath()), fileMatcher));
    }
  }
  return toReturn;
}

QRegExp MusicFinder::getMusicFileMatcher(){
  QStringList availableTypes = availableMusicTypes();
  QString matcherString="";
  for(int i=0;i<availableTypes.size();++i){
    if(i==availableTypes.size()-1){
      matcherString += "(.*" + availableTypes.at(i) + ")";
    }
    else{
      matcherString += "(.*" + availableTypes.at(i) + ")|";
    }
  }
  Logger::instance()->log("Matcher REGEX: " + matcherString);
  QRegExp matcher(matcherString);
  return matcher;
}

QString MusicFinder::getMusicFileExtFilter(){
  QStringList availableTypes = availableMusicTypes();
  QString filterString="(";
  for(int i=0;i<availableTypes.size();++i){
    if(i==availableTypes.size()-1){
      filterString += "*." + availableTypes.at(i);
    }
    else{
      filterString += "*." + availableTypes.at(i) + " ";
    }
  }
  filterString += ")";
  Logger::instance()->log("File Ext Filter: " + filterString);
  return filterString;
}

QStringList MusicFinder::availableMusicTypes(){
  #if IS_APPLE_BUILD
  Logger::instance()->log("On mac, just saying mp3s and m4as");
  QStringList toReturn;
  toReturn << "mp3" << "m4a";
  return toReturn;
  #else
  QStringList mimes = Phonon::BackendCapabilities::availableMimeTypes();
  if(mimes.size() == 0){
    Logger::instance()->log("Didn't find any mime types");
  }
  Logger::instance()->log("Found mime types:");
  Q_FOREACH(QString s, mimes){
    Logger::instance()->log(s);
  }
  QStringList toReturn;
  if(mimes.contains("audio/flac") || mimes.contains("audio/x-flac")){
    toReturn.append("flac");
  }
  if(mimes.contains("audio/mp3") || mimes.contains("audio/x-mp3")){
    toReturn.append("mp3");
  }
  if(mimes.contains("audio/mp4")){
    toReturn.append("mp4");
  }
  if(mimes.contains("audio/m4a") 
    || ("audio/x-m4a") 
|| mimes.contains("applications/x-qt-m4a")){
    toReturn.append("m4a");
  }
  if(mimes.contains("audio/wav") || mimes.contains("audio/x-wav")){
    toReturn.append("wav");
  }
  if(mimes.contains("audio/ogg") ||
mimes.contains("application/ogg") ||
mimes.contains("audio/x-vorbis") || 
mimes.contains("audio/x-vorbis+ogg"))
{
    toReturn.append("ogg");
  }
  return toReturn;
  #endif
}

} //end namespace
