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


class Logger : public QObject{
Q_OBJECT
public:
  static Logger* instance();

  void log(std::string message);

  static void deleteLogger();

  QList<std::string> getLog();

signals:
  void dataChanged(const std::string& loggerData);

private:
  Logger(){};
  Logger(Logger const&){};
  Logger& operator=(Logger const&){};
  static Logger* myInstance;
  QList<std::string> data;
};


}
#endif //LOGGER_HPP_
