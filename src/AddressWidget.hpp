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
#ifndef ADDRESS_WIDGET_HPP
#define ADDRESS_WIDGET_HPP

#include <QWidget>
#include <QRegExp>

class QLineEdit;
class QComboBox;

namespace UDJ{


/** \brief An input widget for getting addresses */
class AddressWidget : public QWidget{
Q_OBJECT
public:
  /** @name Constructors */
  //@{

  /**
   * \brief Constructs an AddressWidget.
   *
   * @param parent The parent widget.
   * @param streetAddress The street address to be displayed.
   * @param city The city to be displayed.
   * @param state The state to be selected.
   * @param zipcode the current zipcode.
   */
  AddressWidget(
    QWidget *parent=0,
    QString streetAddress="",
    QString city="",
    QString state="",
    QString zipcode=""
  );


  //@}

  /** @name Getters */
  //@{

  QString getStreetAddress() const;
  QString getCity() const;
  QString getState() const;
  QString getZipcode() const;

  QString getBadInputs() const;
  //@}

private:

  /** \brief LineEdit used to specify the street address of the player */
  QLineEdit *streetAddressEdit;

  /** \brief LineEdit used to specify the city of the player */
  QLineEdit *cityEdit;

  /** \brief ComboBox used to specify the state of the player */
  QComboBox *stateCombo;

  /** \brief LineEdit used to specify the zipcode of the player */
  QLineEdit *zipcodeEdit;


  /**
   * \brief Retrieves the regex for validating the zipcode.
   *
   * \return The regex for validating the zipcode.
   */
  static const QRegExp& getZipcodeRegex(){
    //We're keeping it simple for now.
    //static const QRegExp zipcodeRegex("^\\d{5}(-\\d{4})?$");
    static const QRegExp zipcodeRegex("^\\d{5}$");
    return zipcodeRegex;
  }

  /** \brief Sets up all the options for the state combo box. */
  void setupStateCombo(const QString& selectedState);
};


}//end namespace UDJ

#endif //ADDRESS_WIDGET_HPP
