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
#include <QSettings>
#include "ConfigDefs.hpp"
#include <QNetworkReply>
#include <QThread>

class QTimer;
class QProgressDialog;

namespace UDJ{

class UDJServerConnection;

/** 
 * \brief A class that provides access to all persistent/semi-persistent storage used by UDJ.
 */
class DataStore : public QObject{
Q_OBJECT
public:

  /** @name Public Typedefs and Enums */
  //@{

  /**
   * \brief Actions that can be preformed once the client has reauthenticated.
   */
  enum ReauthAction{
    SYNC_LIB,
    GET_ACTIVE_PLAYLIST,
    SET_CURRENT_SONG,
    MOD_PLAYLIST,
    SET_CURRENT_VOLUME
  };

  /**
   * \brief A minimal set of info describing a song in the database.
   */
  typedef struct {
    Phonon::MediaSource source;
    QString title;
    QString artist;
  } song_info_t;

  //@}


  /** @name Constructor(s) and Destructor */
  //@{

  /** \brief Constructs a DataStore
   *
   * @param username The username being used by the client.
   * @param password The password being used by the client.
   * @param ticketHash The tickethash being used for communication with the server.
   * @param userId Id of the user using this client.
   * @param parent The parent object.
   */
  DataStore(
    const QString& username,
    const QString& password,
    const QByteArray& ticketHash,
    const user_id_t& userId,
    QObject *parent=0);

  //@}

  /** @name Modifiers */
  //@{

  /**
   * \brief Adds a list of songs to the music library.
   *
   * @param songs The list of songs to be added to the library.
   * @param progress A progress dialog representing the progress of the 
   * of adding the songs to the library.
   */
  void addMusicToLibrary(
    const QList<Phonon::MediaSource>& songs, 
    QProgressDialog* progress=0);

  /**
   * \brief Sets the player's password
   *
   * \param newPassword The password that should be set for the player. Should be non-blank.
   */
  void setPlayerPassword(const QString& newPassword);

  /**
   * \brief Set player location.
   *
   * \param streetAddress The street address of the location to be set.
   * \param city The city of the location to be set.
   * \param state The state of the location to be set.
   * \param zipcode The zipcode of the location to be set.
   */
  void setPlayerLocation(
    const QString& streetAddress,
    const QString& city,
    const QString& state,
    int zipcode
  );

  /**
   * \brief Set player name.
   *
   * \param The new name the player should have.
   */
  void setPlayerName(const QString& newName);

  /**
   * \brief Set player state.
   *
   * \param State to which the player should be set.
   */
  void setPlayerState(const QString& newState);

  /**
   * \brief Removes the given songs from the music library. 
   *
   * @param toRemove A set of song ids to remove from the library.
   * @param progress A progress dialog representing the progress of the 
   * of removing the songs from the library.
   */
  void removeSongsFromLibrary(const QSet<library_song_id_t>& toRemove,
      QProgressDialog *progress=0);

  /**
   * \brief Gets the raw connection to the actual database that the DataStore
   * uses.
   *
   * @return The connection to the database backing the DataStore.
   */
  QSqlDatabase getDatabaseConnection();

