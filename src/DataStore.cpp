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
#include "CommErrorHandler.hpp"
#include "simpleCrypt/simplecrypt.h"
#include <QDir>
#include <QDesktopServices>
#include <QDir>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlRecord>
#include <QThread>
#include <QTimer>
#include <QDateTime>
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
  isReauthing(false)
{
  serverConnection = new UDJServerConnection(this);
  serverConnection->setTicket(ticket);
  serverConnection->setUserId(userId);
  activePlaylistRefreshTimer = new QTimer(this);
  activePlaylistRefreshTimer->setInterval(5000);
  setupDB();

  connect(
    serverConnection,
    SIGNAL(libSongsSyncedToServer(const std::vector<library_song_id_t>&)),
    this,
    SLOT(setLibSongsSynced(const std::vector<library_song_id_t>&)));

  connect(
    serverConnection,
    SIGNAL(playerCreated(const player_id_t&)),
    this,
    SLOT(onPlayerCreate(const player_id_t&)));

  connect(
    serverConnection,
    SIGNAL(playerCreationFailed(const QString)),
    this,
    SLOT(onPlayerCreateFail(const QString)));

  connect(
    serverConnection,
    SIGNAL(newActivePlaylist(const QVariantList)),
    this,
    SLOT(setActivePlaylist(const QVariantList)));

  connect(
    serverConnection,
    SIGNAL(songsAddedToActivePlaylist(const std::vector<client_request_id_t>)),
    this,
    SLOT(setPlaylistAddRequestsSynced(const std::vector<client_request_id_t>)));

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
    SIGNAL(songRemovedFromActivePlaylist(const playlist_song_id_t)),
    this,
    SLOT(setPlaylistRemoveRequestSynced(const playlist_song_id_t)));

  connect(
    serverConnection,
    SIGNAL(playerSetActive()),
    this,
    SLOT(onPlayerSetActive()));

  connect(
    serverConnection,
    SIGNAL(playerSetInactive()),
    this,
    SLOT(onPlayerDeactivated()));

  connect(
    serverConnection,
    SIGNAL(libModError(const QString&)),
    this,
    SLOT(onLibModError(const QString&)));

  connect(
    serverConnection,
    SIGNAL(authenticated(const QByteArray& ticketHash, const user_id_t& userId)),
    this,
    SLOT(onReauth(const QByteArray& ticketHash, const user_id_t& userId)));

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

void DataStore::activatePlayer(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  if(settings.contains(getPlayerIdSettingName())){
    serverConnection->setPlayerId(getPlayerId());
    serverConnection->setPlayerActive();
    //activePlaylistRefreshTimer->start();
  }
  else{
    emit needPlayerCreate();
  }
}

void DataStore::deactivatePlayer(){
  serverConnection->setPlayerInactive();
}

void DataStore::addMusicToLibrary(
  QList<Phonon::MediaSource> songs, QProgressDialog* progress)
{
  for(int i =0; i<songs.size(); ++i){
    progress->setValue(i);
    if(progress->wasCanceled()){
      break;
    }
    addSongToLibrary(songs[i]);
  }
  syncLibrary();
}

