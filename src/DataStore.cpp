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
  setupDB();

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
    serverConnection,
    SIGNAL(playerStateSet(const QString&)),
    this,
    SLOT(onPlayerStateChanged(const QString&)));

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

}

void DataStore::setupDB(){
  //TODO do all of this stuff in a seperate thread and return right away.
  QDir dbDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));  
  if(!dbDir.exists()){
    //TODO handle if this fails
    dbDir.mkpath(dbDir.absolutePath());
  }
  database = QSqlDatabase::addDatabase("QSQLITE", getPlayerDBConnectionName());
  database.setDatabaseName(dbDir.absoluteFilePath(getPlayerDBName()));
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

void DataStore::pausePlayer(){
  setPlayerState(getPausedState());
}

void DataStore::playPlayer(){
  setPlayerState(getPlayingState());
}

void DataStore::setPlayerState(const QString& newState){
  serverConnection->setPlayerState(newState);
}


void DataStore::addMusicToLibrary(
  const QList<Phonon::MediaSource>& songs, QProgressDialog* progress)
{
  bool isTransacting=database.transaction();
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
          database.rollback();
        }
        break;
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
    int track = tag->track();
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
    getActivePlaylistLibIdColName() +" FROM " +
    getActivePlaylistViewName() + " LIMIT 1;", 
    database);
  EXEC_SQL(
    "Getting next song in take failed",
    nextSongQuery.exec(),
    nextSongQuery)
  nextSongQuery.next();
  if(!nextSongQuery.isValid()){
    currentSongId = -1;
    song_info_t toReturn = {Phonon::MediaSource(""), "", "" };
    return toReturn;
  }
  currentSongId =
    nextSongQuery.value(3).value<library_song_id_t>();

  Logger::instance()->log("Setting current song with id: " + QString::number(currentSongId).toStdString());
  serverConnection->setCurrentSong(currentSongId);

  QString filePath = nextSongQuery.value(0).toString();
  song_info_t toReturn = {
    Phonon::MediaSource(filePath),
    nextSongQuery.value(1).toString(),
    nextSongQuery.value(2).toString()
  };

  return toReturn;

}

