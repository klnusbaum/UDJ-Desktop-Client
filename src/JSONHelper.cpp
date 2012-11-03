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
#include <QSet>
#include <iostream>

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

QSet<library_song_id_t> JSONHelper::getLibIds(const QByteArray& payload){
  QString responseString = QString::fromUtf8(payload);
  bool success;
  QVariantList songs = 
    QtJson::Json::parse(responseString, success).toList();
  if(!success){
    std::cerr << "Error parsing json from a response to an add library entry" <<
     "request" << std::endl <<
      responseString.toStdString() << std::endl;
  }

  QSet<library_song_id_t> toReturn;
  Q_FOREACH(QVariant song, songs){
    toReturn.insert(song.toMap()["id"].value<library_song_id_t>());
  }
  return toReturn;
}

QSet<library_song_id_t> JSONHelper::convertLibIdArray(const QByteArray& payload){
  QString responseString = QString::fromUtf8(payload);
  bool success;
  QVariantList ids = 
    QtJson::Json::parse(responseString, success).toList();
  if(!success){
    std::cerr << "Error parsing json from a response to an delete library entry" <<
     "request" << std::endl <<
      responseString.toStdString() << std::endl;
  }

  QSet<library_song_id_t> toReturn;
  Q_FOREACH(QVariant id, ids){
    toReturn.insert(id.value<library_song_id_t>());
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
  const QString& zipcode)
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
  const QString& zipcode,
  bool &success)
{
  QVariantMap playerToCreate;
  playerToCreate["name"] = playerName;
  if(password != ""){
    playerToCreate["password"] = password;
  }

  QVariantMap location;
  location["address"] = streetAddress;
  location["locality"] = city;
  location["region"] = state;
  location["postal_code"] = zipcode;
  location["country"] = "United States";
  playerToCreate["location"] = location;

  return QtJson::Json::serialize(QVariant(playerToCreate),success);
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

  return playerCreated["id"].value<player_id_t>();
}

QVariantMap JSONHelper::getActivePlaylistFromJSON(QNetworkReply *reply){
  QByteArray responseData = reply->readAll();
  QString responseString = QString::fromUtf8(responseData);
  bool success;
  QVariantMap activePlaylist = 
    QtJson::Json::parse(responseString, success).toMap();
  if(!success){
    std::cerr << "Error parsing json from a response to an acitve Playlist request" <<
     "request" << std::endl <<
      responseString.toStdString() << std::endl;
  }
  return activePlaylist;
}

QVariantList JSONHelper::getParticipantListFromJSON(QNetworkReply *reply){
  QByteArray responseData = reply->readAll();
  QString responseString = QString::fromUtf8(responseData);
  bool success;
  QVariantList participantsList = 
    QtJson::Json::parse(responseString, success).toList();
  if(!success){
    std::cerr << "Error parsing json from a response to an get Participants List request" <<
     std::endl <<
      responseString.toStdString() << std::endl;
  }
  return participantsList;
}

QByteArray JSONHelper::getJSONLibIds(const QSet<library_song_id_t>& libIds){
  bool success;
  QVariantList idList;
  Q_FOREACH(library_song_id_t libId, libIds){
    idList.append(QVariant::fromValue(libId));
  }
  return QtJson::Json::serialize(idList, success);
}

QSet<library_song_id_t> JSONHelper::extractSongLibIds(const QByteArray& idsString){
  bool success;
  QVariantList libIds = QtJson::Json::parse(idsString, success).toList();
  QSet<library_song_id_t> toReturn;
  Q_FOREACH(QVariant libId, libIds){
    toReturn.insert(libId.value<library_song_id_t>());
  }
  return toReturn;
}


const QVariantMap JSONHelper::getAuthReplyFromJSON(QNetworkReply *reply, bool &success){
  QByteArray responseData = reply->readAll();
  QString responseString = QString::fromUtf8(responseData);
  QVariantMap authReply =
    QtJson::Json::parse(responseString, success).toMap();
  return authReply;
}

} //end namespace UDJ
