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
#include <QApplication>
#include <QIcon>
#include "LoginDialog.hpp"
#include "ConfigDefs.hpp"
#include "Logger.hpp"

#ifdef HAS_CUSTOM_CA_CERT
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QFile>
#endif

int main(int argc, char* argv[]){

  QApplication app(argc, argv);
  //QIcon windowIcon("udjlauncher.svg");
  //QApplication::setWindowIcon(windowIcon);
  app.setApplicationName("Udj");
  app.setApplicationVersion(UDJ_VERSION);
  app.setQuitOnLastWindowClosed(true);
  UDJ::LoginDialog loginDialog;
  loginDialog.show(); 

  #ifdef HAS_CUSTOM_CA_CERT
  QFile servercaFile("serverca.pem");
  if(servercaFile.exists()){
    UDJ::Logger::instance()->log("Explicitly setting server cas");
    servercaFile.open(QIODevice::ReadOnly);
    QList<QSslCertificate> cas;
    QSslCertificate serverca(&servercaFile);
    cas.append(serverca);
    servercaFile.close();
    QSslConfiguration defaultConfig = QSslConfiguration::defaultConfiguration();
    defaultConfig.setCaCertificates(cas);
    QSslConfiguration::setDefaultConfiguration(defaultConfig);
  }
  #endif

  int toReturn = app.exec();
  UDJ::Logger::deleteLogger();
  return toReturn;
}
