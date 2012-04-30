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
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QBuffer>
#include <QRegExp>
#include <QStringList>
#include "UDJServerConnection.hpp"
#include "JSONHelper.hpp"


namespace UDJ{


UDJServerConnection::UDJServerConnection(QObject *parent):QObject(parent),
  ticket_hash(""),
  user_id(-1),
  playerId(-1)
{
  netAccessManager = new QNetworkAccessManager(this);
  connect(netAccessManager, SIGNAL(finished(QNetworkReply*)),
    this, SLOT(recievedReply(QNetworkReply*)));
}

void UDJServerConnection::prepareJSONRequest(QNetworkRequest &request){
  request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
  request.setRawHeader(getTicketHeaderName(), ticket_hash);
}

void UDJServerConnection::authenticate(
  const QString& username,
  const QString& password)
{
  QNetworkRequest authRequest(getAuthUrl());
  QString data("username="+username+"&password="+password);
  QBuffer *dataBuffer = new QBuffer();
  dataBuffer->setData(data.toUtf8());
  QNetworkReply *reply = netAccessManager->post(authRequest, dataBuffer);
  dataBuffer->setParent(reply);
  DEBUG_MESSAGE("Doing auth request")
}

void UDJServerConnection::modLibContents(const QVariantList& songsToAdd, 
   const QVariantList& songsToDelete)
{
  DEBUG_MESSAGE("Modding lib contents")
  QNetworkRequest modRequest(getLibModUrl());
  modRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QByteArray addJSON = JSONHelper::getJSONForLibAdd(songsToAdd);
  QByteArray deleteJSON = JSONHelper::getJSONForLibDelete(songsToDelete);
  QUrl params;
  params.addQueryItem("to_add", addJSON);
  params.addQueryItem("to_delete", deleteJSON);
  QByteArray payload = params.encodedQuery();
  QNetworkReply *reply = netAccessManager->post(modRequest, payload);
  reply->setProperty(getSongsAddedPropertyName(), addJSON);
  reply->setProperty(getSongsDeletedPropertyName(), deleteJSON);
}

void UDJServerConnection::createPlayer(
  const QString& playerName,
  const QString& password)
{
  const QByteArray playerJSON = JSONHelper::getCreatePlayerJSON(playerName, password);
  createPlayer(playerJSON);
}

void UDJServerConnection::createPlayer(
  const QString& playerName,
  const QString& password,
  const QString& streetAddress,
  const QString& city,
  const QString& state,
  const int& zipcode)
{
  const QByteArray playerJSON = JSONHelper::getCreatePlayerJSON(playerName, password,
      streetAddress, city, state, zipcode);
  createPlayer(playerJSON);
}

void UDJServerConnection::createPlayer(const QByteArray& payload){
  QNetworkRequest createPlayerRequest(getCreatePlayerUrl());
  prepareJSONRequest(createPlayerRequest);
  QNetworkReply *reply = netAccessManager->put(createPlayerRequest, payload);
  reply->setProperty(getPayloadPropertyName(), payload);
}

void UDJServerConnection::getActivePlaylist(){
  QNetworkRequest getActivePlaylistRequest(getActivePlaylistUrl());
  getActivePlaylistRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  netAccessManager->get(getActivePlaylistRequest);
}

void UDJServerConnection::addSongToActivePlaylist(library_song_id_t libId){
  QNetworkRequest add2ActivePlaylistRequest(getActivePlaylistAddUrl(libId));
  add2ActivePlaylistRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QNetworkReply *reply = netAccessManager->put(add2ActivePlaylistRequest, "");
}

void UDJServerConnection::removeSongsFromActivePlaylist(
  const std::vector<playlist_song_id_t>& playlistIds)
{
  if(playlistIds.size() <= 0){
    return;
  }
  for(
    std::vector<playlist_song_id_t>::const_iterator it = playlistIds.begin();
    it != playlistIds.end();
    ++it)
  {
    QNetworkRequest removeSongFromActiveRequest(
      getActivePlaylistRemoveUrl(*it));
    removeSongFromActiveRequest.setRawHeader(
      getTicketHeaderName(), ticket_hash);
    netAccessManager->deleteResource(removeSongFromActiveRequest);
  }
}


void UDJServerConnection::setCurrentSong(playlist_song_id_t currentSong){
  QString params = "playlist_entry_id="+QString::number(currentSong);
  setCurrentSong(params.toUtf8());
}

void UDJServerConnection::setCurrentSong(const QByteArray& payload){
  QNetworkRequest setCurrentSongRequest(getCurrentSongUrl());
  setCurrentSongRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QNetworkReply *reply =
    netAccessManager->post(setCurrentSongRequest, payload);
  reply->setProperty(getPayloadPropertyName(), payload);
}

void UDJServerConnection::setPlayerActive(){
  DEBUG_MESSAGE("Setting player active")
  QString params("state=playing");
  QByteArray payload = params.toUtf8();
  QNetworkRequest setPlayerActiveRequest(getPlayerStateUrl());
  setPlayerActiveRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QNetworkReply *reply = netAccessManager->post(setPlayerActiveRequest, payload);
  reply->setProperty(getPayloadPropertyName(), payload);
}

void UDJServerConnection::setPlayerInactive(){
  QString params("state=inactive");
  QByteArray payload = params.toUtf8();
  QNetworkRequest setPlayerActiveRequest(getPlayerStateUrl());
  setPlayerActiveRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QNetworkReply *reply = netAccessManager->post(setPlayerActiveRequest, payload);
  reply->setProperty(getPayloadPropertyName(), payload);
}

void UDJServerConnection::recievedReply(QNetworkReply *reply){
  if(reply->request().url().path() == getAuthUrl().path()){
    handleAuthReply(reply);
  }
  else if(isSetActiveReply(reply)){
    handleSetActiveReply(reply);
  }
  else if(isSetInactiveReply(reply)){
    handleSetInactiveReply(reply);
  }
  else if(isPlayerCreateUrl(reply->request().url().path())){
    handleCreatePlayerReply(reply);
  }
  else if(reply->request().url().path() == getActivePlaylistUrl().path()){
    handleRecievedActivePlaylist(reply);
  }
  else if(isActivePlaylistAddUrl(reply->request().url().path())){
    handleRecievedActivePlaylistAdd(reply);
  }
  else if(reply->request().url().path() == getCurrentSongUrl().path()){
    handleRecievedCurrentSongSet(reply);
  }
  else if(isActivePlaylistRemoveUrl(reply->request().url().path())){
    handleRecievedActivePlaylistRemove(reply);
  }
  else if(reply->request().url().path() == getLibModUrl().path()){
    handleRecievedLibMod(reply);
  }
  else{
    DEBUG_MESSAGE("Recieved unknown response")
    DEBUG_MESSAGE(reply->request().url().path().toStdString())
  }
  reply->deleteLater();
}

void UDJServerConnection::handleAuthReply(QNetworkReply* reply){
  bool success = true;
  QVariantMap authReplyJSON = JSONHelper::getAuthReplyFromJSON(reply, success);
  if(reply->error() == QNetworkReply::NoError && success){
    DEBUG_MESSAGE("Got good auth reply")
    emit authenticated(authReplyJSON["ticket_hash"].toByteArray(), authReplyJSON["user_id"].value<user_id_t>());
  }
  else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 401){
    emit authFailed(tr("Incorrect Username and password"));
  }
  else{
    QByteArray responseData = reply->readAll();
    QString responseString = QString::fromUtf8(responseData);
    DEBUG_MESSAGE(responseString.toStdString())
    emit authFailed(
      tr("We're experiencing some techinical difficulties. "
      "We'll be back in a bit"));
  }
}

