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
#include "PlayerCreationWidget.hpp"
#include "DataStore.hpp"
#include "AddressWidget.hpp"
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>


namespace UDJ{


PlayerCreationWidget::PlayerCreationWidget(
  DataStore *dataStore,
  QWidget *parent):
  WidgetWithLoader(tr("Creating Player..."), parent),
  dataStore(dataStore)
{
  setupUi();
  connect(
    dataStore,
    SIGNAL(playerCreated()),
    this,
    SLOT(playerCreateSuccess()));
  connect(
    dataStore,
    SIGNAL(playerCreationFailed(const QString&)),
    this,
    SLOT(playerCreateFail(const QString&)));
}


void PlayerCreationWidget::setupUi(){
  QWidget *mainWidget = new QWidget(this);
  QLabel *welcomeMessage = new QLabel(tr("Welcome to UDJ! We just need you to fill a few things out before you get started. Can you tell us a little information about this music player?"));
  welcomeMessage->setWordWrap(true);

  nameEdit = new QLineEdit();
  passwordEdit = new QLineEdit();
  addressGroup = new QGroupBox();
  addressWidget = new AddressWidget();
  QGridLayout *addressLayout = new QGridLayout();
  addressLayout->addWidget(addressWidget,0,0);
  addressGroup->setLayout(addressLayout);
  addressGroup->setCheckable(true);
  addressGroup->setChecked(false);
  addressGroup->setTitle(tr("Provide Location"));

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow(tr("Name of player:"), nameEdit);
  formLayout->addRow(tr("Password (optional):"), passwordEdit);

  connect(
    addressGroup,
    SIGNAL(toggled(bool)),
    addressWidget,
    SLOT(setEnabled(bool)));
  addressWidget->setEnabled(false);

  QGridLayout *mainLayout = new QGridLayout();
  mainLayout->addWidget(welcomeMessage, 0,0);
  mainLayout->addLayout(formLayout,1,0);
  mainLayout->addWidget(addressGroup,2,0);

  mainWidget->setLayout(mainLayout);

  setMainWidget(mainWidget);
  showMainWidget();
}


void PlayerCreationWidget::doCreation(){
  showLoadingText();
  if(nameEdit->text() == ""){
    playerCreateFail("You must provide a name for your player." );
    return;
  }

  if(addressGroup->isChecked()){
    QString badInputs = addressWidget->getBadInputs();
    if(badInputs == ""){
      dataStore->createNewPlayer(
        nameEdit->text(),
        passwordEdit->text(),
        addressWidget->getStreetAddress(),
        addressWidget->getCity(),
        addressWidget->getState(),
        addressWidget->getZipcode());
    }
    else{
      playerCreateFail("The address you supplied is invalid. Please correct " 
        "the following errors:\n\n" + badInputs);
    }
  }
  else{
    dataStore->createNewPlayer(nameEdit->text(), passwordEdit->text());
  }
}


void PlayerCreationWidget::playerCreateSuccess(){
  showMainWidget();
  emit playerCreated();
}

void PlayerCreationWidget::playerCreateFail(const QString& errMessage){
  showMainWidget();
  QMessageBox::critical(
    this,
    tr("Player Creation Failed"),
    errMessage);
  emit playerCreateFailed();
}


}//end namespace UDJ

