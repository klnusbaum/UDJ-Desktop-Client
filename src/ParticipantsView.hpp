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
#ifndef PARTICIPANTS_VIEW_HPP
#define PARTICIPANTS_VIEW_HPP
#include "ConfigDefs.hpp"
#include "DataStore.hpp"
#include <QTableView>



namespace UDJ{

class ParticipantsModel;

class ParticipantsView : public QTableView{
Q_OBJECT
public:

  /** @name Constructors */
  //@{

  ParticipantsView(DataStore* dataStore, QWidget* parent=0);

  //@}

private:


  /** @name Private Members */
  //@{

  /**
   * \brief The data store containing music that could potentially be added
   * to the playlist.
   */
  DataStore* dataStore;

  ParticipantsModel *participantsModel;

  //@}

  /** @name Private Functions */
  //@{

  /**
   * \brief Configures how the headers in the view should look.
   */
  void configureHeaders();

  //@}

};


} //end namespace
#endif //PARTICIPANTS_VIEW_HPP