  /**
   * \brief Gets the name of the player.
   *
   * @return The name of the player.
   */
  inline const QString getPlayerName() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getPlayerNameSettingName(), tr("Not Set")).toString();
  }

  /**
   * \brief Gets the id of the player.
   *
   * @return The id of the player.
   */
  inline const player_id_t getPlayerId() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getPlayerIdSettingName()).value<player_id_t>();
  }

  /**
   * \brief Gets the volume of the player.
   *
   * @return The volume of the player.
   */
  inline qreal getPlayerVolume() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getPlayerVolumeSettingName()).value<qreal>();
  }


  /**
   * \brief Gets the username being used by the client
   *
   * @return The username being used by the client.
   */
  inline const QString& getUsername() const{
    return username;
  }

  /**
   * \brief Gets the password being used by the user.
   *
   * @return The password being used by the user.
   */
  inline const QString& getPassword() const{
    return password;
  }

  /**
   * \brief Determines whether or not the player's location has been set.
   *
   * @return True if the player's location is set, false otherwise.
   */
  inline bool hasLocation() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.contains(getAddressSettingName());
  }

  /**
   * \brief Determines whether or not the player has a password.
   *
   * @return True if the player has a password, false otherwise.
   */
  inline bool hasPlayerPassword() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.contains(getPlayerPasswordSettingName());
  }

  /**
   * \brief Retreives the password for the player.
   *
   * @return The password for the player, if none is set a blank string is returned.
   */
  inline QString getPlayerPassword() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getPlayerPasswordSettingName()).toString();
  }

  /**
   * \brief Retrieves a string describing the location of the player.
   *
   * @return A string describing the locaiton of the player.
   */
  inline QString getLocationString() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return 
      settings.value(getAddressSettingName()).toString() + " " + 
      settings.value(getCitySettingName()).toString() + " " +
      settings.value(getStateSettingName()).toString() + ", " +
      settings.value(getZipCodeSettingName()).toString();
  }

  /**
   * \brief Gets the set location street address. If no location is currently set,
   * an empty string is returned.
   *
   * \return The set location street address or an empty string if no address is set.
   */
  inline QString getLocationStreetAddress() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getAddressSettingName()).toString();
  }

  /**
   * \brief Gets the set location city. If no location is currently set,
   * an empty string is returned.
   *
   * \return The set location city or an empty string if no address is set.
   */
  inline QString getLocationCity() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getCitySettingName()).toString();
  }

  /**
   * \brief Gets the set location state. If no location is currently set,
   * an empty string is returned.
   *
   * \return The set location state or an empty string if no address is set.
   */
  inline QString getLocationState() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getStateSettingName()).toString();
  }

  /**
   * \brief Gets the set location zipcode. If no zipcode is currently set,
   * 0 is returned.
   *
   * \return The set location zipcode or 0 if no address is set.
   */
  inline int getLocationZipcode() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getZipCodeSettingName()).toInt();
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
  song_info_t takeNextSongToPlay();

  /**
   * \brief Retrieves the state of the player.
   *
   * @return The state of the player.
   */
  const QString getPlayerState() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return settings.value(getPlayerStateSettingName()).toString();
  }

  /**
   * \brief Determines whether or not this player has a player id.
   *
   * \return True if the player has an id, false otherwise.
   */
  bool hasPlayerId() const{
    QSettings settings(
      QSettings::UserScope, getSettingsOrg(), getSettingsApp());
    return -1 != settings.value(getPlayerIdSettingName(), -1);
  }

  /**
   * \brief Saves the given credentials to persistent storage in a secure manner.
   *
   * @param username The username to save.
   * @param password The password to save.
   */
  static void saveCredentials(const QString& username, const QString& password);

  /**
   * \brief Marks the current saved credentials as invalid.
   */
  static void setCredentialsDirty();

  /**
   * \brief Determines whether or not the currently saved credentials are valid.
   *
   * @return True if the currently saved credentials are valide, false otherwise.
   */
  static bool hasValidSavedCredentials();

  /**
   * \brief Retrieves the currently saved credentials.
   *
   * @param username Pointer to the QString where the retreived username should be put.
   * @param password Pointer to the QString where the retreived password should be put.
   */
  static void getSavedCredentials(QString* username, QString* password);

  /**
   * \brief Deletes all the saved credentials.
   */
  static void clearSavedCredentials();

  //@}


  /** @name Public Constants */
  //@{

  /**
   * \brief When a song title can't be found, this title should be used instead.
   *
   * @return The song title to be used when no title is known.
   */
  static const QString& unknownSongTitle(){
    static const QString unknownSongTitle = tr("Unknown");
    return unknownSongTitle;
  }

  /**
   * \brief When a song artist can't be found, this artist should be used instead.
   *
   * @return The song artist to be used when no artist is known.
   */
  static const QString& unknownSongArtist(){
    static const QString unknownSongArtist = tr("Unknown");
    return unknownSongArtist;
  }

  /**
   * \brief When a song album can't be found, this album should be used instead.
   *
   * @return The song album to be used when no album is known.
   */
  static const QString& unknownSongAlbum(){
    static const QString unknownSongAlbum = tr("Unknown");
    return unknownSongAlbum;
  }

  /**
   * \brief When a song genre can't be found, this genre should be used instead.
   *
   * @return The song genre to be used when no genre is known.
   */
  static const QString& unknownGenre(){
    static const QString unknownGenre = tr("Unknown");
    return unknownGenre;
  }

  /**
   * \brief Gets the name of the table in the playerdb that contains information
   * about the music library.
   *
   * @return The name of the table in the playerdb that contains information
   * about the music library.
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
   * @return The name of the id column in the active playlist table.
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
   * @return The name of the library id column in the active playlist table.
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

  /** 
   * \brief Gets the name of the adder username column in the active playlist table.
   *
   * @return The name of the adder username column in the active playlist table.
   */
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

  /** 
   * \brief Gets the genre column in the library table table.
   *
   * @return The name of the genre column in the library table.
   */
  static const QString& getLibGenreColName(){
    static const QString libGenreColName = "Genre";
    return libGenreColName;
  }

  /** 
   * \brief Gets the track column in the library table table.
   *
   * @return The name of the track column in the library table.
   */
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

  /** 
   * \brief Gets the is banned column in the library table table.
   *
   * @return The name of the is banned column in the library table.
   */
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

  /** 
   * \brief Gets the value for the "needs ban" sync status used in the 
   * library table.
   *
   * @return The value for the "needs ban" sync status used in the library
   * table.
   */
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

  /**
   * \brief Gets the name of the LibIdAlias column for the playlist view.
   *
   * @return The name of the LibIdAlias column for the playlist view.
   */
  static const QString& getLibIdAlias(){
    static const QString libIdAlias = "libIdAlias";
    return libIdAlias;
  }

  /**
   * \brief Gets the name of the player id setting.
   *
   * @return The name of the player id setting.
   */
  static const QString& getPlayerIdSettingName(){
    static const QString playerIdSetting = "playerId";
    return playerIdSetting;
  }

  /**
   * \brief Gets the name of the player volume setting.
   *
   * @return The name of the player volume setting.
   */
  static const QString& getPlayerVolumeSettingName(){
    static const QString playerVolumeSettingName = "volume";
    return playerVolumeSettingName;
  }

  /**
   * \brief Gets the name of the player name setting.
   *
   * @return The name of the player name setting.
   */
  static const QString& getPlayerNameSettingName(){
    static const QString playerIdSetting = "playerName";
    return playerIdSetting;
  }

  /**
   * \brief Gets the name of the player state setting.
   *
   * @return The name of the player state setting.
   */
  static const QString& getPlayerStateSettingName(){
    static const QString playerStateSettingName = "playerState";
    return playerStateSettingName;
  }

  /**
   * \brief Name of the setting used to store the password being used for the player.
   *
   * @return Name of the setting used to store the password being used for the player.
   */
  static const QString& getPlayerPasswordSettingName(){
    static const QString playerPasswordSettingName = "playerPassword";
    return playerPasswordSettingName;
  }

  /**
   * \brief Gets the name of the player address setting.
   *
   * @return The name of the player address setting.
   */
  static const QString& getAddressSettingName(){
    static const QString addressSettingName = "address";
    return addressSettingName;
  }

  /**
   * \brief Gets the name of the player city setting.
   *
   * @return The name of the player city setting.
   */
  static const QString& getCitySettingName(){
    static const QString citySettingName = "city";
    return citySettingName;
  }

  /**
   * \brief Gets the name of the player state setting.
   *
   * @return The name of the player state setting.
   */
  static const QString& getStateSettingName(){
    static const QString stateSettingName = "state";
    return stateSettingName;
  }

  /**
   * \brief Gets the name of the player zip code setting.
   *
   * @return The name of the player zip code setting.
   */
  static const QString& getZipCodeSettingName(){
    static const QString zipCodeSettingName = "zipCode";
    return zipCodeSettingName;
  }

  /**
   * \brief Gets the value corresponding to an playling player state.
   *
   * @return The value corresponding to an playling player state.
   */
  static const QString& getPlayingState(){
    static const QString playingState = "playing";
    return playingState;
  }

  /**
   * \brief Gets the value corresponding to an inactive player state.
   *
   * @return The value corresponding to an inactive player state.
   */
  static const QString& getPausedState(){
    static const QString pausedState = "paused";
    return pausedState;
  }

  /**
   * \brief Gets the value corresponding to an inactive player state.
   *
   * @return The value corresponding to an inactive player state.
   */
  static const QString& getInactiveState(){
    static const QString inactiveState = "inactive";
    return inactiveState;
  }

  /**
   * \brief Gets the value used for the Settings Organization.
   *
   * @return The value of the settings organization.
   */
  static const QString& getSettingsOrg(){
    static const QString settingsOrg = "Bazaar Solutions";
    return settingsOrg;
  }

  /**
   * \brief Gets the value used for the Settings App.
   *
   * @return The value of the settings app.
   */
  static const QString& getSettingsApp(){
    static const QString settingsApp = "UDJ";
    return settingsApp;
  }

 //@}

