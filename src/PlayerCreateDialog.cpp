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
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include "DataStore.hpp"


namespace UDJ{


PlayerCreateDialog::PlayerCreateDialog(DataStore *dataStore, QWidget *parent, Qt::WindowFlags f)
  :QDialog(parent, f),
  dataStore(dataStore)
{
  setupUi();
}

void PlayerCreateDialog::accept(){
  createButton->hide();
  createWidget->doCreation();
}

void PlayerCreateDialog::reject(){
  QDialog::reject();
  QApplication::quit();
}

void PlayerCreateDialog::closeDialog(){
  done(QDialog::Accepted);
}

void PlayerCreateDialog::setupUi(){
  createWidget = new PlayerCreationWidget(dataStore, this);
  createButton = new QPushButton(tr("Create Player"), this);
  createButton->setDefault(true);
  createButton->setAutoDefault(true);

  connect(
    createWidget, 
    SIGNAL(playerCreateSuccess()), 
    this, 
    SLOT(closeDialog()));

  connect(
    createWidget,
    SIGNAL(playerCreateFail()),
    createButton,
    SLOT(show()));

  connect(
    createButton,
    SIGNAL(clicked()),
    this,
    SLOT(accept()));

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(createWidget, 0,0,3,3);
  layout->addWidget(createButton, 3,1);
  setLayout(layout);
}



} //end namespace UDJ

