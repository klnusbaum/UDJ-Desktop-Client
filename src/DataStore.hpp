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
#ifndef DATA_STORE_HPP
#define DATA_STORE_HPP
#include <QSqlDatabase>
#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include <QProgressDialog>
#include <QSettings>
#include "ConfigDefs.hpp"
#include <QNetworkReply>

class QTimer;

namespace UDJ{

class UDJServerConnection;

/** 
 * \brief A class that provides access to all persisten storage used by UDJ.
 */
class DataStore : public QObject{
Q_OBJECT
public:

  enum ReauthAction{
    SYNC_LIB,
    GET_ACTIVE_PLAYLIST,
    SET_CURRENT_SONG,
    MOD_PLAYLIST
  };

  /** @name Constructor(s) and Destructor */
  //@{

  /** \brief Constructs a DataStore
   *
   * @param serverConnection Connection to the UDJ server.
   * @param parent The parent widget.
   */
  DataStore(
    const QString& username,
    const QString& password,
    const QByteArray& ticketHash, 
    const user_id_t& userId, 
    QObject *parent=0);

  //@}

  /** @name Getters and Setters */
  //@{

  void addMusicToLibrary(
    const QList<Phonon::MediaSource>& songs, 
    QProgressDialog* progress=0);

  void activatePlayer();

  void deactivatePlayer();

  /**
   * \brief Removes the given songs from the music library. 
   *
   * @param toRemove The list of songs to be removed from the library.
   */
  void removeSongsFromLibrary(const QSet<library_song_id_t>& toRemove);

  /**
   * \brief Given a media source, determines the song name from the current
   * model data.
   *
   * @param source Source whose song name is desired.
   * @return The name of the song contained in the given source according
   * to current model data. If the source could not be found in the model
   * and emptry string is returned.
   */
  QString getSongNameFromSource(const Phonon::MediaSource &source) const;

  /**
   * \brief Gets the raw connection to the actual database that the DataStore
   * uses.
   *
   * @return The connection to the database backing the DataStore.
   */
  QSqlDatabase getDatabaseConnection();

  /**
   * \brief Adds any songs to the server for which the
   * host client doesn't have valid server_lib_song_id.
   */
  void syncLibrary();

  /**
   * \brief Syncs all the requests for additions to the active playlst.
   */
  //void syncPlaylistAddRequests();

  /**
   * \brief Syncs all the requests for removals from the active playlst.
   */
  void syncPlaylistRemoveRequests();


