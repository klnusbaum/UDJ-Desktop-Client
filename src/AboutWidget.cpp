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
#include "AboutWidget.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include "ConfigDefs.hpp"

namespace UDJ{

AboutWidget::AboutWidget(QWidget *parent):QWidget(parent){
  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->addWidget(new QLabel(tr("UDJ Player Client Version "  UDJ_VERSION)));
  mainLayout->addSpacing(1);
  mainLayout->addWidget(new QLabel(tr("Written By Kurtis Nusbaum")));
  mainLayout->addSpacing(1);
  QLabel *issuesLabel = new QLabel(tr("Please report all bugs to the <a href=\"https://github.com/klnusbaum/UDJ-Desktop-Client/issues\" >UDJ issue tracker</a>"));
  issuesLabel->setOpenExternalLinks(true);
  mainLayout->addWidget(issuesLabel);
  setLayout(mainLayout);
}



} //end namespace


