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
#ifndef PLAYER_DASHBOARD_HPP
#define PLAYER_DASHBOARD_HPP
#include <QWidget>

class QLabel;

namespace UDJ{

class DataStore;


class PlayerDashboard : public QWidget{
  Q_OBJECT
public:
  PlayerDashboard(DataStore *dataStore, QWidget *parent=0);

private slots:
  void onPlayerCreation();

private:
  DataStore *dataStore;
  QLabel *nameLabel;
  QLabel *locationLabel;

  void setupUi();
  void setPlayerInfo();
};


} //end namespace 
#endif //PLAYER_DASHBOARD_HPP

