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

#include "SetLocationDialog.hpp"
#include "DataStore.hpp"
#include "AddressWidget.hpp"
#include <QMessageBox>


namespace UDJ{


SetLocationDialog::SetLocationDialog(DataStore *dataStore, QWidget *parent, Qt::WindowFlags f)
  :DialogWithLoaderWidget(
    tr("Setting Player Location..."),
    tr("Set Location"),
    tr("Cancel"),
    parent,
    f),
  dataStore(dataStore)
{
  setWindowTitle(tr("Set Player Location"));
  setModal(true);
  setupUi();
  connect(dataStore, SIGNAL(playerLocationSet()), this, SLOT(closeDialog()));
  connect(
    dataStore,
    SIGNAL(playerLocationSetError(const QString&)),
    this,
    SLOT(onChangeLocationError(const QString&)));

}

void SetLocationDialog::onChangeLocationError(const QString& errMessage){
  this->showMainWidget();
  QMessageBox::critical(
    this,
    tr("Couldn't Change Location"),
    errMessage
  );
}

void SetLocationDialog::accept(){
  QString badInputs = addressWidget->getBadInputs();
  if(badInputs == ""){
    this->showLoadingText();
    dataStore->setPlayerLocation(
      addressWidget->getStreetAddress(),
      addressWidget->getCity(),
      addressWidget->getState(),
      addressWidget->getZipcode()
    );
  }
  else{
    QMessageBox::critical(
      this,
      tr("Bad Address"),
      tr("The address you supplied is invalid. Please correct " 
        "the following errors:\n\n") + badInputs
    );
  }
}

void SetLocationDialog::setupUi(){
  if(dataStore->hasLocation()){
    addressWidget = new AddressWidget(
      0,
      dataStore->getLocationStreetAddress(),
      dataStore->getLocationCity(),
      dataStore->getLocationState(),
      QString::number(dataStore->getLocationZipcode()));
  }
  else{
    addressWidget = new AddressWidget();
  }

  setMainWidget(addressWidget);
}



} //end namespace UDJ


