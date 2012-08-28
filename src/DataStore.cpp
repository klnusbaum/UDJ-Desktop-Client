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
#include "DataStore.hpp"
#include "UDJServerConnection.hpp"
#include "Utils.hpp"
#include "Logger.hpp"

#include <QDir>
#include <QDesktopServices>
#include <QDir>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlRecord>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QProgressDialog>
#include <QSqlError>

#include <tag.h>
#include <tstring.h>
#include <fileref.h>


namespace UDJ{


DataStore::DataStore(
  const QString& username,
  const QString& password,
  const QByteArray& ticket,
  const user_id_t& userId,
  QObject *parent)
  :QObject(parent),
  username(username),
  password(password),
  isReauthing(false),
  changingPlayerState(false),
  clearingCurrentSong(false),
  currentSongId(-1)
{
  serverConnection = new UDJServerConnection(this);
  serverConnection->setTicket(ticket);
  serverConnection->setUserId(userId);
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  if(settings.contains(getPlayerIdSettingName())){
    serverConnection->setPlayerId(settings.value(getPlayerIdSettingName()).value<player_id_t>());
  }
  activePlaylistRefreshTimer = new QTimer(this);
  activePlaylistRefreshTimer->setInterval(5000);
  participantRefreshTimer = new QTimer(this);
  participantRefreshTimer->setInterval(5000);
  setupDB();

  connect(serverConnection,
      SIGNAL(playerStateSet(const QString&)),
      this,
      SLOT(onPlayerStateSet(const QString&)));

  connect(
    serverConnection,
    SIGNAL(playerStateSetError(const QString&, const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onPlayerStateSetError(const QString&, const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(playerPasswordSetError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onPlayerPasswordSetError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(playerLocationSetError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onPlayerLocationSetError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(playerPasswordRemoveError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onPlayerPasswordRemoveError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(libSongsSyncedToServer(const QSet<library_song_id_t>&)),
    this,
    SLOT(setLibSongsSynced(const QSet<library_song_id_t>&)));

  connect(
    serverConnection,
    SIGNAL(playerCreated(const player_id_t&)),
    this,
    SLOT(onPlayerCreate(const player_id_t&)));

  connect(
    serverConnection,
    SIGNAL(playerCreationFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onPlayerCreationFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(newActivePlaylist(const QVariantMap&)),
    this,
    SLOT(setActivePlaylist(const QVariantMap&)));

  connect(
    serverConnection,
    SIGNAL(getActivePlaylistFail(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onGetActivePlaylistFail(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(activePlaylistRefreshTimer,
    SIGNAL(timeout()),
    this,
    SLOT(refreshActivePlaylist()));

  connect(
    serverConnection,
    SIGNAL(currentSongSet()),
    this,
    SLOT(refreshActivePlaylist()));

  connect(
    participantRefreshTimer,
    SIGNAL(timeout()),
    this,
    SLOT(refreshParticipantList()));

  connect(
    serverConnection,
    SIGNAL(libModError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onLibModError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(setCurrentSongFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onSetCurrentSongFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(activePlaylistModified(const QSet<library_song_id_t>&, const QSet<library_song_id_t>&)),
    this,
    SLOT(onActivePlaylistModified(const QSet<library_song_id_t>&, const QSet<library_song_id_t>&)));

  connect(
    serverConnection,
    SIGNAL(activePlaylistModFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onActivePlaylistModFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(setVolumeFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onSetVolumeFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));


  connect(
    serverConnection,
    SIGNAL(authenticated(const QByteArray&, const user_id_t&)),
    this,
    SLOT(onReauth(const QByteArray&, const user_id_t&)));

  connect(
    serverConnection,
    SIGNAL(authFailed(const QString&)),
    this,
    SLOT(onAuthFail(const QString&)));

  connect(
    serverConnection,
    SIGNAL(currentSongCleared()),
    this,
    SLOT(onCurrentSongCleared()));

  connect(
    serverConnection,
    SIGNAL(currentSongClearError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onCurrentSongClearError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));


  connect(
    serverConnection,
    SIGNAL(newParticipantList(const QVariantList&)),
    this,
    SLOT(onNewParticipantList(const QVariantList&)));

}

void DataStore::setupDB(){
  //TODO do all of this stuff in a seperate thread and return right away.
  QDir dbDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
  if(!dbDir.exists()){
    Logger::instance()->log("DB dir didn't exists. making it at : " + dbDir.absolutePath());
    //TODO handle if this fails
    dbDir.mkpath(dbDir.absolutePath());
  }
  QString dbFilePath = dbDir.absoluteFilePath(getPlayerDBName());
  if(!QFile::exists(dbFilePath)){
    Logger::instance()->log("DB file didn't exist, so creating it at: " + dbFilePath);
    QFile dbFile(dbFilePath);
    dbFile.open(QIODevice::WriteOnly);
    dbFile.close();
  }
  database = QSqlDatabase::addDatabase("QSQLITE", getPlayerDBConnectionName());
  database.setDatabaseName(dbFilePath);
  database.open();

  QSqlQuery setupQuery(database);

  EXEC_SQL(
    "Error creating library table",
    setupQuery.exec(getCreateLibraryQuery()),
    setupQuery)

  EXEC_SQL(
    "Error creating activePlaylist table.",
    setupQuery.exec(getCreateActivePlaylistQuery()),
    setupQuery)

  EXEC_SQL(
    "Error creating activePlaylist view.",
    setupQuery.exec(getCreateActivePlaylistViewQuery()),
    setupQuery)

}

void DataStore::startPlaylistAutoRefresh(){
  Logger::instance()->log("Starting playlist auto refresh");
  activePlaylistRefreshTimer->start();
}

void DataStore::startParticipantsAutoRefresh(){
  Logger::instance()->log("Starting particpants auto refresh");
  participantRefreshTimer->start();
}

void DataStore::clearCurrentSong(){
  currentSongId = -1;
  clearingCurrentSong = true;
  serverConnection->clearCurrentSong();
}

void DataStore::onCurrentSongCleared(){
  clearingCurrentSong = false;
}

void DataStore::onCurrentSongClearError(
  const QString& errMessage,
  int errorCode,
  const QList<QNetworkReply::RawHeaderPair>& headers)
{
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(CLEAR_CURRENT_SONG);
    initReauth();
  }
  else{
    clearingCurrentSong = false;
    emit clearCurrentSongError(errMessage);
  }

}


void DataStore::onPlayerStateSet(const QString& state){
  changingPlayerState = false;
  if(state == getInactiveState()){
    emit playerSuccessfullySetInactive();
  }
}

void DataStore::onPlayerStateSetError(
    const QString& state,
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers)
{
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    if(state != getInactiveState()){
      reauthActions.insert(SET_PLAYER_STATE);
    }
    else{
      reauthActions.insert(SET_PLAYER_INACTIVE);
    }
    initReauth();
  }
  else{
    changingPlayerState = false;
    if(state == getPlayingState()){
      emit playPlayerError(errMessage);
    }
    else if(state == getPausedState()){
      emit pausePlayerError(errMessage);
    }
    else if(state == getInactiveState()){
      emit playerSetInactiveError(errMessage);
    }
  }
}


void DataStore::removePlayerPassword(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.remove(getPlayerPasswordSettingName());
  serverConnection->removePlayerPassword();
  emit playerPasswordRemoved();
}

void DataStore::onPlayerPasswordRemoveError(
  const QString& errMessage,
  int errorCode,
  const QList<QNetworkReply::RawHeaderPair>& headers)
{
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(REMOVE_PLAYER_PASSWORD);
    initReauth();
  }
  else{
    emit playerPasswordRemoveError(errMessage);
  }
}


void DataStore::setPlayerPassword(const QString& newPassword){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerPasswordSettingName(), newPassword);
  serverConnection->setPlayerPassword(newPassword);
  emit playerPasswordSet();
}

void DataStore::onPlayerPasswordSetError(
  const QString& errMessage,
  int errorCode,
  const QList<QNetworkReply::RawHeaderPair>& headers)
{
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(SET_PLAYER_PASSWORD);
    initReauth();
  }
  else{
    emit playerPasswordSetError(errMessage);
  }
}

void DataStore::setPlayerLocation(
  const QString& streetAddress,
  const QString& city,
  const QString& state,
  const QString& zipcode
)
{
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getAddressSettingName(), streetAddress);
  settings.setValue(getCitySettingName(), city);
  settings.setValue(getStateSettingName(), state);
  settings.setValue(getZipCodeSettingName(), zipcode);
  serverConnection->setPlayerLocation(streetAddress, city, state, zipcode);
  emit playerLocationSet();
}


void DataStore::onPlayerLocationSetError(
  const QString& errMessage,
  int errorCode,
  const QList<QNetworkReply::RawHeaderPair>& headers)
{
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(SET_PLAYER_LOCATION);
    initReauth();
  }
  else{
    //TODO handle location not found error
    emit playerLocationSetError(errMessage);
  }
}

void DataStore::pausePlayer(){
  setPlayerState(getPausedState());
}

void DataStore::playPlayer(){
  setPlayerState(getPlayingState());
}

void DataStore::setPlayerState(const QString& newState){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerStateSettingName(), newState);
  changingPlayerState = true;
  serverConnection->setPlayerState(newState);
}

void DataStore::setPlayerInactive(){
  serverConnection->setPlayerState(getInactiveState());
}


void DataStore::addMusicToLibrary(
  const QList<Phonon::MediaSource>& songs, QProgressDialog* progress)
{
  bool isTransacting=database.transaction();
  if(isTransacting){
    Logger::instance()->log("Was able to start transaction");
  }
  QSqlQuery addQuery(database);
  addQuery.prepare(
    "INSERT INTO "+getLibraryTableName()+ 
    "("+
    getLibSongColName() + ","+
    getLibArtistColName() + ","+
    getLibAlbumColName() + ","+
    getLibGenreColName() + "," +
    getLibTrackColName() + "," +
    getLibFileColName() + "," +
    getLibDurationColName() +")" +
    "VALUES ( :song , :artist , :album , :genre, :track, :file, :duration );"
  );

  for(int i =0; i<songs.size(); ++i){
    addSongToLibrary(songs[i], addQuery);
    if(progress != NULL){
      progress->setValue(i);
      if(progress->wasCanceled()){
        if(isTransacting){
          Logger::instance()->log("Rolling back transaction");
          if(!database.rollback()){
            Logger::instance()->log("Roll back failed");
          }
        }
        return;
      }
    }
  }
  if(isTransacting){
    Logger::instance()->log("Committing add transaction");
    database.commit();
  }
}

void DataStore::addSongToLibrary(const Phonon::MediaSource& song, QSqlQuery &addQuery){
  QString fileName = song.fileName();
  QString songName;
  QString artistName;
  QString albumName;
  QString genre;
  int track;
  int duration;
  TagLib::FileRef f(fileName.toStdString().c_str());
  if(!f.isNull() && f.tag() && f.audioProperties()){
    TagLib::Tag *tag = f.tag();
    songName = TStringToQString(tag->title());
    artistName = TStringToQString(tag->artist());
    albumName = TStringToQString(tag->album());
    genre = TStringToQString(tag->genre());
    duration = f.audioProperties()->length();
    track = tag->track();
  }
  else{
    //TODO throw error
    return;
  }

  if(songName == ""){
    songName = unknownSongTitle();
  }
  if(artistName == ""){
    artistName = unknownSongArtist();
  }
  if(albumName == ""){
    albumName = unknownSongAlbum();
  }
  if(genre == ""){
    genre = unknownGenre();
  }

  Logger::instance()->log("adding song with title: " + songName + " to database");

  library_song_id_t hostId =-1;

  addQuery.bindValue(":song", songName);
  addQuery.bindValue(":artist", artistName);
  addQuery.bindValue(":album", albumName);
  addQuery.bindValue(":genre", genre);
  addQuery.bindValue(":track", track);
  addQuery.bindValue(":file", fileName);
  addQuery.bindValue(":duration", duration);
  EXEC_INSERT(
    "Failed to add song library" << songName.toStdString(), 
    addQuery,
    hostId,
    library_song_id_t)
}

bool DataStore::alreadyHaveSongInLibrary(const QString& fileName) const{
  QSqlQuery existsQuery(database);
  existsQuery.prepare(
    "SELECT * FROM "+getLibraryTableName()+ " WHERE " + 
    getLibIsDeletedColName() + "=0 and " + 
    getLibFileColName() + "=\""+fileName+"\";"
  );

  EXEC_SQL(
    "Error executing already in library test query",
    existsQuery.exec(),
    existsQuery)

  return existsQuery.next();

}

void DataStore::removeSongsFromLibrary(const QSet<library_song_id_t>& toRemove,
  QProgressDialog* progress)
{
  bool isTransacting = database.transaction();
  QSqlQuery deleteQuery(database);
  deleteQuery.prepare("UPDATE " + getLibraryTableName() +  " "
    "SET " + getLibIsDeletedColName() + "=1, "+
    getLibSyncStatusColName() + "=" + 
      QString::number(getLibNeedsDeleteSyncStatus()) + " "
    "WHERE " + getLibIdColName() + "= ?"); 
  int i=0;
  Q_FOREACH(library_song_id_t id, toRemove){
    deleteQuery.bindValue(0, QVariant::fromValue<library_song_id_t>(id));
    EXEC_SQL(
      "Error setting song sync status",
      deleteQuery.exec(),
      deleteQuery)
    if(progress != NULL){
      progress->setValue(i);
      if(progress->wasCanceled()){
        if(isTransacting){
          database.rollback();
        }
        break;
      }
    }
    ++i;
  }
  if(isTransacting){
    database.commit();
  }
}


void DataStore::addSongToActivePlaylist(library_song_id_t libraryId){
  QSet<library_song_id_t> libIds;
  libIds.insert(libraryId);
  addSongsToActivePlaylist(libIds);
}

void DataStore::addSongsToActivePlaylist(const QSet<library_song_id_t>& libIds){
  playlistIdsToAdd.unite(libIds);
  QSet<library_song_id_t> emptySet;
  serverConnection->modActivePlaylist(libIds, emptySet);
}

void DataStore::removeSongsFromActivePlaylist(const QSet<library_song_id_t>& libIds){
  playlistIdsToRemove.unite(libIds);
  QSet<library_song_id_t> emptySet;
  serverConnection->modActivePlaylist(emptySet, libIds);
}

QSqlDatabase DataStore::getDatabaseConnection(){
  QSqlDatabase toReturn = QSqlDatabase::database(getPlayerDBConnectionName());
  return toReturn;
}

Phonon::MediaSource DataStore::getNextSongToPlay(){
  QSqlQuery nextSongQuery("SELECT " + getLibFileColName() + " FROM " +
    getActivePlaylistViewName() + " LIMIT 1;");
  EXEC_SQL(
    "Getting next song failed",
    nextSongQuery.exec(),
    nextSongQuery)
  //TODO handle is this returns false
  if(nextSongQuery.first()){
    return Phonon::MediaSource(nextSongQuery.value(0).toString());
  }
  else{
    return Phonon::MediaSource("");
  }
}

DataStore::song_info_t DataStore::takeNextSongToPlay(){
  QSqlQuery nextSongQuery(
    "SELECT " + getLibFileColName() + ", " + 
    getLibSongColName() + ", " +
    getLibArtistColName() + ", " +
    getLibDurationColName() + ", " +
    getActivePlaylistLibIdColName() +" FROM " +
    getActivePlaylistViewName() + " LIMIT 1;", 
    database);
  EXEC_SQL(
    "Getting next song in take failed",
    nextSongQuery.exec(),
    nextSongQuery)
  nextSongQuery.next();
  if(!nextSongQuery.isValid()){
    song_info_t toReturn = {Phonon::MediaSource(""), "", "", "" };
    return toReturn;
  }
  currentSongId =
    nextSongQuery.value(4).value<library_song_id_t>();

  deleteSongFromPlaylist(currentSongId);

  Logger::instance()->log("Setting current song with id: " + QString::number(currentSongId));
  serverConnection->setCurrentSong(currentSongId);

  QString filePath = nextSongQuery.value(0).toString();
  QTime qtime(0, nextSongQuery.value(3).toInt()/60, nextSongQuery.value(3).toInt()%60);
  song_info_t toReturn = {
    Phonon::MediaSource(filePath),
    nextSongQuery.value(1).toString(),
    nextSongQuery.value(2).toString(),
    qtime.toString("mm:ss")
  };

  return toReturn;

}

void DataStore::deleteSongFromPlaylist(library_song_id_t toDelete){
  QSqlQuery deleteSongQuery(
    "DELETE FROM " + getActivePlaylistTableName() +
    " WHERE " + 
    getActivePlaylistLibIdColName() + " = " + QString::number(toDelete) + ";", 
    database);
  EXEC_SQL(
    "Deleting song from playlist failed",
    deleteSongQuery.exec(),
    deleteSongQuery)
}

void DataStore::setCurrentSong(const library_song_id_t& songToPlay){
  QSqlQuery getSongQuery(
    "SELECT " + getLibFileColName() + ", " +
    getLibSongColName() + ", " +
    getLibArtistColName() + ", " +
    getLibDurationColName() + " FROM " +
    getActivePlaylistViewName() + " WHERE " + 
    getActivePlaylistLibIdColName() + " = " + QString::number(songToPlay) + ";", 
    database);
  EXEC_SQL(
    "Getting song for manual playlist set failed.",
    getSongQuery.exec(),
    getSongQuery)
  getSongQuery.next();
  if(getSongQuery.isValid()){
    Logger::instance()->log("Got file, for manual song set");
    QString filePath = getSongQuery.value(0).toString();
    currentSongId = songToPlay;
    serverConnection->setCurrentSong(songToPlay);
    Logger::instance()->log("Retrieved Artist " + getSongQuery.value(2).toString());
    QTime qtime(0, getSongQuery.value(3).toInt()/60, getSongQuery.value(3).toInt()%60);
    song_info_t toEmit = {
      Phonon::MediaSource(filePath),
      getSongQuery.value(1).toString(),
      getSongQuery.value(2).toString(),
      qtime.toString("mm:ss")
    };
    emit manualSongChange(toEmit);
  }
}

void DataStore::createNewPlayer(
  const QString& name,
  const QString& password)
{
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerNameSettingName(), name);
  settings.setValue(getPlayerPasswordSettingName(), password);
  serverConnection->createPlayer(name, password);
}

void DataStore::createNewPlayer(
  const QString& name,
  const QString& password,
  const QString& streetAddress,
  const QString& city,
  const QString& state,
  const QString& zipcode)
{
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerNameSettingName(), name);
  settings.setValue(getPlayerPasswordSettingName(), password);
  settings.setValue(getAddressSettingName(), streetAddress);
  settings.setValue(getCitySettingName(), city);
  settings.setValue(getStateSettingName(), state);
  settings.setValue(getZipCodeSettingName(), zipcode);
  serverConnection->createPlayer(
    name,
    password,
    streetAddress,
    city,
    state,
    zipcode);
}

void DataStore::setVolume(qreal newVolume){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  Logger::instance()->log("Current volume " + QString::number(settings.value(getPlayerVolumeSettingName()).toReal()));
  Logger::instance()->log("New volume " + QString::number(newVolume));
  if((int)(settings.value(getPlayerVolumeSettingName()).toReal()*10) != (int)(newVolume*10)){
    Logger::instance()->log("Volume was different than current volume, now setting");
    settings.setValue(getPlayerVolumeSettingName(), newVolume);
    serverConnection->setVolume((int)(newVolume * 10));
  }
}






void DataStore::syncLibrary(){
  QSqlQuery needAddSongs(database);
  Logger::instance()->log("batching up sync");
  EXEC_SQL(
    "Error querying for song to add",
    needAddSongs.exec(
      "SELECT * FROM " + getLibraryTableName() + " WHERE " + 
      getLibSyncStatusColName() + "==" + 
      QString::number(getLibNeedsAddSyncStatus()) + " LIMIT 100;"),
    needAddSongs)

  QVariantList songsToAdd;
  QSqlRecord currentRecord;
  while(needAddSongs.next()){
    currentRecord = needAddSongs.record();
    QVariantMap songToAdd;
    songToAdd["id"] = currentRecord.value(getLibIdColName()).toString();
    QString title = currentRecord.value(getLibSongColName()).toString();
    title.truncate(199);
    songToAdd["title"] = title;
    QString artist = currentRecord.value(getLibArtistColName()).toString();
    artist.truncate(199);
    songToAdd["artist"] = artist;
    QString album = currentRecord.value(getLibAlbumColName()).toString();
    album.truncate(199);
    songToAdd["album"] = album; 
    songToAdd["duration"] = currentRecord.value(getLibDurationColName());
    songToAdd["track"] = currentRecord.value(getLibTrackColName()).toInt();
    QString genre = currentRecord.value(getLibGenreColName()).toString();
    genre.truncate(49);
    songToAdd["genre"] = genre;
    songsToAdd.append(songToAdd);
  }

  QSqlQuery needDeleteSongs(database);
  EXEC_SQL(
    "Error querying for songs to delete",
    needDeleteSongs.exec(
      "SELECT * FROM " + getLibraryTableName() + " WHERE " + 
      getLibSyncStatusColName() + "==" + 
      QString::number(getLibNeedsDeleteSyncStatus()) + " LIMIT 100;"),
    needDeleteSongs)

  QVariantList songsToDelete;
  while(needDeleteSongs.next()){
    currentRecord = needDeleteSongs.record();
    songsToDelete.append(currentRecord.value(getLibIdColName()).toString());
  }

  Logger::instance()->log("Found " + QString::number(songsToDelete.size()) + " songs which need deleting");
  Logger::instance()->log("Found " + QString::number(songsToAdd.size()) + " songs which need adding");
  if(songsToDelete.size() > 0 || songsToAdd.size() > 0){
    serverConnection->modLibContents(songsToAdd, songsToDelete);
  }
}

void DataStore::setLibSongSynced(library_song_id_t song){
  QSet<library_song_id_t> songSet;
  songSet.insert(song);
  setLibSongsSynced(songSet);
}

void DataStore::setLibSongsSynced(const QSet<library_song_id_t>& songs){
  setLibSongsSyncStatus(songs, getLibIsSyncedStatus());
}

void DataStore::setLibSongsSyncStatus(
  const QSet<library_song_id_t>& songs,
  const lib_sync_status_t syncStatus)
{
  Logger::instance()->log("Setting songs to synced");
  bool isTransacting = database.transaction();
  QSqlQuery syncQuery(database);
  syncQuery.prepare("UPDATE " + getLibraryTableName() +  " "
    "SET " + getLibSyncStatusColName() + "=" + 
      QString::number(syncStatus) + " "
    "WHERE " + getLibIdColName() + "= ?"); 
  Q_FOREACH(library_song_id_t id, songs){
    syncQuery.bindValue(0, QVariant::fromValue<library_song_id_t>(id));
    EXEC_SQL(
      "Error setting song sync status",
      syncQuery.exec(),
      syncQuery)
    QSet<library_song_id_t> modId;
    modId.insert(id);
    emit libSongsModified(modId);
  }
  if(isTransacting){
    database.commit();
  }

  if(hasUnsyncedSongs()){
    Logger::instance()->log("more stuff to sync");
    syncLibrary();
  }
  else{
    emit allSynced();
    Logger::instance()->log("syncing done");
  }
}

bool DataStore::hasUnsyncedSongs() const{
  return getTotalUnsynced() != 0;
}

int DataStore::getTotalUnsynced() const{
  QSqlQuery unsyncedQuery(database);
  EXEC_SQL(
    "Error querying for unsynced songs",
    unsyncedQuery.exec(
      "SELECT COUNT(*) FROM " + getLibraryTableName() + " WHERE " + 
      getLibSyncStatusColName() + "!=" + 
      QString::number(getLibIsSyncedStatus()) + ";"),
    unsyncedQuery)
  if(unsyncedQuery.next()){
    return unsyncedQuery.record().value(0).toInt();
  }
  else{
    return 0;
  }
}


void DataStore::clearActivePlaylist(){
  QSqlQuery deleteActivePlayilstQuery(database);
  EXEC_SQL(
    "Error clearing active playlist table.",
    deleteActivePlayilstQuery.exec(getClearActivePlaylistQuery()),
    deleteActivePlayilstQuery)
}

void DataStore::addSong2ActivePlaylistFromQVariant(
  const QVariantMap &songToAdd, int priority)
{
  QSqlQuery addQuery(
    "INSERT INTO "+getActivePlaylistTableName()+ 
    "("+
    getActivePlaylistLibIdColName() + ","+
    getDownVoteColName() + ","+
    getUpVoteColName() + "," +
    getPriorityColName() + "," +
    getTimeAddedColName() +"," +
    getAdderUsernameColName() +"," +
    getAdderIdColName() + ")" +
    " VALUES ( :libid , :down , :up, :pri , :time , :username, :adder );",
    database);

  addQuery.bindValue(":libid", songToAdd["song"].toMap()["id"]);
  addQuery.bindValue(":down", songToAdd["downvoters"].toList().size());
  addQuery.bindValue(":up", songToAdd["upvoters"].toList().size());
  addQuery.bindValue(":pri", priority);
  addQuery.bindValue(":time", songToAdd["time_added"]);
  addQuery.bindValue(":username", songToAdd["adder"].toMap()["username"]);
  addQuery.bindValue(":adder", songToAdd["adder"].toMap()["id"]);

  long insertId;
  EXEC_INSERT(
    "Failed to add song library" << songToAdd["song"].toString().toStdString(),
    addQuery,
    insertId,
    long)
}

void DataStore::setActivePlaylist(const QVariantMap& newPlaylist){

  int retrievedVolume = newPlaylist["volume"].toInt();
  if(retrievedVolume != (int)(getPlayerVolume()*10)){
    QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    settings.setValue(getPlayerVolumeSettingName(), retrievedVolume/10.0);
    emit volumeChanged(retrievedVolume/10.0);
  }

  QString retrievedState = newPlaylist["state"].toString();
  if(!changingPlayerState && retrievedState != getPlayerState()){
    QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    settings.setValue(getPlayerStateSettingName(), retrievedState);
    emit playerStateChanged(retrievedState);
  }


  library_song_id_t retrievedCurrentId =
    newPlaylist["current_song"].toMap()["song"].toMap()["id"].value<library_song_id_t>();
  if(retrievedCurrentId != currentSongId && !clearingCurrentSong){
    QSqlQuery getSongQuery(
      "SELECT " + getLibFileColName() + ", " +
      getLibSongColName() + ", " +
      getLibArtistColName() + ", " + 
      getLibDurationColName() + " FROM " +
      getActivePlaylistViewName() + " WHERE " + 
      getActivePlaylistLibIdColName() + " = " + QString::number(retrievedCurrentId) + ";", 
      database);
    EXEC_SQL(
      "Getting song for manual playlist set failed.",
      getSongQuery.exec(),
      getSongQuery)
    getSongQuery.next();
    if(getSongQuery.isValid()){
      Logger::instance()->log("Got file, for manual song set");
      QString filePath = getSongQuery.value(0).toString();
      currentSongId = retrievedCurrentId;
      QTime qtime(0, getSongQuery.value(3).toInt()/60, getSongQuery.value(3).toInt()%60);
      song_info_t toEmit = {
        Phonon::MediaSource(filePath),
        getSongQuery.value(1).toString(),
        getSongQuery.value(2).toString(),
	qtime.toString("mm:ss")
      };
      emit manualSongChange(toEmit);
    }
  }
  clearActivePlaylist();
  QVariantList newSongs = newPlaylist["active_playlist"].toList();
  for(int i=0; i<newSongs.size(); ++i){
    addSong2ActivePlaylistFromQVariant(newSongs[i].toMap(), i); 
  }
  emit activePlaylistModified();
  
}

void DataStore::onGetActivePlaylistFail(
  const QString& errMessage,
  int errorCode,
  const QList<QNetworkReply::RawHeaderPair>& headers)
{
  Logger::instance()->log("Playlist error: " + QString::number(errorCode) + " " + errMessage);
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(GET_ACTIVE_PLAYLIST);
    initReauth();
  }
  //TODO handle other possible errors?
}

void DataStore::onActivePlaylistModified(
  const QSet<library_song_id_t>& added,
  const QSet<library_song_id_t>& removed)
{
  playlistIdsToAdd.subtract(added);
  playlistIdsToRemove.subtract(removed);
  refreshActivePlaylist();
}

void DataStore::onActivePlaylistModFailed(
  const QString& /*errMessage*/,
  int errorCode,
  const QList<QNetworkReply::RawHeaderPair>& headers)
{
  Logger::instance()->log("Active playlist mod failed with code " + QString::number(errorCode));
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(MOD_PLAYLIST);
    initReauth();
  }
  //TODO do stuff on failure
}

void DataStore::refreshActivePlaylist(){
  serverConnection->getActivePlaylist();
}

void DataStore::refreshParticipantList(){
  serverConnection->getParticipantList();
}


void DataStore::onPlayerCreate(const player_id_t& issuedId){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerIdSettingName(), QVariant::fromValue(issuedId));
  serverConnection->setPlayerId(issuedId);
  emit playerCreated();
}

void DataStore::onPlayerCreationFailed(const QString& errMessage, int /*errorCode*/,
    const QList<QNetworkReply::RawHeaderPair>& /*headers*/)
{
  //TODO do other stuff as well. like do reauth
  emit playerCreationFailed(errMessage);
}

void DataStore::onLibModError(
    const QString& errMessage, int errorCode, const QList<QNetworkReply::RawHeaderPair>& headers)
{
  Logger::instance()->log("Got bad libmod " + QString::number(errorCode));
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(SYNC_LIB);
    initReauth();
  }
  else{
    Logger::instance()->log("Bad lib mod message " + errMessage);
    emit libModError(errMessage);
  }
}

void DataStore::onSetCurrentSongFailed(
  const QString& errMessage, int errorCode, const QList<QNetworkReply::RawHeaderPair>& headers)
{
  Logger::instance()->log("Setting current song failed: " + QString::number(errorCode) + " " + errMessage);
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(SET_CURRENT_SONG);
    initReauth();
  }
}

