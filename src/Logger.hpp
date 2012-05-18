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
#ifndef LOGGER_HPP_
#define LOGGER_HPP_
#include <QObject>
#include <QList>

namespace UDJ{


/**
 * \brief Singleton class used to keep a log of messages
 */
class Logger : public QObject{
Q_OBJECT
public:

  /** @name Creation/Destruction Functions */
  //@{

  /**
   * \brief Retrieves an instance of the logger.
   *
   * \return An instance of the logger.
   */
  static Logger* instance();

  /**
   * \brief Deletes the instance of the logger. This should only be called when the program
   * is finished and there is no more intent of logging things.
   */
  static void deleteLogger();

  //@}
  
  /** @name Log Functoins */
  //@{

  /**
   * \brief Adds the given message to the log.
   *
   * \param message Message to add to the log.
   */
  void log(std::string message);


  /**
   * \brief Gets the current log of messages.
   *
   * \return The current log of messages.
   */
  QList<std::string> getLog();

  //@}

signals:
  /** @name Signals */
  //@{

  /**
   * \brief Emitted when ever a message is added to the log.
   *
   * \param newLogLine The new message that was added to the log.
   */
  void dataChanged(const std::string& newLogLine);

  //@}

private:
  /** @name Constructor(s) */
  //@{

  /** \brief . */
  Logger(){};
  /** \brief . */
  Logger(Logger const&){};
  /** \brief . */
  Logger& operator=(Logger const&){};

  //@}

  /** Private Members */
  //@{

  /** \brief Singelton instance of the log */
  static Logger* myInstance;
  /** \brief Actual data in the log*/
  QList<std::string> data;

  //@}
};


}
#endif //LOGGER_HPP_
