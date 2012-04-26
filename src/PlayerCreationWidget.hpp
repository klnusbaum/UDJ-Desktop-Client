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
class QComboBox;

namespace UDJ{

class DataStore;

/** \brief Widget used to create new events. */
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

signals:
  /** @name Signals */
  //@{

  /** \brief Emitted when a new player is created */
  void playerCreated();

  //@}

private:
  /** @name Private Functions */
  //@{

  /** \brief Initilizes the UI */
  void setupUi();

  void setupStateCombo();

  QString getAddressBadInputs() const;

  static const QRegExp& getZipcodeRegex(){
    static const QRegExp zipcodeRegex("^\\d{5}(-\\d{4})?$");
    return zipcodeRegex;
  }

  //@}

  /** @name Private Memeber */
  //@{

  /** \brief lineedit used to retrieve the name of the event */
  QLineEdit *nameEdit;

  /** \brief lineedit used to the password of the event */
  QLineEdit *passwordEdit;

  QCheckBox *useAddress;

  /** \brief Lable to display instructions */
  QLabel *createLabel;

  /** \brief Button used to actually attempt to create the new player */
  QPushButton *createPlayerButton;

  /** 
   * \brief Widget containing form elements which need to be filled out in
   * order to create the new player.
   */
  QWidget *playerForm;

  /**
   * \brief The data store containing music that could potentially be added
   * to the playlist.
   */
  DataStore *dataStore;

  QLineEdit *streetAddress;
  QLineEdit *city;
  QComboBox *state;
  QLineEdit *zipcode;


  //@}

private slots:
  /** @name Private Slots */
  //@{

  /** \brief Attemps to create a new event */
  void doCreation();

  /**
   * \brief Executes appropriate actions after player has actually been created.
   */
  void playerCreateSuccess();

  /**
   * \brief Executes appropriate actions after player was failed to be created.
   */
  void playerCreateFail(const QString& errMessage);

  void enableAddressInputs(bool enable);

  //@}

};


}//end namspace UDJ


#endif //PLAYER_CREATION_WIDGET_HPP
