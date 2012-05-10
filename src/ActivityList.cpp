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

#include "ActivityList.hpp"
#include "DataStore.hpp"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QTextEdit>

namespace UDJ{

ActivityList::ActivityList(DataStore *dataStore, QWidget *parent):
  QTreeView(parent), dataStore(dataStore)
{
  setupUi();
  connect(
    this,
    SIGNAL(clicked(const QModelIndex&)),
    this,
    SLOT(itemClicked(const QModelIndex&)));
}


void ActivityList::itemClicked(const QModelIndex& index){
  if(index == libraryItem->index()){
    emit libraryClicked();
  }
  else if(index == playlistItem->index()){
    emit playlistClicked();
  }
}

void ActivityList::setupUi(){
  setSelectionMode(QAbstractItemView::SingleSelection);
  libraryItem = new QStandardItem(getLibraryTitle());
  libraryItem->setEditable(false);

  playlistItem = new QStandardItem(getPlaylistTitle());
  playlistItem->setEditable(false);


  model = new QStandardItemModel(this);
  model->appendRow(libraryItem);
  model->appendRow(playlistItem);


  setModel(model);
  header()->hide();
}

void ActivityList::switchToLibrary(){
  setCurrentIndex(libraryItem->index());
  emit libraryClicked();
}


}// end namespace UDJ
