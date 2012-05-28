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
#include "WidgetWithLoader.hpp"
#include "DataStore.hpp"
#include "AddressWidget.hpp"
#include "Logger.hpp"
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>


namespace UDJ{


SetLocationDialog::SetLocationDialog(DataStore *dataStore, QWidget *parent, Qt::WindowFlags f)
  :QDialog(parent, f),
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
  loaderContainer->showMainWidget();
  QMessageBox::critical(
    this,
    tr("Couldn't Change Location"),
    errMessage
  );
}

void SetLocationDialog::accept(){
  Logger::instance()->log("in accept for setting location dialog");
  QString badInputs = addressWidget->getBadInputs();
  if(badInputs == ""){
    loaderContainer->showLoadingText();
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

void SetLocationDialog::closeDialog(){
  done(QDialog::Accepted);
}

void SetLocationDialog::setupUi(){
  QWidget* containerWidget = new QWidget(this);
  if(dataStore->hasLocation()){
    addressWidget = new AddressWidget(
      containerWidget,
      dataStore->getLocationStreetAddress(),
      dataStore->getLocationCity(),
      dataStore->getLocationState(),
      QString::number(dataStore->getLocationZipcode()));
  }
  else{
    addressWidget = new AddressWidget(containerWidget);
  }

  setLocationButton = new QPushButton(tr("Set Location"), containerWidget);
  cancelButton = new QPushButton(tr("Cancel"), containerWidget);
  setLocationButton->setDefault(true);
  setLocationButton->setAutoDefault(true);

  connect(
    setLocationButton,
    SIGNAL(clicked()),
    this,
    SLOT(accept()));

  connect(
    cancelButton,
    SIGNAL(clicked()),
    this,
    SLOT(reject()));

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(addressWidget, 0,0,3,3);
  layout->addWidget(cancelButton, 4,1);
  layout->addWidget(setLocationButton, 4,2);
  containerWidget->setLayout(layout);

  loaderContainer = new WidgetWithLoader(tr("Setting Location..."), this);
  loaderContainer->setMainWidget(containerWidget);
  loaderContainer->showMainWidget();

  QGridLayout *containerLayout = new QGridLayout();
  containerLayout->addWidget(loaderContainer,0,0,1,1);
  setLayout(containerLayout);
}



} //end namespace UDJ


