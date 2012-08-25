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

#include "PlaybackErrorMessage.hpp"
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include "DataStore.hpp"


namespace UDJ{

PlaybackErrorMessage::PlaybackErrorMessage(const QString& title, const QString& message, 
  QWidget *parent, Qt::WindowFlags f):
  QDialog(parent, f)
{
  setWindowTitle(title);
  errorMessage = new QLabel(message, this);
  errorMessage->setWordWrap(true);
  okButton = new QPushButton(tr("Ok"), this); 
  dontShowAgain = new QCheckBox(tr("Don't show this message again"));

  QGridLayout *layout = new QGridLayout();
  layout->addWidget(errorMessage, 0,0,1,3);
  layout->addWidget(dontShowAgain, 1,0,1,1);
  layout->addWidget(okButton, 1,2,1,1);

  setLayout(layout);

  setModal(true);

  connect(
    okButton,
    SIGNAL(clicked()),
    this,
    SLOT(accept()));

  connect(
    dontShowAgain,
    SIGNAL(toggled(bool)),
    this,
    SLOT(dontShowAgainChecked(bool)));

}


void PlaybackErrorMessage::dontShowAgainChecked(bool checked){
  DataStore::setDontShowPlaybackError(checked);
}


} //end Namespace
