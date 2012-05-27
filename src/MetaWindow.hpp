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
#ifndef METAWINDOW_HPP
#define METAWINDOW_HPP
#include <QMainWindow>
#include <QTableView>
#include <QSqlDatabase>
//#include <QFileSystemWatcher>
#include <QSqlTableModel>
#include "UDJServerConnection.hpp"
#include "PlaybackWidget.hpp"

class QTabWidget;
class QPushButton;
class QAction;
class QLabel;
class QSplitter;
class QStackedWidget;
class QCloseEvent;
class QProgressDialog;

namespace UDJ{

class ActivePlaylistView;
class LibraryWidget;
class ActivityList;
class EventWidget;
class DataStore;
class PlayerDashboard;

/**
 * \brief A class that is the main point of interaction with the user. 
 * 
 * This is the main window with which the user will interact. It contains
 * all information about the current playlist, their music, and any other relevant
 * information.
 */
class MetaWindow : public QMainWindow{
  Q_OBJECT
public:
  /** @name Constructor(s) */
  //@{

  /** \brief Constructs a MetaWindow
   *
   * @param username The username being used by the client.
   * @param password The password being used by the client.
   * @param ticketHash Ticket hash that should be used by the data store.
   * @param userId UserId that should be used by the data store.
   * @param parent The parent widget
   * @param flags Any window flags.
   */
  MetaWindow(
    const QString& username,
    const QString& password,
    const QByteArray& ticketHash,
    const user_id_t& userId,
    QWidget *parent=0, 
    Qt::WindowFlags flags=0);

  //@}

protected:

  /** @name Overridden from QMainWindow */
  //@{

  /** \brief . */
  virtual void closeEvent(QCloseEvent *event);

  //@}

private slots:

  /** @name Private Slots */
  //@{

  /** \brief Shows the logger view */
  void displayLogView();

  /** \brief Shows the about widget*/
  void displayAboutWidget();

  /** \brief Initiates the syncing of the library */
  void syncLibrary();

  /**
   * \brief Displays stuff for adding songs to a library.
   */
  void addMusicToLibrary();

  /**
   * \brief Displays stuff for adding a single to the library.
   */
  void addSongToLibrary();

  /**
   * \brief Displays the library widget in the main content panel.
   */
  void displayLibrary();

  /**
   * \brief Displays the playlist view in the main content panel.
   */
  void displayPlaylist();

  /**
   * \brief Updates the syncprogress given the songs that have been updated.
   *
   * \param songs Songs that were updated.
   */
  void syncUpdate(const QSet<library_song_id_t>& songs);

  /**
   * \brief Performs necessary actions when the library syncing is done.
   */
  void syncDone();

  /**
   * \brief Performs necessary actions when the library syncing has an error.
   *
   * \param errMessage The error message given on a sync error.
   */
  void syncError(const QString& errMessage);

  /**
   * \brief Initiates the process for changing the players name.
   */
  void changePlayerName();

  /**
   * \brief Preforms necessary actions when a players name is succesfully changed.
   */
  void onPlayerNameChanged();

  /**
   * \brief Preforms necessary actions when changing a players name failed.
   *
   * \param errMessage A message describing the error.
   */
  void onPlayerNameChangeError(const QString& errMessage);

  //@}

private:
  /** @name Private Members */
  //@{

  /** \brief Used to display the contents of the users media library */
  LibraryWidget* libraryWidget;

  /** \brief The users media library */
  DataStore* dataStore;

  /** \brief Triggers selection of music directory. */
  QAction *addMusicAction;

  /** \brief Causes the application to quit. */
  QAction *quitAction;

  /** \brief Trigers addition of single song to the library */
  QAction *addSongAction;

  /** \brief Triggers display of the log viewer */
  QAction *viewLogAction;

  /** \brief Triggers display of the about widget */
  QAction *viewAboutAction;

//  QFileSystemWatcher* fileWatcher;


  /** \brief The main display widget. */
  QWidget *mainWidget;

  /** \brief The list of potential activites that can be done in UDJ. */
  ActivityList *activityList;

  /** \brief Widget used for controlling music playback. */
  PlaybackWidget *playbackWidget;

  /** \brief Widget used to display the active playlist. */
  ActivePlaylistView *playlistView;

  /** \brief Progress dialog used when quitting.*/
  QProgressDialog *quittingProgress;

  /** \brief Progress dialog used syncing library.*/
  QProgressDialog *syncingProgress;

  /** \brief Stack used to display various UI components. */
  QStackedWidget *contentStack;

  /** \brief Dashboard used to display information about the player. */
  PlayerDashboard *dashboard;

  /** \brief A flag indicating whether or not the client is in the process of quitting. */
  bool isQuiting;

  //@}

  /** @name Private Functions */
  //@{

  /** \brief Sets up all the MetaWindow's UI components. */
  void setupUi();

  /** \brief Sets up the MetaWindow's menus. */
  void setupMenus();

  /** \brief Creates the actions used in the MetaWindow */
  void createActions();

  /** \brief Configures the menu for changing player settings. */
  void configurePlayerMenu();

  /**
   * \brief Disconnects any signals that may have been setup at the beginning
   * of a library sync operation.
   */
  void disconnectSyncSignals();

  /**
   * \brief Disconnects any signals that may have been setup when initiating a player
   * name change operation.
   */
  void disconnectNameChangeSignals();

  //@}

};


} //end namespace 
#endif //METAWINDOW_HPP
