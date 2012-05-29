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
#ifndef SET_PASSWORD_DIALOG_HPP
#define SET_PASSWORD_DIALOG_HPP

#include "DialogWithLoaderWidget.hpp"

class QLineEdit;

namespace UDJ{

class DataStore;

/** \brief Dialog for setting a player's password. */
class SetPasswordDialog : public DialogWithLoaderWidget{
Q_OBJECT
public:

  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a SetPasswordDialog
   *
   * \param dataStore The DataStore backing the client.
   * \param parent The parent widget.
   * \param f Window flags.
   */
  SetLocationDialog(DataStore *dataStore, QWidget *parent=0, Qt::WindowFlags f=0);

  //@}

private:

  /** @name Private Members */
  //@{

  QLineEdit *passwordEdit;

  /** \brief DataStore backing the client */
  DataStore *dataStore;

  //@}

  /** @name Private Functions */
  //@{

  /** \brief Initializes UI. */
  void setupUi();


  //@}

private slots:
  /** @name Private Slots */
  //@{

  /**
   * \brief Handles password setting errors
   *
   * \param errMessage A message describing the error.
   */
  void onPlayerPasswordSetError(const QString& errMessage);

  //@}
};


} //end namespace UDJ


#endif //SET_PASSWORD_DIALOG_HPP
