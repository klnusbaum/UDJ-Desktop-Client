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
#ifndef PLAYER_CREATE_DIALOG_HPP
#define PLAYER_CREATE_DIALOG_HPP

#include "DialogWithLoaderWidget.hpp"


namespace UDJ{

class PlayerCreationWidget;
class DataStore;

/** \brief Dialog for creating a player. */
class PlayerCreateDialog : public DialogWithLoaderWidget{
Q_OBJECT
public:
  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a PlayerCreateDialog
   *
   * \param dataStore The DataStore backing the client.
   * \param parent The parent widget.
   * \param f Window flags.
   */
  PlayerCreateDialog(DataStore *dataStore, QWidget *parent=0, Qt::WindowFlags f=0);

  //@}

public slots:
  
  /** @name Overridden slots from QDialog */
  //@{

  /** \brief . */
  virtual void accept();

  /** \brief . */
  virtual void reject();

  //@}

private:

  /** @name Private Memeber */
  //@{

  /** \brief Widget used for actual creation of player */
  PlayerCreationWidget *createWidget;

  /** \brief DataStore backing the client */
  DataStore *dataStore;


  //@}

  /** @name Private Functions */
  //@{

  /** \brief Initializes UI. */
  void setupUi();


  //@}

};


} //end namespace UDJ


#endif //PLAYER_CREATE_DIALOG_HPP




