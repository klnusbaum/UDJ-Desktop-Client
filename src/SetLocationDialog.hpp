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
#ifndef SET_LOCATION_DIALOG_HPP
#define SET_LOCATION_DIALOG_HPP

#include <QDialog>
class QPushButton;

namespace UDJ{

class WidgetWithLoader;
class DataStore;

/** \brief Dialog for creating a player. */
class SetLocationDialog : public QDialog{
Q_OBJECT
public:
  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a SetLocationDialog
   *
   * \param dataStore The DataStore backing the client.
   * \param parent The parent widget.
   * \param f Window flags.
   */
  SetLocationDialog(DataStore *dataStore, QWidget *parent=0, Qt::WindowFlags f=0);

  //@}

public slots:

  /** @name Overridden slots from QDialog */
  //@{

  /** \brief . */
  virtual void accept();

  //@}


private:

  /** @name Private Members */
  //@{

  /** \brief Actuall widget used for setting the address. */
  AddressWidget *addressWidget;

  /** \brief Widget used for containing the address widget.*/
  WidgetWithLoader *loaderContainer;

  /** \brief button used for initiating the setting of the location. */
  QPushButton *setLocationButton;

  /** \brief button used for canceling the setting of the location. */
  QPushButton *cancelButton;

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

  /** \brief Closes the dialog */
  void closeDialog();

  /**
   * \brief Handles location change errors
   *
   * \param errMessage A message describing the error.
   */
  void onChangeLocationError(const QString& errMessage);

  //@}
};


} //end namespace UDJ


#endif //SET_LOCATION_DIALOG_HPP