  /**
   * \brief Gets the name of the current event.
   *
   * @return The name of the current event.
   */
  inline const QString getPlayerName() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getPlayerNameSettingName()).toString();
  }

  /**
   * \brief Gets the id of the current event.
   *
   * @return The id of the current event.
   */
  inline const player_id_t getPlayerId() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getPlayerIdSettingName()).value<player_id_t>();
  }

  inline const QString& getUsername() const{
    return username;
  }

  inline const QString& getPassword() const{
    return password;
  }

  /**
   * \brief Retrieves the next song should be played but does not
   * remove it from the active playlist.
   *
   * @return The next song that is going to be played.
   */
  Phonon::MediaSource getNextSongToPlay();

  /**
   * \brief Retrieves the next song that should be played and removes it
   * from the active playlist.
   *
   * @return The next song that should be played.
   */
  Phonon::MediaSource takeNextSongToPlay();

  const QString getPlayerState() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getPlayerStateSettingName()).toString();
  }

  const bool isCurrentlyActive() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return 
      settings.value(getPlayerStateSettingName()).toString()
      ==
      getPlayerActiveState();
  }

  static void saveCredentials(const QString& username, const QString& password);

  static void setCredentialsDirty();

  static bool hasValidSavedCredentials();

  static void getSavedCredentials(QString* username, QString* password);

  static void clearSavedCredentials();

  //@}


  /** @name Public Constants */
  //@{

  /**
   * \brief When a song title can't be found, this title should be used instead.
   */
  static const QString& unknownSongTitle(){
    static const QString unknownSongTitle = tr("Unknown");
    return unknownSongTitle;
  }

  /**
   * \brief When a song artist can't be found, this artist should be used instead.
   */
  static const QString& unknownSongArtist(){
    static const QString unknownSongArtist = tr("Unknown");
    return unknownSongArtist;
  }

  /**
   * \brief When a song album can't be found, this album should be used instead.
   */
  static const QString& unknownSongAlbum(){
    static const QString unknownSongAlbum = tr("Unknown");
    return unknownSongAlbum;
  }

  static const QString& unknownGenre(){
    static const QString unknownGenre = tr("Unknown");
    return unknownGenre;
  }

  /**
   * \brief Gets the name of the table in the musicdb that contains information
   * about the music library associated with the server conneciton.
   *
   * @return The name of the table in the musicdb that contains information
   * about the music library associated with the server connection.
   */
  static const QString& getLibraryTableName(){
    static const QString libraryTableName = "library";
    return libraryTableName;
  }

  /**
   * \brief Gets name of the table storing the active playlist.
   *
   * @return The name of the table containing the active playlist.
   */
  static const QString& getActivePlaylistTableName(){
    static const QString activePlaylistTableName = "active_playlist";
    return activePlaylistTableName;
  }

  /**
   * \brief Gets name of the view containing the active playlist joined with
   * the library table.
   *
   * @return The view containing the active playlist joined with the library
   * table.
   */
  static const QString& getActivePlaylistViewName(){
    static const QString activePlaylistViewName = "active_playlist_view";
    return activePlaylistViewName;
  }

  /**
   * \brief Gets the name of the id column in the active playlist table.
   *
   * @return The name of the id colum in the active playlist table.
   */
  static const QString& getActivePlaylistIdColName(){
    static const QString activePlaylistIdColName = "id";
    return activePlaylistIdColName;
  }

  /** 
   * \brief Gets the name of the library id column (the column that specifies
   * which library entry this playlist entry corresponds with) in the active
   * playlist table.
   *
   * @return The name of the library id column  in the active playlist table.
   */
  static const QString& getActivePlaylistLibIdColName(){
    static const QString activePlaylistLibIdColName = "lib_id";
    return activePlaylistLibIdColName;
  }

  /** 
   * \brief Gets the name of the column in the active playlist view that 
   * contains the vote count.
   *
   * @return The name of the column in the active playlist view that 
   * contains the vote count.
   */
  static const QString& getVoteCountColName(){
    static const QString voteCountColName = "vote_count";
    return voteCountColName;
  }

  /** 
   * \brief Gets the name of the time added column in the active playlist table.
   *
   * @return The name of the time added column in the active playlist table.
   */
  static const QString& getTimeAddedColName(){
    static const QString timeAddedColName = "time_added";
    return timeAddedColName;
  }

  /** 
   * \brief Gets the name of the priority column in the active playlist table.
   *
   * @return The name of the priority column in the active playlist table.
   */
  static const QString& getPriorityColName(){
    static const QString priorityColName = "priority";
    return priorityColName;
  }

  /** 
   * \brief Gets the name of the adder id column in the active playlist table.
   *
   * @return The name of the adder id column in the active playlist table.
   */
  static const QString& getAdderIdColName(){
    static const QString adderIdColName = "adderId";
    return adderIdColName;
  }

  static const QString& getAdderUsernameColName(){
    static const QString adderUsernameColName = "adder_username";
    return adderUsernameColName;
  }

  /** 
   * \brief Gets the name of the upvote column in the active playlist table.
   *
   * @return The name of the upvote column in the active playlist table.
   */
  static const QString& getUpVoteColName(){
    static const QString upVoteColName = "up_votes";
    return upVoteColName;
  }

  /** 
   * \brief Gets the name of the downvote column in the active playlist table.
   *
   * @return The name of the downvote column in the active playlist table.
   */
  static const QString& getDownVoteColName(){
    static const QString downVoteColName = "down_votes";
    return downVoteColName;
  }

  /** 
   * \brief Gets the id column in the library table table.
   *
   * @return The name of the id column in the library table.
   */
  static const QString& getLibIdColName(){
    static const QString libIdColName = "id";
    return libIdColName;
  }

  /** 
   * \brief Gets the song column in the library table table.
   *
   * @return The name of the song column in the library table.
   */
  static const QString& getLibSongColName(){
    static const QString libSongColName = "Song";
    return libSongColName;
  }

  /** 
   * \brief Gets the artist column in the library table table.
   *
   * @return The name of the artist column in the library table.
   */
  static const QString& getLibArtistColName(){
    static const QString libArtistColName = "Artist";
    return libArtistColName;
  }

  /**
   * \brief Gets the album column in the library table table.
   *
   * @return The name of the album column in the library table.
   */
  static const QString& getLibAlbumColName(){
    static const QString libAlbumColName = "Album";
    return libAlbumColName;
  }

  /** 
   * \brief Gets the file column in the library table table.
   *
   * @return The name of the file column in the library table.
   */
  static const QString& getLibFileColName(){
    static const QString libFileColName = "File";
    return libFileColName;
  }

  /** 
   * \brief Gets the duration column in the library table table.
   *
   * @return The name of the duration column in the library table.
   */
  static const QString& getLibDurationColName(){
    static const QString libDurationColName = "Length";
    return libDurationColName;
  }

  static const QString& getLibGenreColName(){
    static const QString libGenreColName = "Genre";
    return libGenreColName;
  }

  static const QString& getLibTrackColName(){
    static const QString libTrackColName = "Track";
    return libTrackColName;
  }

  /** 
   * \brief Gets the is deleted column in the library table table.
   *
   * @return The name of the is deleted column in the library table.
   */
  static const QString& getLibIsDeletedColName(){
    static const QString libIsDeletedColName = "is_deleted";
    return libIsDeletedColName;
  }

  static const QString& getLibIsBannedColName(){
    static const QString libIsBannedColName = "is_banned";
    return libIsBannedColName;
  }

  /**
   * \brief Gets the sycn status column in the library table table.
   *
   * @return The name of the sycn status column in the library table.
   */
  static const QString& getLibSyncStatusColName(){
    static const QString libSyncStatusColName = "sync_status";
    return libSyncStatusColName;
  }

  /** 
   * \brief Gets the value for the "needs add" sync status used in the library
   * table.
   *
   * @return The value for the "needs add" sync status used in the library
   * table.
   */
  static const lib_sync_status_t& getLibNeedsAddSyncStatus(){
    static const lib_sync_status_t libNeedsAddSyncStatus = 1;
    return libNeedsAddSyncStatus;
  }

  /** 
   * \brief Gets the value for the "needs delete" sync status used in the 
   * library table.
   *
   * @return The value for the "needs delete" sync status used in the library
   * table.
   */
  static const lib_sync_status_t& getLibNeedsDeleteSyncStatus(){
    static const lib_sync_status_t libNeedsDeleteSyncStatus = 2;
    return libNeedsDeleteSyncStatus;
  }

  static const lib_sync_status_t& getLibNeedsBanSyncStatus(){
    static const lib_sync_status_t libNeedsBanStatus = 3;
    return libNeedsBanStatus;
  }

  /**
   * \brief Gets the value for the "is synced" sync status used in the library
   * table.
   *
   * @return The value for the "is synced" sync status used in the library
   * table.
   */
  static const lib_sync_status_t& getLibIsSyncedStatus(){
    static const lib_sync_status_t libIsSyncedStatus = 0;
    return libIsSyncedStatus;
  }

  static const QString& getLibIdAlias(){
    static const QString libIdAlias = "libIdAlias";
    return libIdAlias;
  }

  static const QString& getPlayerIdSettingName(){
    static const QString playerIdSetting = "playerId";
    return playerIdSetting;
  }

  static const QString& getPlayerNameSettingName(){
    static const QString playerIdSetting = "playerName";
    return playerIdSetting;
  }

  static const QString& getPlayerStateSettingName(){
    static const QString playerStateSettingName = "playerState";
    return playerStateSettingName;
  }

  static const QString& getNoPlayerState(){
    static const QString noPlayerState = "noPlayer";
    return noPlayerState;
  }

  static const QString& getPlayerActiveState(){
    static const QString playerActiveState = "playerActive";
    return playerActiveState;
  }

  static const QString& getPlayerInactiveState(){
    static const QString playerInactiveState = "playerInactive";
    return playerInactiveState;
  }

  static const QString& getSettingsOrg(){
    static const QString settingsOrg = "Bazaar Solutions";
    return settingsOrg;
  }

  static const QString& getSettingsApp(){
    static const QString settingsApp = "UDJ";
    return settingsApp;
  }

 //@}

