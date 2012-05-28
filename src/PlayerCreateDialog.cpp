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

#include "PlayerCreateDialog.hpp"
#include "PlayerCreationWidget.hpp"
#include <QApplication>
#include "DataStore.hpp"


namespace UDJ{


PlayerCreateDialog::PlayerCreateDialog(DataStore *dataStore, QWidget *parent, Qt::WindowFlags f)
  :DialogWithLoaderWidget(
      tr("Creating Player..."),
      tr("Create Player"),
      tr("Cancel"),
      true,
      parent, 
      f),
  dataStore(dataStore)
{
  setWindowTitle(tr("Player Setup"));
  setModal(true);
  setupUi();
}

void PlayerCreateDialog::accept(){
  showLoadingText();
  createWidget->doCreation();
}

void PlayerCreateDialog::reject(){
  QDialog::reject();
  QApplication::quit();
}


void PlayerCreateDialog::setupUi(){
  createWidget = new PlayerCreationWidget(dataStore, this);
  setMainWidget(createWidget);

  setNegativeButtonEnabled(false);

  connect(
    createWidget, 
    SIGNAL(playerCreated()), 
    this, 
    SLOT(closeDialog()));

  connect(
    createWidget,
    SIGNAL(playerCreateFailed()),
    this,
    SLOT(showMainWidget()));
}



} //end namespace UDJ

