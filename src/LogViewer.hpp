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
#ifndef LOG_VIEWER_HPP
#define LOG_VIEWER_HPP
#include <QTextEdit>

namespace UDJ{

/**
 * \brief A quick and dirty widget for viewing the contents of the log
 */
class LogViewer : public QTextEdit{
Q_OBJECT
public:
  /** @name Constructor(s) */
  //@{

  /**
   * \brief Constructs a LogViewer.
   *
   * \param parent The parent widget.
   */
  LogViewer(QWidget *parent=0);

  //@}

private slots:
  /** @name Private Slots */
  //@{

  /**
   * \brief Updates the text in the log view.
   *
   * \param newLogLine The new text to be added to the log view.
   */
  void updateText(const std::string& newLogLine);

  //@}
};

}
#endif //LOG_VIEWER_HPP
