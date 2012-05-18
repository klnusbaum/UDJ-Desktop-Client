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
#include "LogViewer.hpp"
#include "Logger.hpp"

namespace UDJ{

LogViewer::LogViewer(QWidget *parent):
  QTextEdit(parent)
{
  setReadOnly(true);
  Q_FOREACH(QString logLine, Logger::instance()->getLog()){
    append(logLine);
  }
  connect(
    Logger::instance(),
    SIGNAL(dataChanged(const QString&)),
    this,
    SLOT(updateText(const QString&)));
}

void LogViewer::updateText(const QString& newLogLine){
  append(newLogLine);
}



}