/** @name Public slots */
//@{
public slots:

  /**
   * \brief Syncs the current state of the library with the server.
   */
  void syncLibrary();

  /**
   * \brief Pauses player.
   */
  void pausePlayer();

  /**
   * \brief Start player playing.
   */
  void playPlayer();

  /**
   * \brief Refresh the active playlist table.
   */
  void refreshActivePlaylist();

  /**
   * \brief Adds the given song to the active playlist.
   *
   * @param libraryId The song to add to the active playlist.
   */
  void addSongToActivePlaylist(library_song_id_t libraryId);

  /**
   * \brief Adds the given songs to the active playlist.
   *
   * @param libIds The songs to add to the active playlist.
   */
  void addSongsToActivePlaylist(const QSet<library_song_id_t>& libIds);

  /**
   * \brief Removes the given songs to the active playlist.
   *
   * @param libraryIds The songs to remove to the active playlist.
   */
  void removeSongsFromActivePlaylist(const QSet<library_song_id_t>& libraryIds);

  /** 
   * \brief Creates a new player with the given name and password.
   *
   * @param name The name of the player.
   * @param password The password for the event (is allowed to be empty, thus setting no password).
   */
  void createNewPlayer(
    const QString& name,
    const QString& password);

  /** 
   * \brief Creates a new player with the given name, password, and location.
   *
   * @param name The name of the player.
   * @param password The password for the event (is allowed to be empty, thus setting no password).
   * @param streetAddress The street address of the player.
   * @param city The city of the player.
   * @param state The state of the player.
   * @param zipcode The zipcode of the player.
   */
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

  /** 
   * \brief Changes the volume without emtting a signal noting that the volume has changed.
   *
   * @param newVolume The new player volume.
   */
  void changeVolumeSilently(qreal newVolume);

  /** \brief Determines whether or not the library has unsynced songs.*/
  bool hasUnsyncedSongs() const;

  /** \brief Determines the number of unsynced songs in the library.*/
  int getTotalUnsynced() const;

  //@}