void DataStore::addSongToLibrary(Phonon::MediaSource song){
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
  QSqlQuery addQuery(
    "INSERT INTO "+getLibraryTableName()+ 
    "("+
    getLibSongColName() + ","+
    getLibArtistColName() + ","+
    getLibAlbumColName() + ","+
    getLibGenreColName() + "," +
    getLibTrackColName() + "," +
    getLibFileColName() + "," +
    getLibDurationColName() +")" +
    "VALUES ( :song , :artist , :album , :genre, :track, :file, :duration );", 
    database);

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

void DataStore::removeSongsFromLibrary(std::vector<library_song_id_t> toRemove){
  QVariantList toDelete;
  for(
    std::vector<library_song_id_t>::const_iterator it= toRemove.begin();
    it!=toRemove.end();
    ++it)
  {
    toDelete << QVariant::fromValue<library_song_id_t>(*it);
  }
  QSqlQuery bulkDelete(database);
  bulkDelete.prepare("UPDATE " + getLibraryTableName() +  " "
    "SET " + getLibIsDeletedColName() + "=1, "+
    getLibSyncStatusColName() + "=" + 
      QString::number(getLibNeedsDeleteSyncStatus()) + " "
    "WHERE " + getLibIdColName() + "= ?"); 
  bulkDelete.addBindValue(toDelete);
  EXEC_BULK_QUERY("Error removing songs from library", 
    bulkDelete)
  if(bulkDelete.lastError().type() == QSqlError::NoError){
    syncLibrary();
  }
}

/*
void DataStore::addSongToActivePlaylist(library_song_id_t libraryId){
  serverConnection->addSongToActivePlaylist(libraryId);
  std::vector<library_song_id_t> toAdd(1, libraryId);
  addSongsToActivePlaylist(toAdd);
}*/

void DataStore::addSongsToActivePlaylist(const std::vector<library_song_id_t>& libIds){
  /*
  QVariantList toInsert;
  for(
    std::vector<library_song_id_t>::const_iterator it= libIds.begin();
    it!=libIds.end();
    ++it)
  {
    toInsert << QVariant::fromValue<library_song_id_t>(*it);
  }
  QSqlQuery bulkInsert(database);
  bulkInsert.prepare("INSERT INTO " + getPlaylistAddRequestsTableName() + 
    "(" + getPlaylistAddLibIdColName() + ") VALUES( ? );");
  bulkInsert.addBindValue(toInsert);
  EXEC_BULK_QUERY("Error inserting songs into add queue for active playlist", 
    bulkInsert)
  syncPlaylistAddRequests();*/
}

/*
void DataStore::removeSongFromActivePlaylist(playlist_song_id_t plId){
  std::vector<library_song_id_t> toRemove(1, plId);
  removeSongsFromActivePlaylist(toRemove);
}*/

void DataStore::removeSongsFromActivePlaylist(const std::vector<library_song_id_t>& libIds){

}

QSqlDatabase DataStore::getDatabaseConnection(){
  return database;
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

Phonon::MediaSource DataStore::takeNextSongToPlay(){
  QSqlQuery nextSongQuery(
    "SELECT " + getLibFileColName() + ", " + 
    getActivePlaylistIdColName() +" FROM " +
    getActivePlaylistViewName() + " LIMIT 1;", 
    database);
  EXEC_SQL(
    "Getting next song in take failed",
    nextSongQuery.exec(),
    nextSongQuery)
  nextSongQuery.next();
  if(!nextSongQuery.isValid()){
    return Phonon::MediaSource("");
  }
  QString filePath = nextSongQuery.value(0).toString();
  playlist_song_id_t  currentSongId  = 
    nextSongQuery.value(1).value<playlist_song_id_t>();
  
  serverConnection->setCurrentSong(currentSongId);
  return Phonon::MediaSource(filePath);
}

void DataStore::setCurrentSong(const playlist_song_id_t& songToPlay){
  QSqlQuery getSongQuery(
    "SELECT " + getLibFileColName() + "  FROM " +
    getActivePlaylistViewName() + " WHERE " + 
    getActivePlaylistIdColName() + " = " + QString::number(songToPlay) + ";", 
    database);
  EXEC_SQL(
    "Getting song for manual playlist set failed.",
    getSongQuery.exec(),
    getSongQuery)
  getSongQuery.next();
  if(getSongQuery.isValid()){
    QString filePath = getSongQuery.value(0).toString();
    emit manualSongChange(Phonon::MediaSource(filePath));
    serverConnection->setCurrentSong(songToPlay);
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
  serverConnection->createPlayer(
    name,
    password,
    streetAddress,
    city,
    state,
    zipcode);
}

void DataStore::syncLibrary(){

  QSqlQuery needAddSongs(database);
  EXEC_SQL(
    "Error querying for song to add",
    needAddSongs.exec(
      "SELECT * FROM " + getLibraryTableName() + " WHERE " + 
      getLibSyncStatusColName() + "==" + 
      QString::number(getLibNeedsAddSyncStatus()) + ";"),
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
      QString::number(getLibNeedsDeleteSyncStatus()) + ";"),
    needDeleteSongs)

  QVariantList songsToDelete;
  while(needDeleteSongs.next()){
    currentRecord = needDeleteSongs.record();
    songsToDelete.append(currentRecord.value(getLibIdColName()));
  }

  if(songsToDelete.size() > 0 || songsToAdd.size() > 0){
    serverConnection->modLibContents(songsToAdd, songsToDelete);
  }
}

void DataStore::setLibSongSynced(library_song_id_t song){
  std::vector<library_song_id_t> songVector(1,song);
  setLibSongsSynced(songVector);
}

void DataStore::setLibSongsSynced(const std::vector<library_song_id_t> songs){
  setLibSongsSyncStatus(songs, getLibIsSyncedStatus());
}

void DataStore::setLibSongsSyncStatus(
  const std::vector<library_song_id_t> songs,
  const lib_sync_status_t syncStatus)
{
  QSqlQuery setSyncedQuery(database);
  for(int i=0; i< songs.size(); ++i){
    EXEC_SQL(
      "Error setting song to synced",
      setSyncedQuery.exec(
        "UPDATE " + getLibraryTableName() + " " +
        "SET " + getLibSyncStatusColName() + "=" + QString::number(syncStatus) +
        " WHERE "  +
        getLibIdColName() + "=" + QString::number(songs[i]) + ";"),
      setSyncedQuery)
  }
  emit libSongsModified();
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
    getActivePlaylistIdColName() + ","+
    getActivePlaylistLibIdColName() + ","+
    getDownVoteColName() + ","+
    getUpVoteColName() + "," +
    getPriorityColName() + "," +
    getTimeAddedColName() +"," +
    getAdderUsernameColName() +"," +
    getAdderIdColName() + ")" +
    " VALUES ( :id , :libid , :down , :up, :pri , :time , :username, :adder );",
    database);

  addQuery.bindValue(":id", songToAdd["id"]);
  addQuery.bindValue(":libid", songToAdd["lib_song_id"]);
  addQuery.bindValue(":down", songToAdd["down_votes"]);
  addQuery.bindValue(":up", songToAdd["up_votes"]);
  addQuery.bindValue(":pri", priority);
  addQuery.bindValue(":time", songToAdd["time_added"]);
  addQuery.bindValue(":username", songToAdd["adder_username"]);
  addQuery.bindValue(":adder", songToAdd["adder_id"]);

  long insertId;
  EXEC_INSERT(
    "Failed to add song library" << songToAdd["song"].toString().toStdString(),
    addQuery,
    insertId,
    long)
}

void DataStore::setActivePlaylist(const QVariantList newSongs){
  clearActivePlaylist();
  for(int i=0; i<newSongs.size(); ++i){
    addSong2ActivePlaylistFromQVariant(newSongs[i].toMap(), i); 
  }
  emit activePlaylistModified();
}

void DataStore::refreshActivePlaylist(){
  serverConnection->getActivePlaylist();
}


void DataStore::onPlayerCreate(const player_id_t& issuedId){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerIdSettingName(), QVariant::fromValue(issuedId));
  settings.setValue(getPlayerStateSettingName(), getPlayerActiveState());
  activePlaylistRefreshTimer->start();
  serverConnection->setPlayerId(issuedId);
  emit playerCreated();
}

