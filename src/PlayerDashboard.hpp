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
#ifndef PLAYER_DASHBOARD_HPP
#define PLAYER_DASHBOARD_HPP
#include <QWidget>

class QLabel;

namespace UDJ{

class DataStore;

/**
 * \brief Widget for displaying information about the player
 */
class PlayerDashboard : public QWidget{
  Q_OBJECT
public:

  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a PlayerDashboard.
   *
   * \param dataStore The DataStore backing this client.
   * \param parent The parent widget.
   */
  PlayerDashboard(DataStore *dataStore, QWidget *parent=0);

  //@}

private slots:

  /** @name Private Slots */
  //@{

  /**
   * \brief Sets the player info to be displayed.
   */
  void setPlayerInfo();


  //@}

private:

  /** @name Private Memebers */
  //@{

  /** \brief The DataStore backing the client */
  DataStore *dataStore;

  /** \brief Lable for player name */
  QLabel *nameLabel;

  /** \brief Label for player password */
  QLabel *passwordLabel;

  /** \brief Label for player location */
  QLabel *locationLabel;

  //@}

  /** @name Private functions */
  //@{

  /**
   * \brief Sets up UI components
   */
  void setupUi();

  //@}
};


} //end namespace 
#endif //PLAYER_DASHBOARD_HPP

