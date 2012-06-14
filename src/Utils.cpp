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

#include <QDir>
#include <QDesktopServices>
#include "simpleCrypt/simplecrypt.h"
#include "ConfigDefs.hpp"
#include <QDateTime>
#include <QFile>

namespace UDJ{
namespace Utils{

//Yea yea, I Know. Not very cryptographically secure. But honestly, if your machine
//has been comprised and someone can get at your key file, you have MUCH bigger problems.
SimpleCrypt getCryptoObject(){
  QDir keyDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
  if(!keyDir.exists()){
    //TODO handle if this fails
    keyDir.mkpath(keyDir.absolutePath());
  }
  QString credKeyFilePath = keyDir.absoluteFilePath("credKey");
  QFile credKeyFile(credKeyFilePath);
  if(credKeyFile.exists(credKeyFilePath)){
    credKeyFile.open(QIODevice::ReadOnly);
    QByteArray keyArray = credKeyFile.readAll();
    credKeyFile.close();
    return SimpleCrypt(keyArray.toULongLong());
  }
  else{
    qsrand(QDateTime::currentMSecsSinceEpoch());
    quint64 random = qrand();
    QByteArray randomArray = QByteArray::number(random);
    credKeyFile.open(QIODevice::WriteOnly);
      credKeyFile.write(randomArray);
      credKeyFile.close();
    return SimpleCrypt(random);
  }
}


} //End namespace Utils


} //End namespace UDJ

