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

void UDJServerConnection::addLibSongsToServer(const QVariantList& songs){
  DEBUG_MESSAGE("Adding songs to library on server")

  QByteArray songJSON = JSONHelper::getJSONForLibAdd(songs);
  QNetworkRequest addSongRequest(getLibAddSongUrl());
  prepareJSONRequest(addSongRequest);
  QNetworkReply *reply = netAccessManager->put(addSongRequest, songJSON);
  reply->setProperty(getPayloadPropertyName(), songJSON);

}

void UDJServerConnection::deleteLibSongOnServer(library_song_id_t toDeleteId){
  QNetworkRequest deleteSongRequest(getLibDeleteSongUrl(toDeleteId));
  deleteSongRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QNetworkReply *reply = netAccessManager->deleteResource(deleteSongRequest);
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

void UDJServerConnection::addSongToActivePlaylist(
  client_request_id_t requestId, 
  library_song_id_t songId)
{
  std::vector<client_request_id_t> requestIds(1, requestId);
  std::vector<library_song_id_t> songIds(1, songId);
  addSongsToActivePlaylist(requestIds, songIds);
}

void UDJServerConnection::addSongsToActivePlaylist(
  const std::vector<client_request_id_t>& requestIds, 
  const std::vector<library_song_id_t>& songIds)
{
  QNetworkRequest add2ActivePlaylistRequest(getActivePlaylistAddUrl());
  prepareJSONRequest(add2ActivePlaylistRequest);
  bool success;
  const QByteArray songsAddJSON = JSONHelper::getAddToActiveJSON(
    requestIds,
    songIds,
    success);
  if(!success){
    //TODO handle error
    DEBUG_MESSAGE("Error serializing active playlist addition reuqest")
    return;
  }
  QNetworkReply *reply = 
    netAccessManager->put(add2ActivePlaylistRequest, songsAddJSON);
  reply->setProperty(getPayloadPropertyName(), songsAddJSON); 
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
  else if(reply->request().url().path() == getLibAddSongUrl().path()){
    handleAddLibSongsReply(reply);
  }
  else if(isPlayerCreateUrl(reply->request().url().path())){
    handleCreatePlayerReply(reply);
  }
  else if(reply->request().url().path() == getActivePlaylistUrl().path()){
    handleRecievedActivePlaylist(reply);
  }
  else if(reply->request().url().path() == getActivePlaylistAddUrl().path()){
    handleRecievedActivePlaylistAdd(reply);
  }
  else if(reply->request().url().path() == getCurrentSongUrl().path()){
    handleRecievedCurrentSongSet(reply);
  }
  else if(isLibDeleteUrl(reply->request().url().path())){
    handleDeleteLibSongsReply(reply);
  }
  else if(isActivePlaylistRemoveUrl(reply->request().url().path())){
    handleRecievedActivePlaylistRemove(reply);
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


void UDJServerConnection::handleAddLibSongsReply(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::LIB_SONG_ADD)){
    QVariant payload = reply->property(getPayloadPropertyName());
    std::vector<library_song_id_t> updatedIds = JSONHelper::getUpdatedLibIds(payload.toByteArray());
    emit songsAddedToLibOnServer(updatedIds);
  }
}


void UDJServerConnection::handleDeleteLibSongsReply(QNetworkReply *reply){
  if(!checkReplyAndFireErrors(reply, CommErrorHandler::LIB_SONG_DELETE)){
    QString path = reply->request().url().path();
    QRegExp rx("/udj/users/" + QString::number(user_id) + "/players/" + 
        QString::number(playerId) + "/library/(\\d+)");
    rx.indexIn(path);
    library_song_id_t songDeleted = rx.cap(1).toLong();
    emit songDeletedFromLibOnServer(songDeleted);
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
    QVariant payload = reply->property(getPayloadPropertyName());
    emit songsAddedToActivePlaylist(JSONHelper::extractAddRequestIds(payload.toByteArray()));
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

QUrl UDJServerConnection::getLibAddSongUrl() const{
  return QUrl(getServerUrlPath() + "users/" + QString::number(user_id) +
    "/players/" + QString::number(playerId) + "/library/songs");
}

QUrl UDJServerConnection::getLibDeleteSongUrl(library_song_id_t toDelete) const{
  return QUrl(getServerUrlPath() + "users/" + QString::number(user_id) +
    "/players/" + QString::number(playerId) + "/library/" + QString::number(toDelete));
}

QUrl UDJServerConnection::getActivePlaylistUrl() const{
  return QUrl(getServerUrlPath() + "players/" + QString::number(playerId) +
    "/active_playlist");
}

QUrl UDJServerConnection::getActivePlaylistAddUrl() const{
  return QUrl(getServerUrlPath() + "events/" + QString::number(playerId) +
    "/active_playlist/songs");
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

bool UDJServerConnection::isLibDeleteUrl(const QString& path) const{
  QRegExp rx("^/udj/users/" + QString::number(user_id) + "/players/" +
      QString::number(playerId) + "/library/\\d+$");
  return rx.exactMatch(path);
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




}//end namespace
