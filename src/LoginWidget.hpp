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
#ifndef LOGIN_WIDGET_HPP
#define LOGIN_WIDGET_HPP

#include "WidgetWithLoader.hpp"
#include "ConfigDefs.hpp"
#include <QNetworkReply>

class QLabel;
class QLineEdit;
class QCheckBox;

namespace UDJ{

class UDJServerConnection;

/** \brief Widget used to login to the UDJ server */
class LoginWidget : public WidgetWithLoader{
Q_OBJECT
public:
  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a Login Widget
   *
   * \param parent Parent Widget.
   */
  LoginWidget(QWidget *parent=0);

  //@}


public slots:
  
  /** @name Public slots */
  //@{

  /** \brief Perform actions necessary for loggin in */
  void doLogin();

  //@}


signals:

  /**
   * \brief Emitted once the main gui is started.
   */
  void startedMainGUI();

  /**
   * \brief Emitted when logging in fails.
   */
  void loginFailed();


private:

  /** @name Private Memeber */
  //@{

  /** \brief Label used for displayling the UDJ logo. */
  QLabel *logo;

  /** \brief Lineedit used for entering the user name */
  QLineEdit *usernameBox;

  /** \brief lineedit used for entering the password. */
  QLineEdit *passwordBox;

  /** \brief Checkbox for indicating whether or not password should be saved. */
  QCheckBox *savePassword;

  /** \brief Label giving registration information. */
  QLabel *registerText;

  /** \brief Label giving forgotten password information. */
  QLabel *forgotPasswordText;

  /** \brief Actual display for the login widget. */
  QWidget *loginDisplay;

  /** \brief Connection to the server. */
  UDJServerConnection *serverConnection;

  /** \brief Username label */
  QLabel *usernameLabel;

  /** \brief Password label */
  QLabel *passwordLabel;

  QByteArray ticketHash;

  user_id_t userId;

  QVariantList sortingAlgorithms;


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
   * \brief Preforms the appropriate actions when a successful authentication occurs.
   *
   * \param ticketHash Ticket hash recieved from the server upon succesfully authentication.
   * \param userId User id recieved from the server upon succesfully authentication.
   */
  void onSuccessfulAuth(
    const QByteArray& ticketHash, const user_id_t& userId);

  void onGotSortingAlgorithms(const QVariantList& sortingAlgorithms);


  /**
   * \brief Performs the appropriate actions needed to start up the main gui.
   */
  void startMainGUI();

  /**
   * \brief Displays a message informing the user that the attempt to login to
   * the UDJ server failed.
   *
   * @param errorMessage The error message describing the failure.
   * @param errorCode HTTP error code describing error.
   * @param headers HTTP headers accompianing in the error response.
   */
  void displayLoginFailedMessage(
      const QString& errorMessage,
      int errorCode,
      const QList<QNetworkReply::RawHeaderPair>& headers);

  void onAuthFail(
      const QString& errorMessage,
      int errorCode,
      const QList<QNetworkReply::RawHeaderPair>& headers);

  /**
   * \brief Takes appropriate action when the user clicks on the savePassword check box.
   *
   * \param newSetting The current state of the save password check box.
   */
  void savePasswordChanged(bool newSetting);

  //@}
};


} //end namespace UDJ


#endif //LOGIN_WIDGET_HPP