/** @name Public slots */
//@{
public slots:

  /**
   * \brief Refresh the active playlist table.
   */
  void refreshActivePlaylist();

  void addSongToActivePlaylist(library_song_id_t libraryId);

  void addSongsToActivePlaylist(const QSet<library_song_id_t>& libIds);

  void removeSongsFromActivePlaylist(const QSet<library_song_id_t>& libraryIds);

  /** 
   * \brief Creates a new player with the given name and password.
   *
   * @param name The name of the player.
   * @param password The password for the event (maybe empty).
   */
  void createNewPlayer(
    const QString& name,
    const QString& password);

  void createNewPlayer(
    const QString& name,
    const QString& password,
    const QString& streetAddress,
    const QString& city,
    const QString& state,
    const int& zipcode);

  /** 
   * \brief Sets the current song to the speicified song.
   *
   * @param songToPlay The playlist id of the song to be played.
   */
  void setCurrentSong(const library_song_id_t& songToPlay);

  //@}

signals:

/** @name Signals */
//@{

  /**
   * \brief Emitted when no player exists and needs to be created
   */
  void needPlayerCreate();

  /**
   * \brief Emitted when the library table is modified.
   */
  void libSongsModified();


  /**
   * \brief Emitted when a player is created.
   */
  void playerCreated();

  /**
   * \brief Emitted when the creation of a player fails.
   *
   * @param errMessage Error message describing what happened.
   */
  void playerCreationFailed(const QString errMessage);

  /**
   * \brief Emitted when the active playlist is modified.
   */
  void activePlaylistModified();

  /**
   * \brief Emitted when the current song is manually changed.
   *
   * @param newSong The song that should be set as the current song.
   */
  void manualSongChange(Phonon::MediaSource newSong);

  void playerActive();

  void playerDeactivated();

