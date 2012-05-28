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
#ifndef LOGIN_DIALOG_HPP
#define LOGIN_DIALOG_HPP

#include "DialogWithLoaderWidget.hpp"

namespace UDJ{

class LoginWidget;


/** \brief Dialog used to login to the UDJ server */
class LoginDialog : public DialogWithLoaderWidget{
Q_OBJECT
public:
  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a Login Widget
   *
   * \brief parent Parent widget.
   * \brief f Window flags.
   */
  LoginDialog(QWidget *parent=0, Qt::WindowFlags f=0);

  //@}

public slots:

  /** @name Overridden slots */
  //@{

  /** \brief . */
  virtual void accept();

  /** \brief . */
  virtual void reject();

  //@}

private:

  /** @name Private Memeber */
  //@{

  /** \brief Widget used for logging in. */
  LoginWidget *loginWidget;

  //@}

  /** @name Private Functions */
  //@{

  /** \brief Initializes UI. */
  void setupUi();


  //@}

};


} //end namespace UDJ


#endif //LOGIN_DIALOG_HPP



