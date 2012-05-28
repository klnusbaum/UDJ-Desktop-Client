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

#include "DialogWithLoaderWidget.hpp"
#include "WidgetWithLoader.hpp"
#include "DataStore.hpp"
#include "Logger.hpp"
#include <QPushButton>
#include <QGridLayout>


namespace UDJ{

DialogWithLoaderWidget::DialogWithLoaderWidget(
  QString loadingText,
  QString positiveButtonText,
  QString negativeButtonText,
  QWidget *parent,
  Qt::WindowFlags f):QDialog(parent, f)
{
  setupUi(loadingText, positiveButtonText, negativeButtonText);
}

void DialogWithLoaderWidget::setNegativeButtonEnabled(bool enabled){
  negativeButtonEnabled = enabled;
  if(negativeButtonEnabled && loaderWidget->isMainWidgetShowing()){
    negativeButton->show();
  }
  else{
    negativeButton->hide();
  }
}

void DialogWithLoaderWidget::showLoadingText(){
  loaderWidget->showLoadingText();
  negativeButton->hide();
  positiveButton->hide();
}

void DialogWithLoaderWidget::showMainWidget(){
  loaderWidget->showMainWidget();
  if(negativeButtonEnabled){
    negativeButton->show();
  }
  positiveButton->show();
}

void DialogWithLoaderWidget::setMainWidget(QWidget *mainWidget){
  loaderWidget->setMainWidget(mainWidget);
  loaderWidget->showMainWidget();
}

void DialogWithLoaderWidget::closeDialog(){
  done(QDialog::Accepted);
}

void DialogWithLoaderWidget::setupUi(
  QString loadingText, QString positiveText, QString negativeText)
{

  positiveButton = new QPushButton(positiveText);
  negativeButton = new QPushButton(negativeText);
  positiveButton->setDefault(true);
  positiveButton->setAutoDefault(true);

  loaderWidget = new WidgetWithLoader(loadingText, this);

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget(loaderWidget, 0,0,3,3);
  mainLayout->addWidget(negativeButton, 4,1);
  mainLayout->addWidget(positiveButton, 4,2);

  setLayout(mainLayout);

  connect(
    positiveButton,
    SIGNAL(clicked()),
    this,
    SLOT(accept()));

  connect(
    negativeButton,
    SIGNAL(clicked()),
    this,
    SLOT(reject()));

}



} //end namespace UDJ