//@}

private:

  /** @name Private Members */
  //@{

  /** \brief Connection to the UDJ server */
  UDJServerConnection *serverConnection;

  /** \brief Actual database connection */
  QSqlDatabase database;

  /** \brief Timer used to refresh the active playlist. */
  QTimer *activePlaylistRefreshTimer;

  QString username;

  QString password;

  QSet<ReauthAction> reauthActions;

  bool isReauthing;

  library_song_id_t currentSongId;

  QSet<library_song_id_t> playlistIdsToAdd;

  QSet<library_song_id_t> playlistIdsToRemove;

  //@}

  /** @name Private Functions */
  //@{

  /** \brief Does initiail database setup */
  void setupDB();

  /**
   * \brief Deletes all the entries in the active playlist table.
   */
  void clearActivePlaylist();

  void initReauth();

  void doReauthAction(const ReauthAction& action);

  /**
   * \brief Adds a single song to the music library.
   *
   * @param song Song to be added to the library.
   */
  void addSongToLibrary(const Phonon::MediaSource& song);


  //@}

  /** @name Private Constants */
  //@{
  /**
   * \brief Retrieves the name of the connection to the playerdb.
   *
   * @return The name of the connection to the playerdb.
   */
  static const QString& getPlayerDBConnectionName(){
    static const QString playerDBConnectionName("playerdbConn");
    return playerDBConnectionName;
  }

  /**
   * \brief Retrieves the name of the player database.
   *
   * @return The name of the the player database.
   */
  static const QString& getPlayerDBName(){
    static const QString playerDBName("playerdb");
    return playerDBName;
  }

  /** 
   * \brief Gets the query used to create the library table.
   *
   * @return The query used to create the library table.
   */
  static const QString& getCreateLibraryQuery(){
    static const QString createLibQuery =
      "CREATE TABLE IF NOT EXISTS " +
      getLibraryTableName() +
      "(" + getLibIdColName() + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
      getLibSongColName() + " TEXT NOT NULL, " +
      getLibArtistColName() + " TEXT NOT NULL, "+
      getLibAlbumColName() + " TEXT NOT NULL, " +
      getLibGenreColName() + " TEXT NOT NULL, " +
      getLibTrackColName() + " INTEGER NOT NULL, " +
      getLibFileColName() + " TEXT NOT NULL, " +
      getLibDurationColName() + " INTEGER NOT NULL, " +
      getLibIsDeletedColName() + " INTEGER DEFAULT 0, " +
      getLibIsBannedColName() + " INTEGER DEFAULT 0, " +
      getLibSyncStatusColName() + " INTEGER DEFAULT " +
        QString::number(getLibNeedsAddSyncStatus()) + " " +
      "CHECK("+
        getLibSyncStatusColName()+"="+
          QString::number(getLibIsSyncedStatus()) +" OR " +
        getLibSyncStatusColName()+"="+
          QString::number(getLibNeedsAddSyncStatus()) +" OR " +
        getLibSyncStatusColName()+"="+
          QString::number(getLibNeedsDeleteSyncStatus()) +" OR " +
        getLibSyncStatusColName()+"="+
          QString::number(getLibNeedsBanSyncStatus()) +
      "));";

    return createLibQuery;
  }

  /**
   * \brief Gets the query used to create the active playlist table.
   *
   * @return The query used to create the active playlist table.
   */
  static const QString& getCreateActivePlaylistQuery(){
    static const QString createActivePlaylistQuery =
      "CREATE TABLE IF NOT EXISTS " +
      getActivePlaylistTableName() +  "(" +
      getActivePlaylistIdColName() + " INTEGER PRIMARY KEY, " +
      getActivePlaylistLibIdColName() + " INTEGER REFERENCES " +
      getLibraryTableName() +"(" + getLibIdColName()+ ") ON DELETE CASCADE, "+
      getDownVoteColName() + " INTEGER NOT NULL, " +
      getUpVoteColName() + " INTEGER NOT NULL, " +
      getPriorityColName() + " INTEGER NOT NULL, " +
      getAdderIdColName() + " INTEGER NOT NULL, " +
      getAdderUsernameColName() + " TEXT NOT NULL, " +
      getTimeAddedColName() + " TEXT DEFAULT CURRENT_TIMESTAMP);";
    return createActivePlaylistQuery;
  }

  /** 
   * \brief Gets the query used to create the active playlist view (
   * a join between the active playlist and the library table).
   *
   * @return The query used to create the active playlist view.
   */
  static const QString& getCreateActivePlaylistViewQuery(){
    static const QString createActivePlaylistViewQuery = 
      "CREATE VIEW IF NOT EXISTS "+getActivePlaylistViewName() + " " + 
      "AS SELECT " +
      getActivePlaylistTableName() + "." + getActivePlaylistIdColName() + "," +
      getActivePlaylistTableName() + "." + 
      getActivePlaylistLibIdColName() + "," +
      getLibraryTableName() + "." + getLibSongColName() + "," +
      getLibraryTableName() + "." + getLibFileColName() + "," +
      getLibraryTableName() + "." + getLibArtistColName() + "," +
      getLibraryTableName() + "." + getLibAlbumColName() + "," +
      getActivePlaylistTableName() + "." + getUpVoteColName() + "," +
      getActivePlaylistTableName() + "." + getDownVoteColName() + "," +
      getLibraryTableName() + "." + getLibDurationColName() + "," +
      getActivePlaylistTableName() + "." + getAdderIdColName() + "," +
      getActivePlaylistTableName() + "." + getAdderUsernameColName() + "," +
      getActivePlaylistTableName() + "." + getTimeAddedColName() + "," +
      getLibraryTableName() + "." + getLibIdColName() + " AS " + getLibIdAlias() + " " +
      "FROM " + getActivePlaylistTableName() + " INNER JOIN " +
      getLibraryTableName() + " ON " + getActivePlaylistTableName() + "." +
      getActivePlaylistLibIdColName() + "=" + getLibraryTableName() + "." +
      getLibIdColName() +" "
      "ORDER BY " +getPriorityColName() + " ASC;";
    return createActivePlaylistViewQuery;
  }

  /**
   * \brief Gets the query used to delete all entries in the active playlist
   * table.
   *
   * @return The query used to delete all entries in the active playlist
   * table.
   */
  static const QString& getClearActivePlaylistQuery(){
    static const QString clearActivePlaylistQuery = 
      "DELETE FROM " + getActivePlaylistTableName() + ";";
    return clearActivePlaylistQuery;
  }

  static const QString& getUsernameSettingName(){
    static const QString usernameSettingName = "username";
    return usernameSettingName;
  }

  static const QString& getPasswordSettingName(){
    static const QString passwordSettingName = "password";
    return passwordSettingName;
  }


  static const QString& getHasValidCredsSettingName(){
    static const QString hasValidSavedCredentialsSettingName = 
      "has_valid_creds";
    return hasValidSavedCredentialsSettingName;
  }

  static const QString& getMachineUUIDSettingName(){
    static const QString machineUUIDSettingName = "machine_uuid";
    return machineUUIDSettingName;
  }

 //@}

