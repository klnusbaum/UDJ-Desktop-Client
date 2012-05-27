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
#ifndef PLAYER_CREATION_WIDGET_HPP
#define PLAYER_CREATION_WIDGET_HPP
#include "WidgetWithLoader.hpp"

class QLineEdit;
class QPushButton;
class QLabel;
class QProgressDialog;
class QCheckBox;

namespace UDJ{

class DataStore;
class AddressWidget;

/** \brief Widget used to create a new player. */
class PlayerCreationWidget : public WidgetWithLoader{
Q_OBJECT
public:
  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a PlayerCreationWidget.
   *
   * @param dataStore The DataStore backing this instance of UDJ.
   * @param parent The parent widget.
   */
  PlayerCreationWidget(DataStore *dataStore, QWidget *parent=0);

  //@}

public slots:

  /** @name Public Slots */
  //@{

  /** \brief Attemps to create a new event */
  void doCreation();

  //@}


signals:
  /** @name Signals */
  //@{

  /** \brief Emitted when a new player is created. */
  void playerCreated();

  /** \brief Emitted when creating a new player fails. */
  void playerCreateFailed();

  //@}

private:
  /** @name Private Functions */
  //@{

  /** \brief Initilizes the UI */
  void setupUi();

  //@}

  /** @name Private Memeber */
  //@{

  /** \brief lineedit used to retrieve the name of the event */
  QLineEdit *nameEdit;

  /** \brief lineedit used to the password of the event */
  QLineEdit *passwordEdit;

  /** \brief Checkbox used to indicate the user wishes to provide a location for the player */
  QCheckBox *useAddress;

  /** \brief Lable to display instructions */
  QLabel *createLabel;

  /**
   * \brief The data store containing music that could potentially be added
   * to the playlist.
   */
  DataStore *dataStore;

  /**
   * \brief Widget for retrieving address information.
   */
  AddressWidget *addressWidget;

  /** \brief From containing the inputs required for player creation. */
  QWidget *playerForm;

  //@}

private slots:
  /** @name Private Slots */
  //@{

  /**
   * \brief Executes appropriate actions after player has actually been created.
   */
  void playerCreateSuccess();

  /**
   * \brief Executes appropriate actions after player was failed to be created.
   */
  void playerCreateFail(const QString& errMessage);

  //@}

};


}//end namspace UDJ


#endif //PLAYER_CREATION_WIDGET_HPP