bool UDJServerConnection::checkReplyAndFireErrors(
  QNetworkReply *reply,
  CommErrorHandler::OperationType opType
)
{
  QByteArray payload;
  QVariant potentialPayload = reply->property(getPayloadPropertyName());
  if(potentialPayload.isValid()){
    payload = potentialPayload.toByteArray();
  }

  if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 404 &&
      opType==CommErrorHandler::LIB_SONG_DELETE)
  {
    return false;
  }
  else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 404){
    DEBUG_MESSAGE(reply->request().url().path().toStdString())
    emit commError(opType, CommErrorHandler::NOT_FOUND_ERROR, payload);
    return true;
  }
  else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 401){
    emit commError(opType, CommErrorHandler::AUTH, payload);
    return true;
  }
  else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 409){
    emit commError(opType, CommErrorHandler::CONFLICT, payload);
    return true;
  }
  else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 500){
    QByteArray response = reply->readAll();
    QString resposneMessage = QString(response);
    DEBUG_MESSAGE(resposneMessage.toStdString())
    emit commError(opType, CommErrorHandler::SERVER_ERROR, payload);
    return true;
  }
  else if(reply->error() != QNetworkReply::NoError){
    DEBUG_MESSAGE("Unknown error: " << QString(reply->readAll()).toStdString())
    emit commError(opType, CommErrorHandler::UNKNOWN_ERROR, payload);
    return true;
  }
  return false;
}