void DataStore::onPlayerCreateFail(const QString message){
  emit playerCreationFailed(message);
}


void DataStore::saveCredentials(
  const QString& username, const QString& password)
{
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  SimpleCrypt crypt = UDJ_GET_CRYPTO_OBJECT;
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
  SimpleCrypt crypt = UDJ_GET_CRYPTO_OBJECT;
  QString encryptedUsername = 
    settings.value(getUsernameSettingName()).toString();
  QString encryptedPassword = 
    settings.value(getPasswordSettingName()).toString();
  *username = crypt.decryptToString(encryptedUsername);
  *password = crypt.decryptToString(encryptedPassword);
}

void DataStore::clearSavedCredentials(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getHasValidCredsSettingName(), false);
  settings.setValue(getUsernameSettingName(), "");
  settings.setValue(getPasswordSettingName(), "");
}

void DataStore::pausePlaylistUpdates(){
  if(activePlaylistRefreshTimer->isActive()){
    activePlaylistRefreshTimer->stop();
  }
}

void DataStore::resumePlaylistUpdates(){
  if(!activePlaylistRefreshTimer->isActive()){
    activePlaylistRefreshTimer->start();
  }
}

void DataStore::onPlayerSetActive(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerStateSettingName(), getPlayerActiveState());
  syncLibrary();
  emit playerActive();
}

void DataStore::onPlayerDeactivated(){
  QSettings settings(QSettings::UserScope, getSettingsOrg(), getSettingsApp());
  settings.setValue(getPlayerStateSettingName(), getPlayerInactiveState());
  emit playerDeactivated();
}

void DataStore::onLibModError(const QString& errMessage, int errorCode){
  if(errorCode == 401){
    reauthFunctions.insert(SYNC_LIB);
    initReauth();
  }
}





void DataStore::onReauth(const QByteArray& ticketHash, const user_id_t& userId){
  isReauthing=false;
  serverConnection->setTicket(ticketHash);
  serverConnection->setUserId(userId);

  Q_FOREACH(ReauthFunction r, reauthFunctions){
    doReauthFunction(r);
  }

  reauthFunctions.clear();
}

void DataStore::doReauthFunction(const ReauthFunction& functionType){
  switch(functionType){
  case SYNC_LIB:
    syncLibrary();
    break;
  }
}

void DataStore::onAuthFail(const QString& errMessage){
  isReauthing=false;
  DEBUG_MESSAGE("BAD STUFF, BAD AUTH CREDS");
  //TODO need to do something here
}

void DataStore::initReauth(){
  if(!isReauthing){
    isReauthing=true;
    serverConnection->authenticate(getUsername(), getPassword());
  }
}


} //end namespace
