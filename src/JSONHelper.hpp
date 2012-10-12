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

/** \brief Class used to help serialize and deserialize JSON messages */
class JSONHelper{

public:

  /** @name Converter Functions */
  //@{

  /**
   * \brief Given a set of library ids, gets a QByteArray JSON representation of the
   * library ids.
   *
   * @param libIds Ids to convert to JSON.
   * @return A QByteArray JSON representation of the given library ids.
   */
  static QByteArray getJSONLibIds(const QSet<library_song_id_t>& libIds);

  /**
   * \brief Extracts a set of library song ids from a QByteArray JSON repsenation.
   *
   * @param idsString A QByteArray of JSON with the song ids to be extracted.
   * \return A set of library song ids in the given idsString.
   */
  static QSet<library_song_id_t> extractSongLibIds(const QByteArray& idsString);

  /**
   * \brief Gets the json needed for adding the given songs to add to the library.
   *
   * @param songsToAdd Songs that should be converted to JSON.
   * @return A JSON representation of the songs to be added.
   */
  static QByteArray getJSONForLibAdd(const QVariantList& songsToAdd);

  /**
   * \brief Gets the json needed for adding the given songs to add to the library.
   *
   * @param songsToAdd Songs that should be converted to JSON.
   * @param success A boolean that will be set to true or false depending on wether or not the 
   * JSON was succesfully created.
   * @return A JSON representation of the songs to be added.
   */
  static QByteArray getJSONForLibAdd(const QVariantList& songsToAdd, bool &success);

  /**
   * \brief Gets the json needed for deleting the given songs from a library.
   *
   * @param songsToDelete Songs that should be converted to JSON.
   * @return A JSON representation of the songs to be removed.
   */
  static QByteArray getJSONForLibDelete(const QVariantList& songsToDelete);

  /**
   * \brief Gets the json needed for deleting the given songs from a library.
   *
   * @param songsToDelete Songs that should be converted to JSON.
   * @param success A boolean that will be set to true or false depending on wether or not the 
   * JSON was succesfully created.
   * @return A JSON representation of the songs to be removed.
   */
  static QByteArray getJSONForLibDelete(const QVariantList& songsToDelete, bool &success);

  /**
   * \brief Gets the json needed for creating a player.
   *
   * @param playerName Name of the player to be created.
   * @param password Password of the player to be created.
   * @return A JSON representation of the player to be created.
   */
  static const QByteArray getCreatePlayerJSON(
    const QString& playerName,
    const QString& password);

  /**
   * \brief Gets the json needed for creating a player.
   *
   * @param playerName Name of the player to be created.
   * @param password Password of the player to be created.
   * @param success A boolean that will be set to true or false depending on wether or not the 
   * JSON was succesfully created.
   * @return A JSON representation of the player to be created.
   */
  static const QByteArray getCreatePlayerJSON(
    const QString& playerName,
    const QString& password, 
    bool &success);

  /**
   * \brief Gets the json needed for creating a player.
   *
   * @param playerName Name of the player to be created.
   * @param password Password of the player to be created.
   * @param streetAddress Address of the player to be created.
   * @param city City of the player to be created.
   * @param state State of the player to be created.
   * @param zipcode Zip code of the player to be created.
   * @return A JSON representation of the player to be created.
   */
  static const QByteArray getCreatePlayerJSON(
    const QString& playerName,
    const QString& password,
    const QString& streetAddress,
    const QString& city,
    const QString& state,
    const QString& zipcode);

  /**
   * \brief Gets the json needed for creating a player.
   *
   * @param playerName Name of the player to be created.
   * @param password Password of the player to be created.
   * @param streetAddress Address of the player to be created.
   * @param city City of the player to be created.
   * @param state State of the player to be created.
   * @param zipcode Zip code of the player to be created.
   * @param success A boolean that will be set to true or false depending on wether or not the 
   * JSON was succesfully created.
   * @return A JSON representation of the player to be created.
   */
  static const QByteArray getCreatePlayerJSON(
    const QString& playerName,
    const QString& password,
    const QString& streetAddress,
    const QString& city,
    const QString& state,
    const QString& zipcode,
    bool &success);

  /**
   * \brief Given JSON, this functions extracts a vector containing all of the
   * library ids that are in it.
   *
   * @param payload The JSON from which the library ids should be extracted.
   * @return A vector containing the extracted library ids.
   */
  static QSet<library_song_id_t> getLibIds(const QByteArray& payload);

  /**
   * \brief Given JSON, this fuciton extracts QSet of library ids. The JSON
   * is assumed to be a simple array with each element being a library id.
   *
   * \brief payload JSON to convert.
   */
  static QSet<library_song_id_t> convertLibIdArray(const QByteArray& payload);


  /**
   * \brief Get's the id of a player from the given server reply.
   *
   * @param reply The reply from the server.
   * @return The player id in the servers response.
   */
  static player_id_t getPlayerId(QNetworkReply *reply);

  /**
   * \brief Gets the active playlist from the JSON given in the server reply.
   *
   * \param reply The reply from the server.
   * \return A QVariantMap representing the playlist given in the server reply.
   */
  static QVariantMap getActivePlaylistFromJSON(QNetworkReply *reply);

  /**
   * \brief Gets the list of participants from the JSON given in the server reply.
   *
   * \param reply The reply from the server.
   * \return A QVariantMap representing the participants given in the server reply.
   */
  static QVariantList getParticipantListFromJSON(QNetworkReply *reply);

  static QVariantList getSortingAlgosFromJSON(QNetworkReply *reply);

  static QVariantList generalListParse(QNetworkReply *reply, const QString& errorMsg);

  /**
   * \brief Gets the auth data from a server authentication reply.
   *
   * \param reply The reply from the server.
   * \param success A boolean that will be set to true or false depending on wether or not the 
   * JSON was succesfully created.
   * \return A QVariantMap representing the auth data retreived from the server.
   */
  static const QVariantMap getAuthReplyFromJSON(QNetworkReply *reply, bool &success);

  //@}

};


} //end namespace UDJ
#endif //JSON_HELPER_HPP