void UDJServerConnection::handleSetActiveReply(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::SET_PLAYER_ACTIVE)){
    emit playerSetActive();
  }
}

void UDJServerConnection::handleSetInactiveReply(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::SET_PLAYER_INACTIVE)){
    emit playerSetInactive();
  }
}

void UDJServerConnection::handleRecievedLibMod(QNetworkReply *reply){
  if(isResponseType(reply, 400)){
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    DEBUG_MESSAGE("400 lib mod error: " << responseMsg.toStdString())
    emit libModError("Got 400");
  }
  else if(isResponseType(reply, 401)){
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    DEBUG_MESSAGE("401 lib mod error: " << responseMsg.toStdString())
    emit libModError("Got 401");
  }
  else if(isResponseType(reply, 404)){
    DEBUG_MESSAGE("404 on lib mod");
    emit libModError("Got 404");
  }
  else if(isResponseType(reply, 409)){
    DEBUG_MESSAGE("409 on lib mod");
    emit libModError("Got 409");
  }
  else if(isResponseType(reply, 500)){
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    DEBUG_MESSAGE("500 lib mod error: " << responseMsg.toStdString())
    emit libModError("Got 500");
  }
  else if(isResponseType(reply, 200)){
    QVariant songsAdded = reply->property(getSongsAddedPropertyName());
    QVariant songsDeleted = reply->property(getSongsDeletedPropertyName());
    std::vector<library_song_id_t> addedIds = JSONHelper::getAddedLibIds(songsAdded.toByteArray());
    std::vector<library_song_id_t> deletedIds =
      JSONHelper::getDeletedLibIds(songsDeleted.toByteArray());
    addedIds.insert(addedIds.begin(), deletedIds.begin(), deletedIds.end());

    emit libSongsSyncedToServer(addedIds);
  }
  else{
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    DEBUG_MESSAGE("Unknown lib mod error: " << responseMsg.toStdString())
    emit libModError("Unknown error");
  }
}

void UDJServerConnection::handleCreatePlayerReply(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::CREATE_PLAYER)){
    //TODO handle bad json resturned from the server.
    player_id_t issuedId = JSONHelper::getPlayerId(reply);
    emit playerCreated(issuedId);
  }
}

void UDJServerConnection::handleRecievedActivePlaylist(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::PLAYLIST_UPDATE)){
    emit newActivePlaylist(JSONHelper::getActivePlaylistFromJSON(reply));
  }
}

