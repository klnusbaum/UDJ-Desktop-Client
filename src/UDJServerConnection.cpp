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
#include "Logger.hpp"
#include <QSet>


QByteArray stripControllCharacters(const QByteArray& toStrip){
  QString stripString = QString::fromUtf8(toStrip);
  stripString = stripString.replace("[\\x00-\\x1f]", "");
  return stripString.toUtf8();
}

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
  Logger::instance()->log("Doing auth request");
}

void UDJServerConnection::modLibContents(const QVariantList& songsToAdd,
   const QVariantList& songsToDelete)
{
  QNetworkRequest modRequest(getLibModUrl());
  modRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QByteArray addJSON = JSONHelper::getJSONForLibAdd(songsToAdd);
  QByteArray deleteJSON = JSONHelper::getJSONForLibDelete(songsToDelete);
  Logger::instance()->log("Lib mod add JSON: " + QString::fromUtf8(addJSON));
  Logger::instance()->log("Lib mod delete JSON: " + QString::fromUtf8(deleteJSON));
  addJSON = stripControllCharacters(addJSON);
  //Don't use Qt URL functions to encode. They do weird stuff that we don't
  //want like attempt to encode unicode characters in url encoding style
  QByteArray payload = "to_add=" + 
    addJSON.replace("%", "%25").replace("&", "%26").replace("=", "%3D").replace(";", "%3B").replace("\x02","")
    + "&to_delete=" + deleteJSON;
  Logger::instance()->log("Lib mod payload: " + QString::fromUtf8(payload));
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
  Logger::instance()->log("Sending player json to server for creation: " + QString(playerJSON));
  createPlayer(playerJSON);
}

void UDJServerConnection::createPlayer(const QByteArray& payload){
  QNetworkRequest createPlayerRequest(getCreatePlayerUrl());
  prepareJSONRequest(createPlayerRequest);
  /*QNetworkReply *reply =*/ netAccessManager->put(createPlayerRequest, payload);
}

void UDJServerConnection::removePlayerPassword(){
  QNetworkRequest removePasswordRequest(getPlayerPasswordUrl());
  removePasswordRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  /*QNetworkReply *reply =*/ netAccessManager->deleteResource(removePasswordRequest);
}

void UDJServerConnection::setPlayerPassword(const QString& newPassword){
  QNetworkRequest setPasswordRequest(getPlayerPasswordUrl());
  setPasswordRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QUrl params;
  params.addQueryItem("password", newPassword);
  QByteArray payload = params.encodedQuery();
  QNetworkReply *reply = netAccessManager->post(setPasswordRequest, payload);
  reply->setProperty(getPlayerPasswordPropertyName(), newPassword);
}

void UDJServerConnection::setPlayerLocation(
  const QString& streetAddress,
  const QString& city,
  const QString& state,
  int zipcode
)
{
  QNetworkRequest setLocationRequest(getPlayerLocationUrl());
  setLocationRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QUrl params;
  params.addQueryItem("address", streetAddress);
  params.addQueryItem("city", city);
  params.addQueryItem("state", state);
  params.addQueryItem("zipcode", QString::number(zipcode));
  QByteArray payload = params.encodedQuery();
  QNetworkReply *reply = netAccessManager->post(setLocationRequest, payload);
  reply->setProperty(getLocationAddressPropertyName(), streetAddress);
  reply->setProperty(getLocationCityPropertyName(), city);
  reply->setProperty(getLocationStatePropertyName(), state);
  reply->setProperty(getLocationZipcodePropertyName(), zipcode);
}

void UDJServerConnection::setPlayerName(const QString& newName){
  QNetworkRequest setNameRequest(getPlayerNameUrl());
  setNameRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  QUrl params;
  params.addQueryItem("name", newName);
  QByteArray payload = params.encodedQuery();
  QNetworkReply *reply = netAccessManager->post(setNameRequest, payload);
  reply->setProperty(getPlayerNamePropertyName(), newName);
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
  Logger::instance()->log("Setting current song");
  QString params = "lib_id="+QString::number(currentSong);
  QNetworkRequest setCurrentSongRequest(getCurrentSongUrl());
  setCurrentSongRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  /*QNetworkReply *reply =*/
    netAccessManager->post(setCurrentSongRequest, params.toUtf8());
}

