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
#include "LibraryView.hpp"
#include "Utils.hpp"
#include "MusicModel.hpp"
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSortFilterProxyModel>
#include <QProgressDialog>
#include <QMessageBox>

namespace UDJ{


LibraryView::LibraryView(DataStore *dataStore, QWidget* parent):
  QTableView(parent),
  dataStore(dataStore)
{
  libraryModel = new MusicModel(getDataQuery(), dataStore, this);
  proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(libraryModel);
  proxyModel->setFilterKeyColumn(-1);
  proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);


  verticalHeader()->hide();
  horizontalHeader()->setStretchLastSection(true);
  setModel(proxyModel);
  setSortingEnabled(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setContextMenuPolicy(Qt::CustomContextMenu);
  configureColumns();
  createActions();
  connect(dataStore, SIGNAL(libSongsModified()), libraryModel, SLOT(refresh()));
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
    this, SLOT(handleContextMenuRequest(const QPoint&)));
  /*connect(
    this,
    SIGNAL(activated(const QModelIndex& index)),
    this,
    SLOT(addSongToPlaylist(const QModelIndex& index)));*/
}

void LibraryView::configureColumns(){
  QSqlRecord record = libraryModel->record();
  int idIndex = record.indexOf(DataStore::getLibIdColName());
  int isDeletedIndex = record.indexOf(DataStore::getLibIsDeletedColName());
  int syncStatusIndex = record.indexOf(DataStore::getLibSyncStatusColName());
  int durationIndex = record.indexOf(DataStore::getLibDurationColName());
  setColumnHidden(idIndex, true);
  setColumnHidden(isDeletedIndex, true); 
  setColumnHidden(syncStatusIndex, true); 
  resizeColumnToContents(durationIndex);
}


void LibraryView::createActions(){
  deleteSongAction = new QAction(getDeleteContextMenuItemName(), this);
  connect(
    deleteSongAction, 
    SIGNAL(triggered()), 
    this, 
    SLOT(deleteSongs()));
}


void LibraryView::handleContextMenuRequest(const QPoint &pos){
  QMenu contextMenu(this);
  contextMenu.addAction(deleteSongAction);
  contextMenu.exec(QCursor::pos());
}


void LibraryView::deleteSongs(){
  deletingProgress = new QProgressDialog(tr("Deleting Songs..."), tr("Cancel"), 0,0, this);
  deletingProgress->setWindowModality(Qt::WindowModal);

  connect(
    dataStore,
    SIGNAL(libSongsModified()),
    this,
    SLOT(deletingDone()));

  connect(
    dataStore,
    SIGNAL(libModError(const QString&)),
    this,
    SLOT(deletingError(const QString&)));

  dataStore->removeSongsFromLibrary(
    Utils::getSelectedIds<library_song_id_t>(
      this,
      libraryModel,
      DataStore::getLibIdColName(),
      proxyModel));
}

void LibraryView::deletingDone(){
  disconnect(
    dataStore,
    SIGNAL(libSongsModified()),
    this,
    SLOT(deletingDone()));

  disconnect(
    dataStore,
    SIGNAL(libModError(const QString&)),
    this,
    SLOT(deletingError(const QString&)));

  deletingProgress->close();
}

void LibraryView::deletingError(const QString& errMessage){
  disconnect(
    dataStore,
    SIGNAL(libSongsModified()),
    this,
    SLOT(deletingDone()));

  disconnect(
    dataStore,
    SIGNAL(libModError(const QString&)),
    this,
    SLOT(deletingError(const QString&)));

  deletingProgress->close();

  QMessageBox::critical(
    this,
    tr("Error"),
    tr("Error deleting songs from library. Try again in a little bit"));
}


void LibraryView::filterContents(const QString& filter){
  proxyModel->setFilterFixedString(filter);
}

void LibraryView::addSongToPlaylist(const QModelIndex& index){
  QModelIndex realIndex = proxyModel->mapToSource(index);
  QSqlRecord selectedRecord = libraryModel->record(realIndex.row());
  dataStore->addSongToActivePlaylist(
    selectedRecord.value(DataStore::getLibIdColName()).value<library_song_id_t>());
}


}//end namespace
