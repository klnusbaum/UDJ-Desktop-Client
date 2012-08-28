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
#include "ParticipantsModel.hpp"
#include "DataStore.hpp"
#include "Logger.hpp"


namespace UDJ{

ParticipantsModel::ParticipantsModel(DataStore* dataStore, QObject *parent)
  :QStandardItemModel(parent),
  dataStore(dataStore)
{
  setHeaders();
  createConnections();
}

void ParticipantsModel::createConnections(){
  connect(
    dataStore,
    SIGNAL(newParticipantList(const QVariantList&)),
    this,
    SLOT(onNewParticipantList(const QVariantList&)));
}


void ParticipantsModel::onNewParticipantList(const QVariantList& newParticipants){
  Logger::instance()->log("Got new particpants list of length: " + QString::number(newParticipants.size()));
  clear();
  setHeaders();
  QVariantMap participant;
  for(int i=0; i<newParticipants.size(); ++i){
    participant = newParticipants.at(i).toMap();
    QStandardItem *newId = new QStandardItem(participant["id"].toString());
    QStandardItem *newUsername = new QStandardItem(participant["username"].toString());
    QStandardItem *newFirstName = new QStandardItem(participant["first_name"].toString());
    QStandardItem *newLastName = new QStandardItem(participant["last_name"].toString());
    QList<QStandardItem*> newRow;
    newRow << newId << newUsername << newFirstName << newLastName;
    appendRow(newRow);
  }

}

void ParticipantsModel::setHeaders(){
  QStringList headers;
  headers << tr("Id") << tr("Username") << tr("First Name") << tr("Last Name");
  setHorizontalHeaderLabels(headers);
}

} // end namespace
