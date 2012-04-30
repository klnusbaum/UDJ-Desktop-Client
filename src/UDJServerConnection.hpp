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
#ifndef UDJ_SERVER_CONNECTION_HPP
#define UDJ_SERVER_CONNECTION_HPP

#include "ConfigDefs.hpp"
#include "CommErrorHandler.hpp"
#include <QSqlDatabase>
#include <QDateTime>
#include <QObject>
#include <vector>
#include <QNetworkRequest>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkCookieJar;

namespace UDJ{


/**
 * Set's up a connection to the UDJ server and facilitates all communication.
 * with the server.
 */
class UDJServerConnection : public QObject{
Q_OBJECT
public:

  /** @name Constructor(s) and Destructor */
  //@{

  /**
   * \brief Constructs a UDJServerConnection.
   */
  UDJServerConnection(QObject *parent=NULL);


  //@}

  /** @name Connection Controls */
  //@{

  /**
   * \brief Perform authentication with the server.
   *
   * @param username The username.
   * @param password The password.
   */
  void authenticate(const QString& username, const QString& password);

  inline void setTicket(const QByteArray& ticket){
    ticket_hash = ticket;
  }

  inline void setUserId(const user_id_t& userId){
    user_id = userId;
  }

  inline void setPlayerId(const player_id_t& newPlayerId){
    playerId = newPlayerId;
  }

  void setPlayerActive();

  void setPlayerInactive();

  //@}


public slots:

  /** @name Slots */
  //@{

  void modLibContents(const QVariantList& songsToAdd, const QVariantList& songsToDelete);

  //void addLibSongsToServer(const QVariantList& songs);

  /**
   * \brief Deletes a song from the library on the server.
   *
   * @param toDeleteId The host id of the song to delete from the library on the
   * server.
   */
  //void deleteLibSongOnServer(library_song_id_t toDeleteId);

  /**
   * \brief Creates an event on the server.
   *
   * @param eventName The name of the event.
   * @param password The password of the event.
   */
  void createPlayer(
    const QString& playerName,
    const QString& password);

  void createPlayer(
    const QString& playerName,
    const QString& password,
    const QString& streetAddress,
    const QString& city,
    const QString& state,
    const int& zipcode);

  void createPlayer(const QByteArray& payload);

  /**
   * \brief Retrieves the latest version of the active playlist from the server.
   */
  void getActivePlaylist();

  /**
   * \brief Adds the given song to the active playlist on the server.
   *
   * @param requestId The request id of this add request.
   * @param songId Id of the song to be added to the active playlist.
   */
  void addSongToActivePlaylist(library_song_id_t songId);

  /**
   * \brief Removes the given songs from the active playlist on the server.
   *
   * @param playlistIds The ids of the playlist entries that should be remove.
   */
  void removeSongsFromActivePlaylist(
    const std::vector<playlist_song_id_t>& playlistIds);

  /**
   * \brief Set's the current song that the host is playing on the server.
   *
   * @param currentSong Id The current song that the host is playing.
   */
  void setCurrentSong(playlist_song_id_t currentSong);

  void setCurrentSong(const QByteArray& payload);

  //@}

signals:

  /** @name Signals */
  //@{

  /**
   * \brief Emitted when a connection with the server has been established.
   */
  void authenticated(const QByteArray& ticketHash, const user_id_t& userId);

  /**
   * \brief Emitted when there was a failure to establish a connection with the
   * server.
   *
   * @param errMessage A message describing the error.
   */
  void authFailed(const QString errMessage);

  void commError(
    CommErrorHandler::OperationType opType,
    CommErrorHandler::CommErrorType error,
    const QByteArray &payload);

  void playerSetActive();

  void playerSetInactive();

  void libSongsSyncedToServer(const std::vector<library_song_id_t>& syncedIds);

  void libModError(const QString& errMessage);


  /**
   * \brief Emitted when songs are added to the library on the server.
   *
   * @param addedIds Ids of the songs that were added to the library on the
   * server.
   */
  void songsAddedToLibOnServer(const std::vector<library_song_id_t> addedIds);

  /**
   * \brief Emitted when a song is deleted from the library on the server.
   *
   * @param deletedId Id of the song that was deleted from the library on the
   * server.
   */
  void songDeletedFromLibOnServer(
    const library_song_id_t deletedId);

  /**
   * \brief Emitted when an event is succesfully created.
   */
  void playerCreated(const player_id_t& issuedId);

  /**
   * \brief Emitted when a new version of the active playlist is retrieved from
   * the server.
   */
  void newActivePlaylist(const QVariantList newPlaylist);

