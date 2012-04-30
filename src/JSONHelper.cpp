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
#include <QNetworkReply>
#include "JSONHelper.hpp"
#include "qt-json/json.h"

namespace UDJ{

QByteArray JSONHelper::getJSONForLibAdd(const QVariantList& songsToAdd){
  bool success;
  return getJSONForLibAdd(songsToAdd, success);
}

QByteArray JSONHelper::getJSONForLibAdd(const QVariantList& songsToAdd, bool &success){
  return QtJson::Json::serialize(songsToAdd,success);
}

QByteArray JSONHelper::getJSONForLibDelete(const QVariantList& songsToDelete){
  bool success;
  return getJSONForLibDelete(songsToDelete, success);
}

QByteArray JSONHelper::getJSONForLibDelete(const QVariantList& songsToDelete, bool &success){
  return QtJson::Json::serialize(songsToDelete, success);
}

std::vector<library_song_id_t> JSONHelper::getAddedLibIds(const QByteArray& payload){
  QString responseString = QString::fromUtf8(payload);
  bool success;
  QVariantList songsAdded = 
    QtJson::Json::parse(responseString, success).toList();
  if(!success){
    std::cerr << "Error parsing json from a response to an add library entry" <<
     "request" << std::endl <<
      responseString.toStdString() << std::endl;
  }

  std::vector<library_song_id_t> toReturn(songsAdded.size());
  for(int i=0; i<songsAdded.size(); ++i){
    toReturn[i] = songsAdded[i].toMap()["id"].value<library_song_id_t>();
  }
  return toReturn;
}

std::vector<library_song_id_t> JSONHelper::getDeletedLibIds(const QByteArray& payload){
  QString responseString = QString::fromUtf8(payload);
  bool success;
  QVariantList songsDeleted = 
    QtJson::Json::parse(responseString, success).toList();
  if(!success){
    std::cerr << "Error parsing json from a response to an delete library entry" <<
     "request" << std::endl <<
      responseString.toStdString() << std::endl;
  }

  std::vector<library_song_id_t> toReturn(songsDeleted.size());
  for(int i=0; i<songsDeleted.size(); ++i){
    toReturn[i] = songsDeleted[i].value<library_song_id_t>();
  }
  return toReturn;
}

const QByteArray JSONHelper::getCreatePlayerJSON(
  const QString& playerName,
  const QString& password)
{
  bool success;
  return getCreatePlayerJSON(playerName, password, success);
}

const QByteArray JSONHelper::getCreatePlayerJSON(
  const QString& playerName,
  const QString& password,
  bool &success)
{
  QVariantMap playerToCreate;
  playerToCreate["name"] = playerName;
  if(password != ""){
    playerToCreate["password"] = password;
  }
  return QtJson::Json::serialize(QVariant(playerToCreate),success);
}

const QByteArray JSONHelper::getCreatePlayerJSON(
  const QString& playerName,
  const QString& password,
  const QString& streetAddress,
  const QString& city,
  const QString& state,
  const int& zipcode)
{
  bool success;
  return getCreatePlayerJSON(playerName, password, streetAddress, city, state, zipcode, success);
}

const QByteArray JSONHelper::getCreatePlayerJSON(
  const QString& playerName,
  const QString& password,
  const QString& streetAddress,
  const QString& city,
  const QString& state,
  const int& zipcode,
  bool &success)
{
  QVariantMap playerToCreate;
  playerToCreate["name"] = playerName;
  if(password != ""){
    playerToCreate["password"] = password;
  }

  playerToCreate["address"] = streetAddress;
  playerToCreate["city"] = city;
  playerToCreate["state"] = state;
  playerToCreate["zipcode"] = zipcode;

  return QtJson::Json::serialize(QVariant(playerToCreate),success);
}



const QByteArray JSONHelper::getAddToActiveJSON(
  const std::vector<client_request_id_t>& requestIds,
  const std::vector<library_song_id_t>& libIds)
{
  bool success;
  return getAddToActiveJSON(requestIds, libIds, success);
}

const QByteArray JSONHelper::getAddToActiveJSON(
  const std::vector<client_request_id_t>& requestIds,
  const std::vector<library_song_id_t>& libIds,
  bool &success)
{
  QVariantList toSerialize;
  std::vector<library_song_id_t>::const_iterator songIt = libIds.begin();
  for(
    std::vector<client_request_id_t>::const_iterator requestIt = 
      requestIds.begin();
    requestIt != requestIds.end() && songIt != libIds.end();
    ++requestIt, ++songIt)
  {
    QVariantMap requestToAdd;
    requestToAdd["lib_id"] = QVariant::fromValue<library_song_id_t>(*songIt);
    requestToAdd["client_request_id"] = 
      QVariant::fromValue<client_request_id_t>(*requestIt);
    toSerialize.append(requestToAdd);
  }
  return QtJson::Json::serialize(toSerialize, success);
}


player_id_t JSONHelper::getPlayerId(QNetworkReply *reply){
  QByteArray responseData = reply->readAll();
  QString responseString = QString::fromUtf8(responseData);
  bool success;
  QVariantMap playerCreated = 
    QtJson::Json::parse(responseString, success).toMap();
  if(!success){
    std::cerr << "Error parsing json from a response to an player creation" <<
     "request" << std::endl <<
      responseString.toStdString() << std::endl;
  }

  return playerCreated["player_id"].value<player_id_t>();
}

const QVariantList JSONHelper::getActivePlaylistFromJSON(QNetworkReply *reply){
  QByteArray responseData = reply->readAll();
  QString responseString = QString::fromUtf8(responseData);
  bool success;
  QVariantMap activePlaylist = 
    QtJson::Json::parse(responseString, success).toMap();
  if(!success){
    std::cerr << "Error parsing json from a response to an event creation" <<
     "request" << std::endl <<
      responseString.toStdString() << std::endl;
  }
  return activePlaylist["active_playlist"].toList();
}

/*const QVariantList JSONHelper::getEventGoersJSON(QNetworkReply *reply){
  QByteArray responseData = reply->readAll();
  QString responseString = QString::fromUtf8(responseData);
  bool success;
  QVariantList eventGoers = 
    QtJson::Json::parse(responseString, success).toList();
  return eventGoers;
}*/

/*
const QVariantMap JSONHelper::getSingleEventInfo(QNetworkReply *reply){
  QByteArray responseData = reply->readAll();
  QString responseString = QString::fromUtf8(responseData);
  bool success;
  QVariantMap event = 
    QtJson::Json::parse(responseString, success).toMap();
  return event;
}*/

std::vector<client_request_id_t> JSONHelper::extractAddRequestIds(
  const QByteArray& payload)
{
  QString payloadString = QString::fromUtf8(payload);
  bool success;
  QVariantList addRequests = 
    QtJson::Json::parse(payloadString, success).toList();
  std::vector<client_request_id_t> requestIds(addRequests.size());
  for(int i=0; i<addRequests.size(); ++i){
    requestIds[i] = 
      addRequests[i].toMap()["client_request_id"].value<client_request_id_t>();
  }
  return requestIds;
}

const QVariantMap JSONHelper::getAuthReplyFromJSON(QNetworkReply *reply, bool &success){
  QByteArray responseData = reply->readAll();
  QString responseString = QString::fromUtf8(responseData);
  QVariantMap authReply =
    QtJson::Json::parse(responseString, success).toMap();
  return authReply;
}

} //end namespace UDJ