/** @name Private Slots */
//@{
private slots:

  /**
   * \brief Sets the sync status of a library song to synced.
   *
   * @param song The id of the song whose sync status should be set to synced.
   */
  void setLibSongSynced(library_song_id_t song);

  /**
   * \brief Sets the sync status of the given library songs to synced.
   *
   * @param songs The ids of the songs whose sync status should be set 
   * to synced.
   */
  void setLibSongsSynced(const std::vector<library_song_id_t> songs);

  /**
   * \brief Sets the sync status of the given library songs to the given
   * given sync status.
   *
   * @param songs The ids of the songs whose sync status should be set.
   * @param syncStatus The sync status to which the given songs should be set. 
   */
  void setLibSongsSyncStatus(
    const std::vector<library_song_id_t> songs,
    const lib_sync_status_t syncStatus);


  void addSong2ActivePlaylistFromQVariant(const QVariantMap &songToAdd, int priority);
  /**
   * \brief Sets the active playlist to the given songs.
   *
   * @param newSongs The new songs which should populate the active playlist.
   */
  void setActivePlaylist(const QVariantMap& newSongs);

  void onGetActivePlaylistFail(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);


  void onPlayerCreate(const player_id_t& issuedId);

  void onPlayerCreationFailed(const QString& errMessage, int errorCode,
          const QList<QNetworkReply::RawHeaderPair>& headers);

  void onPlayerSetActive();

  void onPlayerDeactivated();

  void onLibModError(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);

  void onSetCurrentSongFailed(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);

  void onReauth(const QByteArray& ticketHash, const user_id_t& userId);

  void onAuthFail(const QString& errMessage);

  void onActivePlaylistModified(
    const QSet<library_song_id_t>& added,
    const QSet<library_song_id_t>& removed);

  void onActivePlaylistModFailed(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);


  static QByteArray getHeaderValue(const QByteArray& headerName, const QList<QNetworkReply::RawHeaderPair>& headers);

  static inline bool isTicketAuthError(
      int errorCode,
      const QList<QNetworkReply::RawHeaderPair>& headers)
  {
    return errorCode==401 && getHeaderValue("WWW-Authenticate", headers) == "ticket-hash";
  }


//@}

};

} //end namespace
#endif //DATA_STORE_HPP