void DataStore::setCurrentSong(const library_song_id_t& songToPlay){
  QSqlQuery getSongQuery(
    "SELECT " + getLibFileColName() + ", " +
    getLibSongColName() + ", " +
    getLibArtistColName() + " FROM " +
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
    Logger::instance()->log("Retrieved Artist " + getSongQuery.value(2).toString().toStdString());
    song_info_t toEmit = {
      Phonon::MediaSource(filePath),
      getSongQuery.value(1).toString(),
      getSongQuery.value(2).toString()
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
  serverConnection->createPlayer(name, password);
}

void DataStore::createNewPlayer(
  const QString& name,
  const QString& password,
  const QString& streetAddress,
  const QString& city,
  const QString& state,
  const int& zipcode)
{
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerNameSettingName(), name);
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

void DataStore::changeVolumeSilently(qreal newVolume){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  Logger::instance()->log("Current volume " + QString::number(settings.value(getPlayerVolumeSettingName()).toReal()).toStdString());
  Logger::instance()->log("New volume " + QString::number(newVolume).toStdString());
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
    songToAdd["id"] = currentRecord.value(getLibIdColName());
    songToAdd["title"] = currentRecord.value(getLibSongColName());
    songToAdd["artist"] = currentRecord.value(getLibArtistColName());
    songToAdd["album"] = currentRecord.value(getLibAlbumColName());
    songToAdd["duration"] = currentRecord.value(getLibDurationColName());
    songToAdd["track"] = currentRecord.value(getLibTrackColName()).toInt();
    songToAdd["genre"] = currentRecord.value(getLibGenreColName()).toString();
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
    songsToDelete.append(currentRecord.value(getLibIdColName()));
  }

  Logger::instance()->log("Found " + QString::number(songsToDelete.size()).toStdString() + " songs which need deleting");
  Logger::instance()->log("Found " + QString::number(songsToAdd.size()).toStdString() + " songs which need adding");
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

  onPlayerStateChanged(newPlaylist["state"].toString());

  library_song_id_t retrievedCurrentId =
    newPlaylist["current_song"].toMap()["song"].toMap()["id"].value<library_song_id_t>();
  if(retrievedCurrentId != currentSongId){
    QSqlQuery getSongQuery(
      "SELECT " + getLibFileColName() + ", " +
      getLibSongColName() + ", " +
      getLibArtistColName() + " FROM " +
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
      song_info_t toEmit = {
        Phonon::MediaSource(filePath),
        getSongQuery.value(1).toString(),
        getSongQuery.value(2).toString()
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
  Logger::instance()->log("Playlist error: " + QString::number(errorCode).toStdString() + " " + errMessage.toStdString());
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
  const QString& errMessage,
  int errorCode,
  const QList<QNetworkReply::RawHeaderPair>& headers)
{
  Logger::instance()->log("Active playlist mod failed with code " + QString::number(errorCode).toStdString());
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


void DataStore::onPlayerCreate(const player_id_t& issuedId){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerIdSettingName(), QVariant::fromValue(issuedId));
  serverConnection->setPlayerId(issuedId);
  setPlayerState(getPlayingState());
  emit playerCreated();
}

void DataStore::onPlayerCreationFailed(const QString& errMessage, int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers)
{
  //TODO do other stuff as well. like do reauth
  emit playerCreationFailed(errMessage);
}


void DataStore::onPlayerStateChanged(const QString& newState){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  if(newState != settings.value(getPlayerStateSettingName())){
    settings.setValue(getPlayerStateSettingName(), newState);
    emit playerStateChanged(newState);
  }

  //If this player state change is the result of starting up the player for the first time
  //we need to do a few things
  if(!activePlaylistRefreshTimer->isActive()){
    refreshActivePlaylist();
    activePlaylistRefreshTimer->start();
  }
}


void DataStore::onLibModError(
    const QString& errMessage, int errorCode, const QList<QNetworkReply::RawHeaderPair>& headers)
{
  Logger::instance()->log("Got bad libmod " + QString::number(errorCode).toStdString());
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(SYNC_LIB);
    initReauth();
  }
  else{
    Logger::instance()->log("Bad lib mod message " + errMessage.toStdString());
    emit libModError(errMessage);
  }
}

void DataStore::onSetCurrentSongFailed(
  const QString& errMessage, int errorCode, const QList<QNetworkReply::RawHeaderPair>& headers)
{
  Logger::instance()->log("Setting current song failed: " + QString::number(errorCode).toStdString() + " " + errMessage.toStdString());
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
    QString::number(errorCode).toStdString() + " " + errMessage.toStdString());
  if(isTicketAuthError(errorCode, headers)){
    Logger::instance()->log("Got the ticket-hash challenge");
    reauthActions.insert(SET_CURRENT_VOLUME);
    initReauth();
  }
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
      QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
      serverConnection->setVolume((int)(getPlayerVolume() * 10));
      break;
  }
}

void DataStore::onAuthFail(const QString& errMessage){
  isReauthing=false;
  Logger::instance()->log("BAD STUFF, BAD AUTH CREDS, BAD REAUTH");
  //TODO need to do something here
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




void DataStore::saveCredentials(
  const QString& username, const QString& password)
{
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  SimpleCrypt crypt = Utils::getCryptoObject();
  QString cryptUsername = crypt.encryptToString(username);
  QString cryptPassword = crypt.encryptToString(password);
  settings.setValue(getHasValidCredsSettingName(), true);
  settings.setValue(getUsernameSettingName(), cryptUsername);
  settings.setValue(getPasswordSettingName(), cryptPassword);
}

void DataStore::setCredentialsDirty(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getHasValidCredsSettingName(), false);
}

bool DataStore::hasValidSavedCredentials(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  return settings.value(getHasValidCredsSettingName()).toBool();
}

void DataStore::getSavedCredentials(QString* username, QString* password){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  SimpleCrypt crypt = Utils::getCryptoObject();
  QString encryptedUsername = settings.value(getUsernameSettingName()).toString();
  QString encryptedPassword = settings.value(getPasswordSettingName()).toString();
  *username = crypt.decryptToString(encryptedUsername);
  *password = crypt.decryptToString(encryptedPassword);
}

void DataStore::clearSavedCredentials(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getHasValidCredsSettingName(), false);
  settings.setValue(getUsernameSettingName(), "");
  settings.setValue(getPasswordSettingName(), "");
}



} //end namespace
