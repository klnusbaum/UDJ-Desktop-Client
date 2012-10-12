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

#include "LoginWidget.hpp"
#include "UDJServerConnection.hpp"
#include "MetaWindow.hpp"
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QMessageBox>
#include <QCheckBox>
#include "DataStore.hpp"


namespace UDJ{


LoginWidget::LoginWidget(QWidget *parent)
  :WidgetWithLoader(tr("Logging in..."), parent)
{
  serverConnection = new UDJServerConnection(this);
  setupUi();
  connect(
    serverConnection,
    SIGNAL(authenticated(const QByteArray&, const user_id_t&)),
    this,
    SLOT(onSuccessfulAuth(const QByteArray&, const user_id_t&)));

  connect(
      serverConnection,
      SIGNAL(gotSortingAlgorithms(const QVariantList&)),
      this,
      SLOT(onGotSortingAlgorithms(const QVariantList&)));



  connect(
    serverConnection,
    SIGNAL(authFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(onAuthFail(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(authFailed(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(displayLoginFailedMessage(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));

  connect(
    serverConnection,
    SIGNAL(getSortingAlgorithmsError(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)),
    this,
    SLOT(displayLoginFailedMessage(const QString&, int, const QList<QNetworkReply::RawHeaderPair>&)));
}

void LoginWidget::setupUi(){
  loginDisplay = new QWidget(this);

  logo = new QLabel("UDJ", this);

  usernameBox = new QLineEdit(this);
  usernameLabel = new QLabel(tr("Username"));
  usernameLabel->setBuddy(usernameBox);

  passwordBox = new QLineEdit(this);
  passwordBox->setEchoMode(QLineEdit::Password);
  passwordLabel = new QLabel(tr("Password"));
  passwordLabel->setBuddy(passwordBox);


  savePassword = new QCheckBox(tr("Remember password"));

  registerText = new QLabel(tr("No account? <a href=\"https://www.udjplayer.com/registration/register/\">Register here</a>"));
  registerText->setOpenExternalLinks(true);

  forgotPasswordText = new QLabel(tr("<a href=\"https://www.udjplayer.com/recover/\">Forgot your password?</a>"));
  forgotPasswordText->setOpenExternalLinks(true);

  connect(
    savePassword,
    SIGNAL(toggled(bool)),
    this,
    SLOT(savePasswordChanged(bool)));


  QGridLayout *layout = new QGridLayout;
  layout->addWidget(logo,0,0,1,2, Qt::AlignCenter);
  layout->addWidget(usernameLabel,1,0);
  layout->addWidget(usernameBox,1,1);
  layout->addWidget(passwordLabel,2,0);
  layout->addWidget(passwordBox,2,1);
  layout->addWidget(savePassword, 3, 1);
  layout->addWidget(forgotPasswordText, 4, 0, 1, 2, Qt::AlignCenter);
  layout->addWidget(registerText, 5, 0, 1, 2, Qt::AlignCenter);



  loginDisplay->setLayout(layout);

  setMainWidget(loginDisplay);
  showMainWidget();

  //If we already have a player id we can't let the sign in with a different user
  //then the last one the signed in with. Otherwise bad things might happen because
  //the user they sign in as may not have permission to do things like set the player state.
  //The check for if they have a saved username or not is for people who are upgrading and may
  //not have saved their username in older versions but have created a player.
  if(DataStore::hasPlayerId() && DataStore::getSavedUsername() != "" ){
    QString alreadyAssociatedMessage(tr("You have already associated a player with this computer.\n"
      "This means you have to login as the player's owner."));
    usernameBox->setEnabled(false);
    usernameBox->setToolTip(alreadyAssociatedMessage);
    usernameLabel->setEnabled(false);
    usernameLabel->setToolTip(alreadyAssociatedMessage);
  }

  usernameBox->setText(DataStore::getSavedUsername());

  if(DataStore::hasValidSavedPassword()){
    passwordBox->setText(DataStore::getSavedPassword());
    savePassword->setChecked(true); 
  }
}

void LoginWidget::doLogin(){
  showLoadingText();
  serverConnection->authenticate(usernameBox->text(), passwordBox->text());
}

void LoginWidget::onSuccessfulAuth(
  const QByteArray& ticketHash, const user_id_t& userId)
{
  this->ticketHash = ticketHash;
  this->userId = userId;
  serverConnection->setTicket(ticketHash);
  serverConnection->getSortingAlgorithms();

}

void LoginWidget::onGotSortingAlgorithms(const QVariantList& sortingAlgorithms){
  this->sortingAlgorithms = sortingAlgorithms;
  startMainGUI();
}

void LoginWidget::startMainGUI()
{
  if(savePassword->isChecked()){
    DataStore::savePassword(passwordBox->text());
  }

  DataStore::saveUsername(usernameBox->text());

  MetaWindow *metaWindow = new MetaWindow(
    usernameBox->text(),
    passwordBox->text(),
    this->ticketHash,
    this->userId,
    this->sortingAlgorithms);
  metaWindow->show();
  emit startedMainGUI();
}

void LoginWidget::onAuthFail(
  const QString& /*errorMessage*/,
  int /*errorCode*/,
  const QList<QNetworkReply::RawHeaderPair>& /*headers*/)
{
  DataStore::setPasswordDirty();
}


void LoginWidget::displayLoginFailedMessage(
  const QString& errorMessage,
  int /*errorCode*/,
  const QList<QNetworkReply::RawHeaderPair>& /*headers*/)
{
  emit loginFailed();
  showMainWidget();
  setCurrentWidget(loginDisplay);
  QMessageBox::critical(
    this,
    tr("Login Failed"),
    errorMessage);
}

void LoginWidget::savePasswordChanged(bool newSetting){
  if(!newSetting && DataStore::hasValidSavedPassword()){
    DataStore::clearSavedPassword();
    passwordBox->setText("");
  }
}


}// end namespace UDJ