void DataStore::onSetVolumeFailed(
  const QString& errMessage, int errorCode, const QList<QNetworkReply::RawHeaderPair>& headers)
{
  Logger::instance()->log("Setting volume failed " + 
    QString::number(errorCode) + " " + errMessage);
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(SET_CURRENT_VOLUME);
    initReauth();
  }
  else{
    emit setVolumeError(errMessage);
  }
}

void DataStore::onNewParticipantList(const QVariantList& newParticipants){
  emit newParticipantList(newParticipants);
}





void DataStore::onReauth(const QByteArray& ticketHash, const user_id_t& userId){
  Logger::instance()->log("in on reauth");
  isReauthing=false;
  serverConnection->setTicket(ticketHash);
  serverConnection->setUserId(userId);

  Q_FOREACH(ReauthAction r, reauthActions){
    doReauthAction(r);
  }

  reauthActions.clear();
}

void DataStore::doReauthAction(const ReauthAction& action){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  switch(action){
    case SYNC_LIB:
      syncLibrary();
      break;
    case GET_ACTIVE_PLAYLIST:
      refreshActivePlaylist();
      break;
    case SET_CURRENT_SONG:
      if(currentSongId != -1){
        serverConnection->setCurrentSong(currentSongId);
      }
      break;
    case MOD_PLAYLIST:
      serverConnection->modActivePlaylist(playlistIdsToAdd, playlistIdsToRemove);
      break;
    case SET_CURRENT_VOLUME:
      serverConnection->setVolume((int)(getPlayerVolume() * 10));
      break;
    case SET_PLAYER_STATE:
      serverConnection->setPlayerState(getPlayerState());
      break;
    case SET_PLAYER_INACTIVE:
      serverConnection->setPlayerState(getInactiveState());
      break;
    case SET_PLAYER_LOCATION:
      serverConnection->setPlayerLocation(
        settings.value(getAddressSettingName()).toString(),
        settings.value(getCitySettingName()).toString(),
        settings.value(getStateSettingName()).toString(),
        settings.value(getZipCodeSettingName()).toString());
      break;
    case SET_PLAYER_PASSWORD:
      serverConnection->setPlayerPassword(
        settings.value(getPlayerPasswordSettingName()).toString());
      break;
    case REMOVE_PLAYER_PASSWORD:
      serverConnection->removePlayerPassword();
      break;
    case CLEAR_CURRENT_SONG:
      serverConnection->clearCurrentSong();
      break;
  }
}