void UDJServerConnection::handleRecievedActivePlaylistAdd(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::PLAYLIST_ADD)){
    QString path = reply->request().url().path();
    QRegExp rx("/udj/players/" + QString::number(playerId) + "/active_playlist/(\\d+)");
    rx.indexIn(path);
    library_song_id_t songAdded = rx.cap(1).toLong();
    emit songAddedToActivePlaylist(songAdded);
  }
}

void UDJServerConnection::handleRecievedCurrentSongSet(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::SET_CURRENT_SONG)){
    emit currentSongSet();
  }
}

void UDJServerConnection::handleRecievedActivePlaylistRemove(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::PLAYLIST_REMOVE)){
    QString path = reply->request().url().path();
    QRegExp rx("/udj/players/" + QString::number(playerId) + "/active_playlist/(\\d+)");
    rx.indexIn(path);
    playlist_song_id_t songDeleted = rx.cap(1).toLong();
    emit songRemovedFromActivePlaylist(songDeleted);
  }
}

QUrl UDJServerConnection::getActivePlaylistUrl() const{
  return QUrl(getServerUrlPath() + "players/" + QString::number(playerId) +
    "/active_playlist");
}

QUrl UDJServerConnection::getActivePlaylistAddUrl(const library_song_id_t& libId) const{
  return QUrl(getServerUrlPath() + "players/" + QString::number(playerId) +
    "/active_playlist/songs/" + QString::number(libId));
}

QUrl UDJServerConnection::getActivePlaylistRemoveUrl(
  playlist_song_id_t toDelete) const
{
  return QUrl(getServerUrlPath() + "events/" + QString::number(playerId) +
    "/active_playlist/songs/" + QString::number(toDelete));
}


QUrl UDJServerConnection::getCurrentSongUrl() const{
  return QUrl(getServerUrlPath() + "events/" + QString::number(playerId) +
    "/current_song");
}

QUrl UDJServerConnection::getUsersUrl() const{
  return QUrl(getServerUrlPath() + "events/" + QString::number(playerId) +
    "/users");
}

bool UDJServerConnection::isActivePlaylistRemoveUrl(const QString& path) const{
  QRegExp rx("^/udj/events/" + QString::number(playerId) + 
    "/active_playlist/songs/\\d+$");
  return rx.exactMatch(path);
}

QUrl UDJServerConnection::getCreatePlayerUrl() const{
  return QUrl(getServerUrlPath()+ "users/" + QString::number(user_id) + "/players/player");
}

QUrl UDJServerConnection::getPlayerStateUrl() const{
  return QUrl(getServerUrlPath()+ "users/" + QString::number(user_id) + "/players/" + 
      QString::number(playerId) + "/state");
}

QUrl UDJServerConnection::getLibModUrl() const{
  return QUrl(getServerUrlPath()+ "users/" + QString::number(user_id) + "/players/" + 
      QString::number(playerId) + "/library");
}


bool UDJServerConnection::isPlayerCreateUrl(const QString& path) const{
  return (path == "/udj/users/" + QString::number(user_id) + "/players/player");
}

bool UDJServerConnection::isSetActiveReply(const QNetworkReply *reply) const{
  return
    (reply->request().url().path() == getPlayerStateUrl().path()) &&
    (reply->property(getPayloadPropertyName()).toString() == "state=playing");
}

bool UDJServerConnection::isSetInactiveReply(const QNetworkReply *reply) const{
  return
    reply->request().url().path() == getPlayerStateUrl().path() &&
    reply->property(getPayloadPropertyName()).toString() == "state=inactive";
}

bool UDJServerConnection::isActivePlaylistAddUrl(const QString& path) const{
  QRegExp rx("^/udj/player/" + QString::number(playerId) + 
    "/active_playlist/songs/\\d+$");
  return rx.exactMatch(path);
}

bool UDJServerConnection::isResponseType(QNetworkReply *reply, int code){
  return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == code;
}



}//end namespace
