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
#ifndef MUSIC_MODEL_HPP
#define MUSIC_MODEL_HPP
#include <QSqlQueryModel>

namespace UDJ{

class DataStore;

/**
 * \brief An abstract class representing a model that contains music.
 */
class MusicModel : public QSqlQueryModel{
Q_OBJECT
public:

  /** @name Constructors */
  //@{

  /**
   * \brief Constructs a Music Model
   *
   * \param query Query used to obtain the actual data in the model.
   * \param dataStore The datastore backing the client.
   * \param parent The parent object.
   */
  MusicModel(const QString& query, DataStore *dataStore, QObject *parent);

  //@}

  /** @name Overridden from QSqlQueryModel */
  //@{

  /** \brief . */
  virtual QVariant data(const QModelIndex& item, int role) const;

  //@}
  
public slots:
  /** @name Public Slots */
  //@{

  /**
   * \brief Refreshes the data in the model
   */
  void refresh();

  /**
   * \brief Refreshes the data in the model with a new query.
   *
   * \param query New query which should back the model.
   */
  void refresh(QString query);

  //@}

private:

  /** @name Private Memebers */
  //@{

  /** \brief DataStore backing the client */
  DataStore *dataStore;

  /** \brief Query used to populate the model with data. */
  QString query;

  //@}

};


}
#endif //MUSIC_MODEL_HPP
