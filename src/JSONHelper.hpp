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
#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP
#include "ConfigDefs.hpp"
#include <vector>
#include <QVariantList>

class QNetworkReply;

namespace UDJ{

typedef struct {
  library_song_id_t id;
  QString songName;
  QString artistName;
  QString albumName;
  int duration;
} lib_song_t;

/** \brief Class used to help serialize and deserialize JSON messages */
class JSONHelper{

public:

  /** @name Converter Functions */
  //@{
  
  static QByteArray getJSONLibIds(const QSet<library_song_id_t>& libIds);


  /**
   * \brief Creates the JSON necessary for doing a request to add a song
   * to the library.
   *
   * @param song The id of the song to be added
   * @return A bytearray contianing the JSON for the song add request.
   */
  static const QByteArray getJSONForLibAdd(const lib_song_t &song);


  /**
   * \brief Creates the JSON necessary for doing a request to add a song
   * to the library.
   *
   * @param song The id of the song to be added.
   * @param success A boolean whose value will be set to true if the JSON
   * was able to be successfully generated and false otherwise.
   * @return A bytearray contianing the JSON for the song add request.
   */
  static const QByteArray getJSONForLibAdd(
    const lib_song_t &song,
    bool &success);


  static QByteArray getJSONForLibAdd(const QVariantList& songsToAdd);

  static QByteArray getJSONForLibAdd(const QVariantList& songsToAdd, bool &success);

  static QByteArray getJSONForLibDelete(const QVariantList& songsToDelete);

  static QByteArray getJSONForLibDelete(const QVariantList& songsToDelete, bool &success);

  static const QByteArray getCreatePlayerJSON(
    const QString& playerName,
    const QString& password);

  static const QByteArray getCreatePlayerJSON(
    const QString& playerName,
    const QString& password, 
    bool &success);

  static const QByteArray getCreatePlayerJSON(
    const QString& playerName,
    const QString& password,
    const QString& streetAddress,
    const QString& city,
    const QString& state,
    const int& zipcode);

  static const QByteArray getCreatePlayerJSON(
    const QString& playerName,
    const QString& password,
    const QString& streetAddress,
    const QString& city,
    const QString& state,
    const int& zipcode,
    bool &success);

  static std::vector<library_song_id_t> getAddedLibIds(const QByteArray& payload);

  static std::vector<library_song_id_t> getDeletedLibIds(const QByteArray& payload);

  /**
   * \brief Get's the id of a player from the given server reply.
   * 
   * @param reply The reply from the server.
   * @return The player id in the servers response.
   */
  static player_id_t getPlayerId(QNetworkReply *reply);

  /**
   * \brief Gets the active playlist from the servers reply.
   *
   * @param reply The servers response.
   * @return A QVariantList containing all the songs in the active playlist in
   * their approriate order.
   */
  static const QVariantList getActivePlaylistFromJSON(QNetworkReply *reply);

  static const QVariantMap getAuthReplyFromJSON(QNetworkReply *reply, bool &success);

  //@}

};


} //end namespace UDJ
#endif //JSON_HELPER_HPP