void UDJServerConnection::setVolume(int volume){
  Logger::instance()->log("Setting volume");
  QUrl params;
  params.addQueryItem("volume", QString::number(volume));
  QNetworkRequest setCurrentVolumeRequest(getVolumeUrl());
  setCurrentVolumeRequest.setRawHeader(getTicketHeaderName(), ticket_hash);
  /*QNetworkReply *reply = */
    netAccessManager->post(setCurrentVolumeRequest,  params.encodedQuery());
}

void UDJServerConnection::setPlayerState(const QString& newState){
  Logger::instance()->log("Setting player state to " + newState);
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
    handleReceivedActivePlaylist(reply);
  }
  else if(reply->request().url().path() == getCurrentSongUrl().path()){
    handleReceivedCurrentSongSet(reply);
  }
  else if(reply->request().url().path() == getLibModUrl().path()){
    handleReceivedLibMod(reply);
  }
  else if(reply->request().url().path() == getVolumeUrl().path()){
    handleReceivedVolumeSet(reply);
  }
  else if(isModActivePlaylistReply(reply)){
    handleReceivedPlaylistMod(reply);
  }
  else if(reply->request().url().path() == getPlayerNameUrl().path()){
    handleNameSetReply(reply);
  }
  else if(reply->request().url().path() == getPlayerLocationUrl().path()){
    handleLocationSetReply(reply);
  }
  else if(isPasswordSetReply(reply)){
    handlePlayerPasswordSetReply(reply);
  }
  else if(isPasswordRemoveReply(reply)){
    handlePlayerPasswordRemoveReply(reply);
  }
  else{
    Logger::instance()->log("Received unknown response");
    Logger::instance()->log(reply->request().url().path());
  }
  reply->deleteLater();
}