void DataStore::onAuthFail(const QString& /*errMessage*/){
  Logger::instance()->log("BAD STUFF, BAD AUTH CREDS, BAD REAUTH");
  setPasswordDirty();
  emit hardAuthFailure();
}

void DataStore::initReauth(){
  if(!isReauthing){
    isReauthing=true;
    serverConnection->authenticate(getUsername(), getPassword());
  }
}

QByteArray DataStore::getHeaderValue(
    const QByteArray& headerName,
    const QList<QNetworkReply::RawHeaderPair>& headers)
{
  //Yes yes, I know this is an O(n) search. But it's fine. 
  //This list of headers shouldn't be that long.
  Q_FOREACH(const QNetworkReply::RawHeaderPair& pair, headers){
    if(headerName == pair.first){
      return pair.second;
    }
  }
  return "";
}


bool DataStore::getDontShowPlaybackErrorSetting(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  return settings.value(getDontShowPlaybackErrorSettingName(), false ).toBool();
}

void DataStore::setDontShowPlaybackError(bool checked){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getDontShowPlaybackErrorSettingName(), checked);
}

void DataStore::saveUsername(const QString& username){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  SimpleCrypt crypt = Utils::getCryptoObject();
  QString cryptUsername = crypt.encryptToString(username);
  settings.setValue(getUsernameSettingName(), cryptUsername);
}

QString DataStore::getSavedUsername(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  SimpleCrypt crypt = Utils::getCryptoObject();
  QString encryptedUsername = settings.value(getUsernameSettingName()).toString();
  return encryptedUsername == "" ? "" : crypt.decryptToString(encryptedUsername);
}



void DataStore::savePassword(const QString& password)
{
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  SimpleCrypt crypt = Utils::getCryptoObject();
  QString cryptPassword = crypt.encryptToString(password);
  settings.setValue(getHasValidSavedPasswordSettingName(), true);
  settings.setValue(getPasswordSettingName(), cryptPassword);
}

void DataStore::setPasswordDirty(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getHasValidSavedPasswordSettingName(), false);
}

bool DataStore::hasValidSavedPassword(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  return settings.value(getHasValidSavedPasswordSettingName()).toBool();
}

QString DataStore::getSavedPassword(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  SimpleCrypt crypt = Utils::getCryptoObject();
  QString encryptedPassword = settings.value(getPasswordSettingName()).toString();
  return crypt.decryptToString(encryptedPassword);
}

void DataStore::clearSavedPassword(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getHasValidSavedPasswordSettingName(), false);
  settings.setValue(getPasswordSettingName(), "");
}



} //end namespace
