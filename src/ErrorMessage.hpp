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
#ifndef ERROR_MESSAGE_HPP
#define ERROR_MESSAGE_HPP

#include <QDialog>

class QPushButton;
class QLabel;
class QCheckBox;

namespace UDJ{

class DataStore;


class ErrorMessage : public QDialog{
Q_OBJECT
public:
  ErrorMessage(const QString& title, const QString& message, 
  QWidget *parent=0, Qt::WindowFlags f=0);

  //@}

private:

  /** @name Private Memeber */
  //@{

  QLabel *errorMessage;

  QPushButton *okButton;

  QCheckBox *dontShowAgain;


  //@}

private slots:
  void dontShowAgainChecked(bool checked);
};


} //end namespace UDJ


#endif //ERROR_MESSAGE_HPP




