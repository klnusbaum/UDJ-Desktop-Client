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
#ifndef DIALOG_WITH_LOADER_WIDGET_HPP
#define DIALOG_WITH_LOADER_WIDGET_HPP

#include <QDialog>
class QPushButton;

namespace UDJ{

class WidgetWithLoader;

/** \brief Dialog with a widget that should be put inside a loader widget. */
class DialogWithLoaderWidget : public QDialog{
Q_OBJECT
public:
  /** @name Constructors */
  //@{

  DialogWithLoaderWidget(
    QString loadingText,
    QString postiveButtonText=tr("Ok"),
    QString negativeButtonText=tr("Cancel"),
    QWidget *parent=0,
    Qt::WindowFlags f=0);

  //@}

  void setMainWidget(QWidget *mainWidget);

public slots:

  void showLoadingText();

  void showMainWidget();

  void closeDialog();

private:

  /** @name Private Members */
  //@{

  /** \brief Widget used for containing the actual input widget.*/
  WidgetWithLoader *loaderWidget;

  /** \brief Button used for indicating the postive action should be invoked. */
  QPushButton *positiveButton;

  /** \brief Button for indicating the negative action should be invoked. */
  QPushButton *negativeButton;

  //@}

  /** @name Private Functions */
  //@{

  /** \brief Initializes UI. 
   *
   * \param loadingText The text to show while loading.
   * \param positiveText The text to display on the positive button.
   * \param negativeText The text to diplay on the negative button.
   */
  void setupUi(QString loadingText, QString positiveText, QString negativeText);


  //@}

};


} //end namespace UDJ


#endif //DIALOG_WITH_LOADER_WIDGET_HPP