signals:

/** @name Signals */
//@{

  /**
   * \brief Emitted when the player's password is set.
   */
  void playerPasswordSet();

  /**
   * \brief Emitter when there is an error setting the player's password.
   *
   * \param errMessage A message describing the error.
   */
  void playerPasswordSetError(const QString& errMessage);

  /**
   * \brief Emitted when the players location has been set.
   */
  void playerLocationSet();

  /**
   * \brief Emitted when there is an error setting the player's location.
   *
   * \brief errMessage Message describing the error.
   */
  void playerLocationSetError(const QString& errMessage);

  /**
   * \brief Emitted when the players name is succesfully changed.
   *
   * \param newName The new name the player was set to.
   */
  void playerNameChanged(const QString& newName);

  /**
   * \brief Emitted when changing the players name failes.
   *
   * \param errMessage An error message describing the failure.
   */
  void playerNameChangeError(const QString& errMessage);

  /**
   * \brief Emitted when the library table is modified.
   */
  void libSongsModified(const QSet<library_song_id_t>& modifiedSongs);

  /**
   * \brief Emitted when there was an error modifying the library.
   * 
   * @param errMessage An error message describing what happened.
   */
  void libModError(const QString& errMessage);

  /**
   * \brief Emitted when the library is completely synced with the server.
   */
  void allSynced();

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
  void manualSongChange(DataStore::song_info_t newSong);

  /**
   * \brief Emitted when the players state is changed.
   *
   * \param newState The new state of the player.
   */
  void playerStateChanged(const QString& newState);

  /**
   * \brief Emitted when the volume of the player is changed.
   *
   * @param newVolume The new volume of the player.
   */
  void volumeChanged(qreal newVolume);

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

  /** \brief Current username being used by the client */
  QString username;

  /** \brief Current password being used by the client */
  QString password;

  /** \brief A set of actions to be performed once the client has succesfully reauthenticated. */
  QSet<ReauthAction> reauthActions;

  /** \brief Whether or not the client is currently reauthenticating. */
  bool isReauthing;

  /** \brief The current song being played. */
  library_song_id_t currentSongId;

  /** \brief The set of songs that still need to be added to the active playlist. */
  QSet<library_song_id_t> playlistIdsToAdd;

  /** \brief The set of songs that still need to be removed from the active playlist. */
  QSet<library_song_id_t> playlistIdsToRemove;

  //@}

  /** @name Private Functions */
  //@{

  /** \brief Does initial database setup */
  void setupDB();

  /**
   * \brief Deletes all the entries in the active playlist table.
   */
  void clearActivePlaylist();

  /**
   * \brief Initiates reauthentication if it hasn't already been initiated.
   */
  void initReauth();

  /**
   * \brief Performs the specified ReauthAction.
   *
   * @param action The ReauthAction to preform.
   */
  void doReauthAction(const ReauthAction& action);

  /**
   * \brief Adds a single song to the music library.
   *
   * @param song Song to be added to the library.
   * @param addQuery Prepared statmemnt ready to be used for adding
   */
  void addSongToLibrary(const Phonon::MediaSource& song, QSqlQuery& addQuery);

  /**
   * \brief Gets the value of a header.
   *
   * @param headerName The name of the desired header.
   * @param headers The given headers.
   * @return The value of the header. If the header is not located in the given headers a blank
   * string is returned.
   */
  static QByteArray getHeaderValue(
    const QByteArray& headerName, const QList<QNetworkReply::RawHeaderPair>& headers);

  /**
   * \brief Determines whether or not an error is a ticket auth error.
   * 
   * \param errorCode The given error code from the server.
   * \param headers The given headers from the server.
   * \return True if the error is a ticket auth error, false otherwise.
   */
  static inline bool isTicketAuthError(
      int errorCode,
      const QList<QNetworkReply::RawHeaderPair>& headers)
  {
    return errorCode==401 && getHeaderValue("WWW-Authenticate", headers) == "ticket-hash";
  }

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

  /**
   * \brief Name of the setting used to store the username being used by the client.
   *
   * @return Name of the setting used to store the username being used by the client.
   */
  static const QString& getUsernameSettingName(){
    static const QString usernameSettingName = "username";
    return usernameSettingName;
  }

  /**
   * \brief Name of the setting used to store the password being used by the client.
   *
   * @return Name of the setting used to store the password being used by the client.
   */
  static const QString& getPasswordSettingName(){
    static const QString passwordSettingName = "password";
    return passwordSettingName;
  }


  /**
   * \brief Name of the setting used to store whether or not the current credentials are valid.
   *
   * @return Name of the setting used to store whether or not the current credentials are valid.
   */
  static const QString& getHasValidCredsSettingName(){
    static const QString hasValidSavedCredentialsSettingName = 
      "has_valid_creds";
    return hasValidSavedCredentialsSettingName;
  }

 //@}

