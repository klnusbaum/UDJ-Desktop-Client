/**
 * Copyright 2011 Kurtis L. Nusbaum * 
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

#include "PlayerDashboard.hpp"
#include "DataStore.hpp"
#include <QLabel>
#include <QVBoxLayout>

namespace UDJ{

PlayerDashboard::PlayerDashboard(DataStore *dataStore, QWidget *parent):
  QWidget(parent),
  dataStore(dataStore)
{
  setupUi();
  connect(
    dataStore,
    SIGNAL(playerCreated()),
    this,
    SLOT(setPlayerInfo()));
  connect(
    dataStore,
    SIGNAL(playerLocationSet()),
    this,
    SLOT(setPlayerInfo()));
  connect(
    dataStore,
    SIGNAL(playerPasswordSet()),
    this,
    SLOT(setPlayerInfo()));
  connect(
    dataStore,
    SIGNAL(playerPasswordRemoved()),
    this,
    SLOT(setPlayerInfo()));
}

void PlayerDashboard::setupUi(){
  nameLabel = new QLabel();
  locationLabel = new QLabel();
  passwordLabel = new QLabel();

  setPlayerInfo();

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(nameLabel);
  layout->addWidget(passwordLabel);
  layout->addWidget(locationLabel);
  setLayout(layout);
}

void PlayerDashboard::setPlayerInfo(){
  nameLabel->setText(tr("Player Name: ") + dataStore->getPlayerName());
  passwordLabel->setText(tr("Password: No Password"));
  if(dataStore->hasPlayerPassword()){
    passwordLabel->setText(tr("Password: ") + dataStore->getPlayerPassword());
  }
  locationLabel->setText(tr("Location: Not Set"));
  if(dataStore->hasLocation()){
    locationLabel->setText(tr("Location: ") + dataStore->getLocationString());
  }
}





} //end namespace UDJ

