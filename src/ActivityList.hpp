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
#ifndef ACTIVITY_LIST_HPP
#define ACTIVITY_LIST_HPP
#include <QTreeView>
#include "ConfigDefs.hpp"

class QStandardItemModel;
class QStandardItem;
class QActivity;
namespace UDJ{


class DataStore;


/**
 * \brief Displays the various activities that can be done in UDJ.
 */
class ActivityList : public QTreeView{
Q_OBJECT
public:
  /** @name Constructors */
  //@{

  /**
   * \brief Constructs an ActivityList
   *
   * @param dataStore The DataStore backing this instance of UDJ.
   * @param parent The parent widget.
   */
  ActivityList(DataStore *dataStore, QWidget *parent=0);

  //@}

  /** @name Public Slots */
  //@{

public slots:

  /**
   * \brief Switches the selected activity to Library.
   */
  void switchToLibrary();

  //@}

signals:
  /** @name Signals */
  //@{

  /**
   * \brief Emitted when the library activity is clicked.
   */
  void libraryClicked();

  /**
   * \brief Emitted when the playlist activity is clicked.
   */
  void playlistClicked();

  /**
   * \brief Emitted when the participants item is clicked.
   */
  void participantsClicked();

  //@}

private:

  /** @name Private Functions */
  //@{

  /**
   * \brief Does UI initialization.
   */
  void setupUi();

  /**
   * \brief Gets the name of the library activity.
   *
   * @return The name of the library activity.
   */
  static const QString& getLibraryTitle(){
    static const QString libraryTitle(tr("Library"));
    return libraryTitle;
  }

  /** 
   * \brief Gets the name of the Playlist activity.
   *
   * @return The name of the Playlist activity.
   */
  static const QString& getPlaylistTitle(){
    static const QString playlistTitle(tr("Playlist"));
    return playlistTitle;
  }

  /** 
   * \brief Gets the name of the Participants activity.
   *
   * @return The name of the Participants activity.
   */
  static const QString& getParticipantsTitle(){
    static const QString participantsTitle(tr("Participants"));
    return participantsTitle;
  }


  //@}

  /** @name Private Members */
  //@{

  /** \brief Pointer to the DataStore backing this instance of UDJ */
  DataStore *dataStore;

  /** \brief Model used to list the activities. */
  QStandardItemModel *model;

  /** \brief The item representing the library activity. */
  QStandardItem *libraryItem;

  /** \brief The item representing the playlist activity. */
  QStandardItem *playlistItem;

  /** \brief The item representing the participants activity. */
  QStandardItem *participantsItem;

  //@}

private slots:

  /** @name Private Slots */
  //@{

  /**
   * \brief Emits the appropriate signals when an activity is clicked.
   *
   * @param index The index of the activity that was clicked.
   */
  void itemClicked(const QModelIndex& index);

  //@}
};


}//end namespace UDJ
#endif //ACTIVITY_LIST_HPP
