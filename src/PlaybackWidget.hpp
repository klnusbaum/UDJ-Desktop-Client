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
#ifndef PLAYBACK_WIDGET_HPP
#define PLAYBACK_WIDGET_HPP
#include "DataStore.hpp"
#include <QWidget>
#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>


class QAction;
class QLabel;


namespace UDJ{

/** \brief Widget used for controlling music playback. */
class PlaybackWidget : public QWidget{

Q_OBJECT

public:
  /** @name Public Enums */
  //@{

  /** \brief The various states of playback that the widget can be in. */
  enum PlaybackState {PAUSED, PLAYING};

  //@}

  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a Playback widget.
   *
   * @param dataStore The DataStore backing this instance of UDJ.
   * @param parent The parent widget.
   */
  PlaybackWidget(DataStore *dataStore, QWidget *parent=0);

  //@}
  
public slots:
  /** @name Public Slots */
  //@{ 

  /**
   * \brief Toggles the playback state of playback widget.
   */
  void togglePlaybackState();

  //@}

private slots:

  /** @name Private Slots */
  //@{

   /** \brief Enables playback. */
   void play();

   /** \brief Pauses playback. */
   void pause();

  /**
   * \brief Handles whenever the state of the primary
   * MediaObject is changed. 
   *
   * @param newState The new state of the primary MediaObject.
   * @parma oldState The old state of the primary MediaObject.
   */
  void stateChanged(Phonon::State newState, Phonon::State oldState);

  /** \brief Called when ever the primary MediaObject "ticks" 
   *
   * @param time The current time of the primary MediaObject.
   */
  void tick(qint64 time);

  /**
   * \brief Called whenever media source of the primary MediaObject is changed.
   *
   * @param source The new source of the primary MediaObject.
   */
  void sourceChanged(const Phonon::MediaSource &source);

  /**
   * \brief Called when the next available song should be played.
   */
   void playNextSong();

   /** \brief Handles when meta data is changed. */
   void metaDataChanged();

   /**
    * \brief Sets the currently playing song to the given new song.
    *
    * @param newSong The new song that should be playing.
    */
   void setNewSource(DataStore::song_info_t newSong);

   /** \brief Clears the data on the playback widget. */
   void clearWidget();

   /** \brief Takes appropriate action when the playlist is changed. */
   void handlePlaylistChange();

   /** \brief Takes appropriate action when the player state is changed. */
   void onPlayerStateChanged(const QString& newState);

  //@}

private:

  /** @name Private Functions */
  //@{

  /** \brief Sets up all the actions used by the MetaWindow. */
  void createActions();

  /** \brief Initializes UI. */
  void setupUi();

  /** 
   * \brief Informs the user the song that they just tried to play doesn't work.
   */
  void informBadSong();

  /**
   * \brief Sets the song info in the widget.
   *
   * \param newSong The info that should be set in the widget.
   */
   void setSongInfo(const DataStore::song_info_t& newSong);

  //@}

  /** @name Private Memeber */
  //@{

  /** \brief The data store backing this instance of UDJ. */
  DataStore *dataStore;

  /** \brief The current state of music playback. */
  PlaybackState currentPlaybackState;

  /** \brief The title of the current song being played. */
  QString currentSongTitle;

  /** \brief The artist of the current song being played. */
  QString currentSongArtist;

  /** \brief The duration of the current song being played. */
  QString currentSongDuration;

  /** \brief Causes playback to start */
  QAction *playAction;

  /** \brief Pauses playback */
  QAction *pauseAction;

  /** \brief Skips to the next song */
  QAction *skipAction;

  /** \brief Used to display info of the currently playing song. */
  QLabel *songInfo;

  /** \bried Used to display the time played of the current song. */
  QLabel *timeLabel;

  /** \brief The primary media object used for song playback. */
  Phonon::MediaObject *mediaObject;

  /** \brief The primary audioOutput device used for song playback. */
  Phonon::AudioOutput *audioOutput;

  /** \brief The volume slider used to control playback volume. */
  Phonon::VolumeSlider *volumeSlider;

  /** \brief The seek slider for adjusting playback position. */
  Phonon::SeekSlider *seekSlider;

  //@}

};

} //end namespace UDJ
#endif