  /**
   * \brief emitted when songs are succesfully added to the active playlist 
   * for an event on the server.
   *
   * @param ids the client request ids of the songs that were succesfully added
   * to the active playlist for an event on the server.
   */
  //void songsAddedToActivePlaylist(const std::vector<client_request_id_t> ids);

  void songAddedToActivePlaylist(const library_song_id_t& libId);

  /**
   * \brief emitted when a song is succesfully remove from the active playlist 
   * for an event on the server.
   *
   * @param songId Playlist id of song that was succesfully removed
   * from the active playlist for an event on the server.
   */
  void songRemovedFromActivePlaylist(const playlist_song_id_t songId);

  /**
   * \brief Emitted when the current song that the host is playing is
   * succesfully set on the server.
   */
  void currentSongSet();

  /**
   * \brief Emitted when there in a error setting host is playing on the server.
   */
  void currentSongSetError();


  void newEventGoers(QVariantList eventGoers);

  //@}


private slots:

  /** @name Private Slots */
  //@{

  /**
   * \brief Handles a reply from the server.
   *
   * @param reply The reply from the server.
   */
  void recievedReply(QNetworkReply *reply);

  //@}


private:
  /** @name Private Members */
  //@{

  /** \brief Id of the player associated with this conneciton */
  player_id_t playerId;

  /** \brief Ticket hash that should be used for all requests. */
  QByteArray ticket_hash;

  /** \brief Id of the user that is currently logged in. */
  user_id_t  user_id;

  /** \brief Manager for access to the network. */
  QNetworkAccessManager *netAccessManager;

  /**
   * \brief Time at which the current ticket has being used was issued by the
   * server.
   */
  QDateTime timeTicketIssued;

  /**
   * \brief Prepares a network request that is going to include JSON.
   *
   * @param request Request to prepare.
   */
  void prepareJSONRequest(QNetworkRequest &request);


  //@}

  /** @name Private Function */
  //@{

  QUrl getLibModUrl() const;

  /**
   * \brief Get the url to be used for adding songs to the library on the 
   * server.
   *
   * @return The url to be used for adding songs to the library on the 
   * server.
   */
  QUrl getLibAddSongUrl() const;

  /**
   * \brief Get the url to be used for removing a song from the library on the
   * server.
   *
   * @param toDelete The id of the song to delete from the library on the
   * server.
   * @return The url to be used for adding songs to the library on the
   * server.
   */
  QUrl getLibDeleteSongUrl(library_song_id_t toDelete) const;

  /**
   * \brief Get the url to be used for ending an event.
   *
   * @return The url to be used for ending an event.
   */
  QUrl getEndEventUrl() const;

  /**
   * \brief Get the url for retreiving the active playlist from the server.
   *
   * @return The for retreiving the active playlist from the server.
   */
  QUrl getActivePlaylistUrl() const;

  /**
   * \brief Get the url for adding songs to the active playlist on the server.
   *
   * @return The url for adding songs to the active playlist on the server.
   */
  QUrl getActivePlaylistAddUrl(const library_song_id_t& libId) const;

  /**
   * \brief Get the url for removing a song from the active playlist on the 
   * server.
   *
   * @return The url for removing a song from the active playlist on the 
   * server.
   */
  QUrl getActivePlaylistRemoveUrl(playlist_song_id_t toDelete) const;

  /**
   * \brief Get the url to be used for setting the current song on the server.
   *
   * @return The url to be used for setting the current song on the server.
   */
  QUrl getCurrentSongUrl() const;

  QUrl getUsersUrl() const;

  QUrl getCreatePlayerUrl() const;

  QUrl getPlayerStateUrl() const;

  /**
   * \brief Determines whether or not a url path is a path which can be used
   * for deleting a song from the library on the server.
   *
   * @param path The path whose identity is in question.
   * @return True if the url path is one which can be used for deleting a song
   * from the libray on the server. False otherwise.
   */
  bool isLibDeleteUrl(const QString& path) const;

  /**
   * \brief Determines whether or not a url path is a path which can be used
   * for deleting a song from the active playlist on the server.
   *
   * @param path The path whose identity is in question.
   * @return True if the url path is one which can be used for deleting a song
   * from the acitve playlist on the server. False otherwise.
   */
  bool isActivePlaylistRemoveUrl(const QString& path) const;

  bool isActivePlaylistAddUrl(const QString& path) const;

  bool isPlayerCreateUrl(const QString& path) const;

  bool isSetActiveReply(const QNetworkReply *reply) const;

  bool isSetInactiveReply(const QNetworkReply *reply) const;



