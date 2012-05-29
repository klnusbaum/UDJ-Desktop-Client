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

#include "SetPasswordDialog.hpp"
#include "DataStore.hpp"
#include <QMessageBox>
#include <QLineEdit>
#include <QFormLayout>


namespace UDJ{


SetPasswordDialog::SetPasswordDialog(DataStore *dataStore, QWidget *parent, Qt::WindowFlags f)
  :DialogWithLoaderWidget(
    tr("Setting Player Password..."),
    tr("Set Password"),
    tr("Cancel"),
    false,
    parent,
    f),
  dataStore(dataStore)
{
  setWindowTitle(tr("Set Player Password"));
  setModal(true);
  setupUi();
  connect(dataStore, SIGNAL(playerPasswordSet()), this, SLOT(closeDialog()));
  connect(
    dataStore,
    SIGNAL(playerPasswordSetError(const QString&)),
    this,
    SLOT(onPlayerPasswordSetError(const QString&)));

}

void SetPasswordDialog::onPlayerPasswordSetError(const QString& errMessage){
  this->showMainWidget();
  QMessageBox::critical(
    this,
    tr("Couldn't Change Password"),
    errMessage
  );
}

void SetPasswordDialog::accept(){
  QString password = passwordEdit->text();
  if(password!=""){
    this->showLoadingText();
    dataStore->setPlayerPassword(password);
  }
  else{
    QMessageBox::critical(
      this,
      tr("Blank Password"),
      tr("The password must not be blank." )
    );
  }
}

void SetPasswordDialog::setupUi(){
  QWidget *passwordWidget = new QWidget();
  passwordEdit = new QLineEdit(dataStore->getPlayerPassword());
  QFormLayout *layout = new QFormLayout();
  layout->addRow(tr("Password:"), passwordEdit);
  passwordWidget->setLayout(layout);
  setMainWidget(passwordWidget);
}



} //end namespace UDJ


