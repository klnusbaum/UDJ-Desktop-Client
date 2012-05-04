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
#include <QBuffer>
#include <QRegExp>
#include <QStringList>
#include "UDJServerConnection.hpp"
#include "JSONHelper.hpp"
#include <QSet>


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

void UDJServerConnection::modActivePlaylist(
  const QSet<library_song_id_t>& toAdd,
  const QSet<library_song_id_t>& toRemove
)
{
  QNetworkRequest modRequest(getActivePlaylistUrl());
  modRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QByteArray addJSON = JSONHelper::getJSONLibIds(toAdd);
  QByteArray removeJSON = JSONHelper::getJSONLibIds(toRemove);
  QUrl params;
  params.addQueryItem("to_add", addJSON);
  params.addQueryItem("to_remove", removeJSON);
  QByteArray payload = params.encodedQuery();
  QNetworkReply *reply = netAccessManager->post(modRequest, payload);
  reply->setProperty(getSongsAddedPropertyName(), addJSON);
  reply->setProperty(getSongsRemovedPropertyName(), removeJSON);
}

void UDJServerConnection::setCurrentSong(library_song_id_t currentSong){
  DEBUG_MESSAGE("Setting current song")
  QString params = "lib_id="+QString::number(currentSong);
  QNetworkRequest setCurrentSongRequest(getCurrentSongUrl());
  setCurrentSongRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QNetworkReply *reply =
    netAccessManager->post(setCurrentSongRequest, params.toUtf8());
}

void UDJServerConnection::setVolume(int volume){
  DEBUG_MESSAGE("Setting volume")
  QUrl params;
  params.addQueryItem("volume", QString::number(volume));
  QNetworkRequest setCurrentVolumeRequest(getVolumeUrl());
  setCurrentVolumeRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QNetworkReply *reply = 
    netAccessManager->post(setCurrentVolumeRequest,  params.encodedQuery());
}

void UDJServerConnection::setPlayerState(const QString& newState){
  DEBUG_MESSAGE("Setting player state to " << newState.toStdString())
  QString params("state="+newState);
  QByteArray payload = params.toUtf8();
  QNetworkRequest setPlayerActiveRequest(getPlayerStateUrl());
  setPlayerActiveRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QNetworkReply *reply = netAccessManager->post(setPlayerActiveRequest, payload);
  reply->setProperty(getStatePropertyName(), newState);
}

void UDJServerConnection::recievedReply(QNetworkReply *reply){
  if(reply->request().url().path() == getAuthUrl().path()){
    handleAuthReply(reply);
  }
  else if(reply->request().url().path() == getPlayerStateUrl().path()){
    handleSetStateReply(reply);
  }
  else if(isPlayerCreateUrl(reply->request().url().path())){
    handleCreatePlayerReply(reply);
  }
  else if(isGetActivePlaylistReply(reply)){
    handleRecievedActivePlaylist(reply);
  }
  else if(reply->request().url().path() == getCurrentSongUrl().path()){
    handleRecievedCurrentSongSet(reply);
  }
  else if(reply->request().url().path() == getLibModUrl().path()){
    handleRecievedLibMod(reply);
  }
  else if(reply->request().url().path() == getVolumeUrl().path()){
    handleRecievedVolumeSet(reply);
  }
  else if(isModActivePlaylistReply(reply)){
    handleRecievedPlaylistMod(reply);
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

void UDJServerConnection::handleSetStateReply(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit playerStateSet(reply->property(getStatePropertyName()).toString());
  }
  else{
    //TODO handle error
  }
}


void UDJServerConnection::handleRecievedLibMod(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    QVariant songsAdded = reply->property(getSongsAddedPropertyName());
    QVariant songsDeleted = reply->property(getSongsDeletedPropertyName());
    std::vector<library_song_id_t> addedIds = JSONHelper::getLibIds(songsAdded.toByteArray());
    std::vector<library_song_id_t> deletedIds =
      JSONHelper::getLibIds(songsDeleted.toByteArray());
    addedIds.insert(addedIds.begin(), deletedIds.begin(), deletedIds.end());

    emit libSongsSyncedToServer(addedIds);
  }
  else{
    DEBUG_MESSAGE("Got bad lib mod")
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit libModError("error: " + responseMsg,
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
        reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleCreatePlayerReply(QNetworkReply *reply){
  if(isResponseType(reply, 201)){
    player_id_t issuedId = JSONHelper::getPlayerId(reply);
    emit playerCreated(issuedId);
  }
  else{
    DEBUG_MESSAGE("Player creation failed")
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit playerCreationFailed(
      "error: " + responseMsg, 
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleRecievedActivePlaylist(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit newActivePlaylist(JSONHelper::getActivePlaylistFromJSON(reply));
  }
  else{
    DEBUG_MESSAGE("Getting playlist failed")
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit getActivePlaylistFail(
      "error: " + responseMsg, 
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleRecievedPlaylistMod(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit activePlaylistModified(
      JSONHelper::extractSongLibIds(reply->property(getSongsAddedPropertyName()).toByteArray()),
      JSONHelper::extractSongLibIds(reply->property(getSongsRemovedPropertyName()).toByteArray())
    );
  }
  else{
    DEBUG_MESSAGE("Modding playlist failed")
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit activePlaylistModFailed(
      "error: " + responseMsg,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleRecievedCurrentSongSet(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit currentSongSet();
  }
  else{
    DEBUG_MESSAGE("Setting current song failed")
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit setCurrentSongFailed(
      "error: " + responseMsg,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleRecievedVolumeSet(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit volumeSetOnServer();
  }
  else{
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit setVolumeFailed(
      "error: " + responseMsg,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

QUrl UDJServerConnection::getActivePlaylistUrl() const{
  return QUrl(getServerUrlPath() + "players/" + QString::number(playerId) +
    "/active_playlist");
}

QUrl UDJServerConnection::getCurrentSongUrl() const{
  return QUrl(getServerUrlPath() + "players/" + QString::number(playerId) +
    "/current_song");
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

QUrl UDJServerConnection::getVolumeUrl() const{
  return QUrl(getServerUrlPath()+ "users/" + QString::number(user_id) + "/players/" + 
      QString::number(playerId) + "/volume");
}


bool UDJServerConnection::isPlayerCreateUrl(const QString& path) const{
  return (path == "/udj/users/" + QString::number(user_id) + "/players/player");
}

bool UDJServerConnection::isResponseType(QNetworkReply *reply, int code){
  return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == code;
}

bool UDJServerConnection::isGetActivePlaylistReply(const QNetworkReply *reply) const{
  return
    reply->request().url().path() == getActivePlaylistUrl().path()
    && !reply->property(getSongsRemovedPropertyName()).isValid()
    && !reply->property(getSongsAddedPropertyName()).isValid();
}

bool UDJServerConnection::isModActivePlaylistReply(const QNetworkReply *reply) const{
  return
    reply->request().url().path() == getActivePlaylistUrl().path()
    && reply->property(getSongsRemovedPropertyName()).isValid()
    && reply->property(getSongsAddedPropertyName()).isValid();
}



}//end namespace
