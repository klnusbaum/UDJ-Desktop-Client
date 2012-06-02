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
#ifndef MUSIC_FINDER_HPP
#define MUSIC_FINDER_HPP

#include <QDir>
#include "phonon/mediasource.h"


namespace UDJ{

/**
 * \brief A class used to find music on a machine.
 */
class MusicFinder{
public:
  /** @name Finder Function(s) */
  //@{

  /**
   * \brief Finds all the music in a given iTunes library.
   *
   * This function parses the given iTunes library file and returns
   * a list of Phonon MediaSources representing all of the songs
   * which it found in the given iTunes library.
   *
   * @param itunesLibFileName The iTunes library file.
   * @return A list of MediaSources corresponding to all the songs found in the iTunes library.
   */

  static QList<Phonon::MediaSource> findItunesMusic(const QString& itunesLibFileName);


  /**
   * \brief Finds all the music in a given directory.
   *
   * Recusrively searchs the given directory and all subdirectories looking
   * for any music files to be added to the users music library. It then
   * returns a list of MediaSources representing all of the found songs.
   *
   * @param musicDir The directory in which to search for music.
   * @return A list of MediaSources corresponding to each found song.
   */
  static QList<Phonon::MediaSource> findMusicInDir(const QString& musicDir);

  /**
   * \brief Finds all the music in a given directory which matches certain criteria.
   *
   * Recusrively searchs the given directory and all subdirectories looking
   * for any music files to be added to the users music library. It then
   * returns a list of MediaSources representing all of the found songs. All the file
   * names of the found songs must match the QRegExp that is provided.
   *
   * @param musicDir The directory in which to search for music.
   * @param fileMatcher QRegExp used to determine if a file is a valid song.
   * @return A list of MediaSources corresponding to each found song.
   */
  static QList<Phonon::MediaSource> findMusicInDirWithMatcher(
      const QString& musicDir, const QRegExp& fileMatcher);

  /**
   * Examines the system on whitch the client is running, determines what types of music
   * can be played, and returns a filter for just those file types.
   *
   * @return A filter representing the file types of songs that can be played by the client.
   */
  static QString getMusicFileExtFilter();

  //@}
private:
  /** @name Private Function(s) */
  //@{

  /**
   * Retrieves the regular expression used to help determine if a file
   * constains music that can be played by the client.
   *
   * @return The regular expression user to help determine if a file
   * constains music that can be played by the client.
   */
  static QRegExp getMusicFileMatcher();

  /**
   * Retrieves a list of all file extensions that can be played by the client.
   *
   * @return A list of all file extensions that can be played by the client.
   */
  static QStringList availableMusicTypes();

  //@}
}; 


} //end namespace
#endif //MUSIC_FINDER_HPP
