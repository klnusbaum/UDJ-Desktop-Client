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
#ifndef ERROR_MESSAGE_HPP
#define ERROR_MESSAGE_HPP

#include <QDialog>

class QPushButton;
class QLabel;
class QCheckBox;

namespace UDJ{

class DataStore;


/**
 * \brief Dialog for displaying a playback error message
 */
class PlaybackErrorMessage : public QDialog{
Q_OBJECT
public:


  /** @name Constructors */
  //@{

  /**
   * \brief Creates a PlaybackErrorMessage.
   *
   * \param title Title of the error message.
   * \param message The message to be displayed
   * \param parent The parent widget.
   * \param f Any window flags.
   */
  PlaybackErrorMessage(const QString& title, const QString& message, 
  QWidget *parent=0, Qt::WindowFlags f=0);

  //@}

private:

  /** @name Private Memeber */
  //@{

  /** \brief A label displaying the error message. */
  QLabel *errorMessage;

  /** \brief The button used to dismiss the dialog. */
  QPushButton *okButton;

  /** \brief A checkbox indicating whether or not to 
   * display the error message again if the error arrises again.
   */
  QCheckBox *dontShowAgain;


  //@}

private slots:
  /** @name Private Slots */
  //@{

  /** \brief Takes appropriate action when the dontShowAgain
   * checkbox is checked or unchecked.
   *
   * \param checked Whether or not the checkbox is now checked.
   */
  void dontShowAgainChecked(bool checked);

  //@}
};


} //end namespace UDJ


#endif //ERROR_MESSAGE_HPP




