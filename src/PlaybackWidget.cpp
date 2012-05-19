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

#include "PlaybackWidget.hpp"
#include <QAction>
#include <QLabel>
#include <QTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStyle>
#include <QDesktopServices>
#include <QFile>
#include "Logger.hpp"
#include <QMessageBox>

#ifdef WIN32
#include <mpegfile.h>
void removeTags(UDJ::DataStore::song_info_t& song){
  static int fileCount =0;
  if(song.source.fileName().endsWith(".mp3")){
    UDJ::Logger::instance()->log("On windows and got mp3, copying and striping metadata tags");
    QString tempCopy = QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/striped" + QString::number(fileCount) +".mp3";
    if(QFile::exists(tempCopy)){
      UDJ::Logger::instance()->log("Prevoius file existed, deleting now");
      if(QFile::remove(tempCopy)){
        UDJ::Logger::instance()->log("File removal worked");
      }
    }
    bool fileCopyWorked = QFile::copy(song.source.fileName(), tempCopy);
    if(!fileCopyWorked){
      UDJ::Logger::instance()->log("File copy didn't work");
      return;
    }

    TagLib::MPEG::File file(tempCopy.toStdString().c_str()); 
    file.strip();
    file.save();
    Phonon::MediaSource newSource(tempCopy);
    song.source = newSource;
    if(fileCount == 3){
      fileCount =0;
    }
    else{
      fileCount++;
    }

  }

}
#endif


namespace UDJ{



PlaybackWidget::PlaybackWidget(DataStore *dataStore, QWidget *parent):
  QWidget(parent), dataStore(dataStore), currentPlaybackState(PLAYING)
{
  audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
  mediaObject = new Phonon::MediaObject(this);
  createActions();
  setupUi();
  Phonon::createPath(mediaObject, audioOutput);

  audioOutput->setVolume(dataStore->getPlayerVolume());

  mediaObject->setTickInterval(1000);

  connect(
    audioOutput,
    SIGNAL(volumeChanged(qreal)),
    dataStore,
    SLOT(changeVolumeSilently(qreal)));

  connect(
    dataStore,
    SIGNAL(volumeChanged(qreal)),
    audioOutput,
    SLOT(setVolume(qreal)));


  connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
  connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
    this, SLOT(stateChanged(Phonon::State, Phonon::State)));
  connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
    this, SLOT(sourceChanged(Phonon::MediaSource)));
  connect(mediaObject, SIGNAL(finished()), this, SLOT(playNextSong()));
  connect(
    mediaObject,
    SIGNAL(metaDataChanged()),
    this,
    SLOT(metaDataChanged()));
  connect(
    dataStore,
    SIGNAL(manualSongChange(DataStore::song_info_t)),
    this,
    SLOT(setNewSource(DataStore::song_info_t)));

  connect(
    dataStore,
    SIGNAL(activePlaylistModified()),
    this,
    SLOT(handlePlaylistChange()));


  connect(
    dataStore,
    SIGNAL(playerStateChanged(const QString&)),
    this,
    SLOT(onPlayerStateChanged(const QString&)));


}

void PlaybackWidget::tick(qint64 time){
  QTime tickTime(0, (time/60000)%60, (time/1000)%60);
  timeLabel->setText(tickTime.toString("mm:ss"));
}

void PlaybackWidget::sourceChanged(const Phonon::MediaSource &source){

}

void PlaybackWidget::metaDataChanged(){

}

void PlaybackWidget::stateChanged(
  Phonon::State newState, Phonon::State oldState)
{
  if(newState == Phonon::ErrorState){
    Logger::instance()->log("Playback error: " + mediaObject->errorString());
    if(mediaObject->errorType() == Phonon::FatalError){
      QMessageBox::critical(this, "Bad Song", "Ooops. Looks like we're having some trouble playing the current song. Can you pick another song?");
    }
  }

}

void PlaybackWidget::playNextSong(){
  DataStore::song_info_t nextSong = dataStore->takeNextSongToPlay();
  #ifdef WIN32
  removeTags(nextSong);
  #endif
  mediaObject->setCurrentSource(nextSong.source);
  if(nextSong.source.type() != Phonon::MediaSource::Empty){
    mediaObject->play();
    songInfo->setText(nextSong.title + " - " + nextSong.artist);
  }
}

void PlaybackWidget::handlePlaylistChange(){
  if((mediaObject->currentSource().type() == Phonon::MediaSource::Empty ||
      mediaObject->currentSource().type() == Phonon::MediaSource::Invalid) &&
      currentPlaybackState != PAUSED)
  {
    playNextSong();
  }
}

void PlaybackWidget::setupUi(){

  songInfo = new QLabel(this);
  timeLabel = new QLabel("--:--", this);

  QToolBar *bar = new QToolBar;
  bar->addAction(playAction);
  bar->addAction(pauseAction);

  volumeSlider = new Phonon::VolumeSlider(this);
  volumeSlider->setAudioOutput(audioOutput);
  volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  seekSlider = new Phonon::SeekSlider(this);
  seekSlider->setMediaObject(mediaObject);

  QHBoxLayout *infoLayout = new QHBoxLayout;
  infoLayout->addWidget(songInfo);
  infoLayout->addStretch();
  infoLayout->addWidget(timeLabel);

  QHBoxLayout *playBackLayout = new QHBoxLayout;
  playBackLayout->addWidget(bar);
  playBackLayout->addStretch();
  playBackLayout->addWidget(volumeSlider);

  QHBoxLayout *seekerLayout = new QHBoxLayout;
  seekerLayout->addWidget(seekSlider);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(infoLayout);
  mainLayout->addLayout(seekerLayout);
  mainLayout->addLayout(playBackLayout);
  setLayout(mainLayout);

}

void PlaybackWidget::play(){
  currentPlaybackState = PLAYING;
  mediaObject->play();
  playAction->setEnabled(false);
  pauseAction->setEnabled(true);
}

void PlaybackWidget::pause(){
  currentPlaybackState = PAUSED;
  mediaObject->pause();
  playAction->setEnabled(true);
  pauseAction->setEnabled(false);
}

void PlaybackWidget::onPlayerStateChanged(const QString& newState){
  if(newState == DataStore::getPlayingState()){
    play();
  }
  else if(newState == DataStore::getPausedState()){
    pause();
  }
}


void PlaybackWidget::createActions(){
  playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay),
    tr("Play"), this);
  playAction->setShortcut(tr("Ctrl+P"));
  playAction->setEnabled(false);
  pauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause),
    tr("Pause"), this);
  pauseAction->setShortcut(tr("Ctrl+A"));

  connect(playAction, SIGNAL(triggered()), dataStore, SLOT(playPlayer()));
  connect(pauseAction, SIGNAL(triggered()), dataStore, SLOT(pausePlayer()));
}

void PlaybackWidget::setNewSource(DataStore::song_info_t newSong){
  #ifdef WIN32
  //Phonon on windows doesn't like compressed id3 tags. so we have to
  //uncrompress them. Tis a bitch.
  removeTags(newSong);
  #endif
  Logger::instance()->log("in set new source");
  mediaObject->setCurrentSource(newSong.source);
  songInfo->setText(newSong.title + " - " + newSong.artist);
  if(dataStore->getPlayingState() == DataStore::getPausedState()){
    dataStore->playPlayer();
  }
  else{
    mediaObject->play();
  }
}

void PlaybackWidget::clearWidget(){
  mediaObject->stop();
  mediaObject->clear();
  songInfo->setText("");
  timeLabel->setText("--:--");
}



} //end namespace UDJ