  /**
   * \brief Get the port number to be used when communicating with the server.
   *
   * This port number is a memorial to Keith Nusbaum, my father. I loved him
   * deeply and he was taken from this world far too soon. Never-the-less
   * we all continue to benefit from his good deeds. Without him, I wouldn't
   * be here, and there would be no UDJ. Please, don't change this port
   * number. Keep the memory of my father alive.
   * K = 10 % 10 = 0
   * e = 4  % 10 = 4
   * i = 8  % 10 = 8
   * t = 19 % 10 = 9
   * h = 7  % 10 = 7
   * Port 4897, the Keith Nusbaum Memorial Port
   *
   * @return The port number to be used for communicating with the server.
   */
  static const QString & getServerPortNumber(){
    static const QString serverPortNumber = "4898";
    return serverPortNumber;
  }

  /**
   * \brief Gets the url path to the server in string form.
   *
   * @return The url path to the server in string form.
   */
  static const QString& getServerUrlPath(){
    static const QString SERVER_URL_PATH= 
      "https://udjplayer.com:" + getServerPortNumber() + "/udj/";
    return SERVER_URL_PATH;
  }

  /**
   * \brief Gets the url path to the server in URL form.
   *
   * @return The url path to the server in URL form.
   */
  static const QUrl& getServerUrl(){
    static const QUrl SERVER_URL(getServerUrlPath());
    return SERVER_URL;
  }

  /**
   * \brief Gets the url for authenticating with the server.
   *
   * @return The url for authenticating with the server.
   */
  static const QUrl& getAuthUrl(){
    static const QUrl AUTH_URL(getServerUrlPath() + "auth");
    return AUTH_URL;
  }

  /**
   * \brief Get the header used for identifying the ticket hash header.
   *
   * @return The header used for identifying the ticket hash header.
   */
  static const QByteArray& getTicketHeaderName(){
    static const QByteArray ticketHeaderName = "X-Udj-Ticket-Hash";
    return ticketHeaderName;
  }

  static const QByteArray& getGoneResourceHeaderName(){
    static const QByteArray goneResourceHeaderName = "X-Udj-Gone-Resource";
    return goneResourceHeaderName;
  }

  static const char* getPayloadPropertyName(){
    static const char* payloadPropertyName = "payload";
    return payloadPropertyName;
  }

  static const char* getSongsAddedPropertyName(){
    static const char* songsAddedPropertyName = "songs_added";
    return songsAddedPropertyName;
  }

  static const char* getSongsDeletedPropertyName(){
    static const char* songsDeletedPropertyName = "songs_deleted";
    return songsDeletedPropertyName;
  }




  /**
   * \brief Handle a response from the server regarding authentication.
   *
   * @param reply Response from the server.
   */
  void handleAuthReply(QNetworkReply* reply);

  void handleSetActiveReply(QNetworkReply* reply);

  void handleSetInactiveReply(QNetworkReply* reply);

  void handleRecievedLibMod(QNetworkReply *reply);

  /**
   * \brief Handle a response from the server regarding adding songs to the
   * library.
   *
   * @param reply Response from the server.
   */
  //void handleAddLibSongsReply(QNetworkReply *reply);

  /**
   * \brief Handle a response from the server regarding deleiting songs from the
   * library.
   *
   * @param reply Response from the server.
   */
  //void handleDeleteLibSongsReply(QNetworkReply *reply);

  /**
   * \brief Handle a response from the server regarding player creation.
   *
   * @param reply Response from the server.
   */
  void handleCreatePlayerReply(QNetworkReply *reply);

  /**
   * \brief Handle a response from the server regarding a new active playlist.
   *
   * @param reply Response from the server.
   */
  void handleRecievedActivePlaylist(QNetworkReply *reply);

  /**
   * \brief Handle a response from the server regarding the addition of a song
   * to the active playlist.
   *
   * @param reply Response from the server.
   */
  void handleRecievedActivePlaylistAdd(QNetworkReply *reply);

  /**
   * \brief Handle a response from the server regarding the removal of a song 
   * from the active playlist.
   *
   * @param reply Response from the server.
   */
  void handleRecievedActivePlaylistRemove(QNetworkReply *reply);

  /**
   * \brief Handle a response from the server regarding the setting of the 
   * current song that is being played.
   *
   * @param reply Response from the server.
   */
  void handleRecievedCurrentSongSet(QNetworkReply *reply);

  void handleLocaitonResponse(QNetworkReply *reply);

  void parseLocationResponse(QNetworkReply *reply);

  bool checkReplyAndFireErrors(
    QNetworkReply *reply,
    CommErrorHandler::OperationType opType);

  static bool isResponseType(QNetworkReply *reply, int code);

  //@}


};


} //end namespace
#endif //UDJ_SERVER_CONNECTION_HPP
