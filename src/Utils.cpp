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

QString getDebugFileName(){
  QDir dataDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
  return dataDir.absoluteFilePath("debug_file.txt");
}

void writeToDebugFile(std::string debugMessage){
  QFile debugFile(getDebugFileName());
  debugFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  debugFile.write(QString::fromStdString(debugMessage + "\n").toUtf8());
  debugFile.close();
}

void clearDebugFile(){
  QFile::remove(getDebugFileName());
}


} //End namespace Utils


} //End namespace UDJ