void UDJServerConnection::handleAuthReply(QNetworkReply* reply){
  bool success = true;
  QVariantMap authReplyJSON = JSONHelper::getAuthReplyFromJSON(reply, success);
  if(reply->error() == QNetworkReply::NoError && success){
    Logger::instance()->log("Got good auth reply");
    emit authenticated(authReplyJSON["ticket_hash"].toByteArray(), authReplyJSON["user_id"].value<user_id_t>());
  }
  else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 401){
    emit authFailed(tr("Incorrect Username and password"));
  }
  else{
    QByteArray responseData = reply->readAll();
    QString responseString = QString::fromUtf8(responseData);
    Logger::instance()->log(responseString);
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
    QString responseData = QString(reply->readAll());
    Logger::instance()->log("Player state set error " + responseData);
    emit playerStateSetError(
      responseData,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handlePlayerPasswordRemoveReply(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit playerPasswordRemoved();
  }
  else{
    QString responseData = QString(reply->readAll());
    Logger::instance()->log("Player password remove error " + responseData);
    emit playerPasswordRemoveError(
      responseData,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}


void UDJServerConnection::handlePlayerPasswordSetReply(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit playerPasswordSet(reply->property(getPlayerPasswordPropertyName()).toString());
  }
  else{
    QString responseData = QString(reply->readAll());
    Logger::instance()->log("Set player password error " + responseData);
    emit playerPasswordSetError(
      responseData,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleNameSetReply(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit playerNameChanged(reply->property(getPlayerNamePropertyName()).toString());
  }
  else if(isResponseType(reply, 409)){
    Logger::instance()->log("Change player name got 409");
    emit playerNameChangeError("Name already in use.", 409, reply->rawHeaderPairs());
  }
  else{
    QString responseData = QString(reply->readAll());
    Logger::instance()->log("Change player name got error " + responseData);
    emit playerNameChangeError(
      responseData,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleLocationSetReply(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit playerLocationSet(
      reply->property(getLocationAddressPropertyName()).toString(),
      reply->property(getLocationCityPropertyName()).toString(),
      reply->property(getLocationStatePropertyName()).toString(),
      reply->property(getLocationZipcodePropertyName()).toInt());
  }
  else{
    QString responseData = QString(reply->readAll());
    Logger::instance()->log("Setting player location error: " + responseData);
    emit playerLocationSetError(responseData,
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
        reply->rawHeaderPairs());
  }
}


void UDJServerConnection::handleReceivedLibMod(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    QVariant songsAdded = reply->property(getSongsAddedPropertyName());
    QVariant songsDeleted = reply->property(getSongsDeletedPropertyName());
    QSet<library_song_id_t> addedIds = JSONHelper::getLibIds(songsAdded.toByteArray());
    QSet<library_song_id_t> deletedIds =
      JSONHelper::convertLibIdArray(songsDeleted.toByteArray());
    QSet<library_song_id_t> allSynced = addedIds.unite(deletedIds);
    emit libSongsSyncedToServer(allSynced);
  }
  else if(isResponseType(reply, 409)){
    QSet<library_song_id_t> alreadyExistingIds = JSONHelper::convertLibIdArray(reply->readAll());
    emit libSongsSyncedToServer(alreadyExistingIds);
  }
  else{
    Logger::instance()->log("Got bad lib mod");
    QByteArray response = reply->readAll();
    QString responseMsg = QString::fromUtf8(response);
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
    Logger::instance()->log("Player creation failed");
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit playerCreationFailed(
      "error: " + responseMsg, 
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleReceivedActivePlaylist(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit newActivePlaylist(JSONHelper::getActivePlaylistFromJSON(reply));
  }
  else{
    Logger::instance()->log("Getting playlist failed");
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit getActivePlaylistFail(
      "error: " + responseMsg, 
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleReceivedPlaylistMod(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit activePlaylistModified(
      JSONHelper::extractSongLibIds(reply->property(getSongsAddedPropertyName()).toByteArray()),
      JSONHelper::extractSongLibIds(reply->property(getSongsRemovedPropertyName()).toByteArray())
    );
  }
  else{
    Logger::instance()->log("Modding playlist failed");
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit activePlaylistModFailed(
      "error: " + responseMsg,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleReceivedCurrentSongSet(QNetworkReply *reply){
  if(isResponseType(reply, 200)){
    emit currentSongSet();
  }
  else{
    Logger::instance()->log("Setting current song failed");
    QByteArray response = reply->readAll();
    QString responseMsg = QString(response);
    emit setCurrentSongFailed(
      "error: " + responseMsg,
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
      reply->rawHeaderPairs());
  }
}

void UDJServerConnection::handleReceivedVolumeSet(QNetworkReply *reply){
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

QUrl UDJServerConnection::getPlayerNameUrl() const{
  return QUrl(getServerUrlPath()+ "users/" + QString::number(user_id) + "/players/" + 
      QString::number(playerId) + "/name");
}

QUrl UDJServerConnection::getPlayerLocationUrl() const{
  return QUrl(getServerUrlPath()+ "users/" + QString::number(user_id) + "/players/" + 
      QString::number(playerId) + "/location");
}

QUrl UDJServerConnection::getPlayerPasswordUrl() const{
  return QUrl(getServerUrlPath()+ "users/" + QString::number(user_id) + "/players/" + 
      QString::number(playerId) + "/password");
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

bool UDJServerConnection::isPasswordSetReply(const QNetworkReply *reply) const{
  return reply->request().url().path() == getPlayerPasswordUrl().path() &&
    reply->operation() == QNetworkAccessManager::PostOperation;
}

bool UDJServerConnection::isPasswordRemoveReply(const QNetworkReply *reply) const{
  return reply->request().url().path() == getPlayerPasswordUrl().path() &&
    reply->operation() == QNetworkAccessManager::DeleteOperation;
}


}//end namespace