/** @name Private Slots */
//@{
private slots:

  /**
   * \brief Preforms appropriate tasks when a player's password is set.
   *
   * \brief password The password that has been set on the server.
   */
  void onPlayerPasswordSet(const QString& password);

  /**
   * \brief Preforms appropriate tasks when there was an error setting the player's location.
   *
   * \param errMessage A message describing the error.
   * \param errorCode HTTP error code describing error.
   * \param headers HTTP headers accompianing in the error response.
   */
  void onPlayerPasswordSetError(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);


  /**
   * \brief Preforms appropriate tasks when a players location was succesfully set.
   *
   * \brief streetAddress The street address of location that was set for the player.
   * \brief city The city of location that was set for the player.
   * \brief state The state of location that was set for the player.
   * \brief zipcode The zipcode of location that was set for the player.
   */
  void onPlayerLocationSet(
    const QString& streetAddress,
    const QString& city,
    const QString& state,
    int zipcode
  );

  /**
   * \brief Preforms appropriate tasks when there was an error setting the player's location.
   *
   * \param errMessage A message describing the error.
   * \param errorCode HTTP error code describing error.
   * \param headers HTTP headers accompianing in the error response.
   */
  void onPlayerLocationSetError(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);

  /**
   * \brief Preforms appropriate tasks when a players name was succesfully changed on the
   * server.
   *
   * \param newName The new name of the player.
   */
  void onPlayerNameChanged(const QString& newName);

  /**
   * \brief Preforms appropriate tasks when there was an error changing the player name.
   *
   * \param errMessage A message describing the error.
   * \param errorCode HTTP error code describing error.
   * \param headers HTTP headers accompianing in the error response.
   */
  void onPlayerNameChangeError(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);

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
  void setLibSongsSynced(const QSet<library_song_id_t>& songs);

  /**
   * \brief Sets the sync status of the given library songs to the given
   * given sync status.
   *
   * @param songs The ids of the songs whose sync status should be set.
   * @param syncStatus The sync status to which the given songs should be set. 
   */
  void setLibSongsSyncStatus(
    const QSet<library_song_id_t>& songs,
    const lib_sync_status_t syncStatus);


  /**
   * \brief Adds the given song to the active playlist in the database.
   *
   * @param songToAdd A QVariantMap representing the song that should be added to the active
   * playlist in the database.
   * @param priority The priority of the song to be added.
   */
  void addSong2ActivePlaylistFromQVariant(const QVariantMap &songToAdd, int priority);

  /**
   * \brief Sets the active playlist to the given playlist.
   *
   * @param newPlaylist The new playlist to be set in the database.
   */
  void setActivePlaylist(const QVariantMap& playlist);

  /**
   * \brief Takes appropriate action when retreiving the active playlist fails.
   *
   * @param errMessage A message describing the error.
   * @param errorCode The http status code that describes the error.
   * @param headers The headers from the http response that indicated a failure.
   */
  void onGetActivePlaylistFail(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);


  /**
   * \brief Takes the appropriate action when a player is succesfully created.
   *
   * @param issuedId The id the server issued to the player that was created.
   */
  void onPlayerCreate(const player_id_t& issuedId);

  /**
   * \brief Takes appropriate action when creating a player fails.
   *
   * @param errMessage A message describing the error.
   * @param errorCode The http status code that describes the error.
   * @param headers The headers from the http response that indicated a failure.
   */
  void onPlayerCreationFailed(const QString& errMessage, int errorCode,
          const QList<QNetworkReply::RawHeaderPair>& headers);

  /**
   * \brief Takes appropriate action when the player state is changed.
   */
  void onPlayerStateChanged(const QString& newState);


  /**
   * \brief Takes appropriate action when modifiying the library on the server fails.
   *
   * @param errMessage A message describing the error.
   * @param errorCode The http status code that describes the error.
   * @param headers The headers from the http response that indicated a failure.
   */
  void onLibModError(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);

  /**
   * \brief Takes appropriate action when retreiving setting the current song on the server fails.
   *
   * @param errMessage A message describing the error.
   * @param errorCode The http status code that describes the error.
   * @param headers The headers from the http response that indicated a failure.
   */
  void onSetCurrentSongFailed(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);

  /**
   * \brief Takes appropriate action when the client succesfully reauthenticates.
   *
   * \param ticketHash The ticket hash that was recieved from the server.
   * \param userId The userId that was recieved from the server.
   */
  void onReauth(const QByteArray& ticketHash, const user_id_t& userId);

  /**
   * \brief Takes appropriate action when reauthentication fails.
   *
   * \param errMessage Error message given by the server.
   */
  void onAuthFail(const QString& errMessage);

  /**
   * \brief Takes appropriate action when the active playlist is succesfully modified on the server.
   *
   * \param added The songs that were added to the active playlist on the server.
   * \param removed The songs that were removed from the active playlist on the server.
   */
  void onActivePlaylistModified(
    const QSet<library_song_id_t>& added,
    const QSet<library_song_id_t>& removed);

  /**
   * \brief Takes appropriate action when modifiying the active playlist on the server fails.
   *
   * @param errMessage A message describing the error.
   * @param errorCode The http status code that describes the error.
   * @param headers The headers from the http response that indicated a failure.
   */
  void onActivePlaylistModFailed(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);

  /**
   * \brief Takes appropriate action when setting the volume fails.
   *
   * @param errMessage A message describing the error.
   * @param errorCode The http status code that describes the error.
   * @param headers The headers from the http response that indicated a failure.
   */
  void onSetVolumeFailed(
    const QString& errMessage,
    int errorCode,
    const QList<QNetworkReply::RawHeaderPair>& headers);

  //@}


//@}

};


} //end namespace
#endif //DATA_STORE_HPP
