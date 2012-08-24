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
#include "AddressWidget.hpp"
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>


namespace UDJ{


AddressWidget::AddressWidget(
    QWidget *parent,
    QString streetAddress,
    QString city,
    QString state,
    QString zipcode
  ):QWidget(parent)
{
  setupStateCombo(state);
  streetAddressEdit = new QLineEdit(streetAddress);
  cityEdit = new QLineEdit(city);
  setupStateCombo(state);
  zipcodeEdit = new QLineEdit(zipcode);
  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow(tr("Address (Optional):"), streetAddressEdit);
  formLayout->addRow(tr("City (Optional):"), cityEdit);
  formLayout->addRow(tr("State:"), stateCombo);
  formLayout->addRow(tr("Zipcode:"), zipcodeEdit);
  setLayout(formLayout);
}


QString AddressWidget::getStreetAddress() const{
  return streetAddressEdit->text();
}

QString AddressWidget::getCity() const{
  return cityEdit->text();
}

QString AddressWidget::getState() const{
  return stateCombo->currentText();
}

QString AddressWidget::getZipcode() const{
  return zipcodeEdit->text();
}


QString AddressWidget::getBadInputs() const{
  QString toReturn ="";
/*  int errorCounter = 1;
  if(streetAddressEdit->text() == ""){
    toReturn += QString::number(errorCounter++) + 
      ". You did not enter a street address.\n";
  }
  if(cityEdit->text() == ""){
    toReturn += QString::number(errorCounter++) + 
      ". You did not enter a city.\n";
  }
  if(!getZipcodeRegex().exactMatch(zipcodeEdit->text())){
    toReturn += QString::number(errorCounter++) + 
      ". Zipcode invalid.";
  }*/
  return toReturn;
}

void AddressWidget::setupStateCombo(const QString& selectedState){
  stateCombo = new QComboBox();
  stateCombo->addItem("AL");
  stateCombo->addItem("AK");
  stateCombo->addItem("AZ");
  stateCombo->addItem("AR");
  stateCombo->addItem("CA");
  stateCombo->addItem("CO");
  stateCombo->addItem("CT");
  stateCombo->addItem("DE");
  stateCombo->addItem("DC");
  stateCombo->addItem("FL");
  stateCombo->addItem("GA");
  stateCombo->addItem("HI");
  stateCombo->addItem("ID");
  stateCombo->addItem("IL");
  stateCombo->addItem("IN");
  stateCombo->addItem("IA");
  stateCombo->addItem("KS");
  stateCombo->addItem("KY");
  stateCombo->addItem("LA");
  stateCombo->addItem("ME");
  stateCombo->addItem("MT");
  stateCombo->addItem("NE");
  stateCombo->addItem("NV");
  stateCombo->addItem("NH");
  stateCombo->addItem("NJ");
  stateCombo->addItem("NM");
  stateCombo->addItem("NY");
  stateCombo->addItem("NC");
  stateCombo->addItem("ND");
  stateCombo->addItem("OH");
  stateCombo->addItem("OK");
  stateCombo->addItem("OR");
  stateCombo->addItem("MD");
  stateCombo->addItem("MA");
  stateCombo->addItem("MI");
  stateCombo->addItem("MN");
  stateCombo->addItem("MS");
  stateCombo->addItem("MO");
  stateCombo->addItem("PA");
  stateCombo->addItem("RI");
  stateCombo->addItem("SC");
  stateCombo->addItem("SD");
  stateCombo->addItem("TN");
  stateCombo->addItem("TX");
  stateCombo->addItem("UT");
  stateCombo->addItem("VT");
  stateCombo->addItem("VA");
  stateCombo->addItem("WA");
  stateCombo->addItem("WV");
  stateCombo->addItem("WI");
  stateCombo->addItem("WY");

  if(selectedState != "" && stateCombo->findText(selectedState) != -1){
    stateCombo->setCurrentIndex(stateCombo->findText(selectedState));
  }
}

}//end namespace UDJ

