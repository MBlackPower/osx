/* LocaleInformation_ga_IE.java
   Copyright (C) 2002 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


// This file was automatically generated by localedef.

package gnu.java.locale;

import java.util.ListResourceBundle;

public class LocaleInformation_ga_IE extends ListResourceBundle
{
  static final String decimalSeparator = ".";
  static final String groupingSeparator = ",";
  static final String numberFormat = "#,###,##0.###";
  static final String percentFormat = "#,###,##0%";
  static final String[] weekdays = { null, "D\u00E9 Domhnaigh", "D\u00E9 Luain", "D\u00E9 M\u00E1irt", "D\u00E9 C\u00E9adaoin", "D\u00E9ardaoin", "D\u00E9 hAoine", "D\u00E9 Sathairn" };

  static final String[] shortWeekdays = { null, "Domh", "Luan", "M\u00E1irt", "C\u00E9ad", "D\u00E9ar", "Aoine", "Sath" };

  static final String[] shortMonths = { "Ean", "Feabh", "M\u00E1rta", "Aib", "Beal", "Meith", "I\u00FAil", "L\u00FAn", "MF\u00F3mh", "DF\u00F3mh", "Samh", "Noll", null };

  static final String[] months = { "Ean\u00E1ir", "Feabhra", "M\u00E1rta", "Aibre\u00E1n", "M\u00ED na Bealtaine", "Meith", "I\u00FAil", "L\u00FAnasa", "Me\u00E1n F\u00F3mhair", "Deireadh F\u00F3mhair", "M\u00ED na Samhna", "M\u00ED na Nollag", null };

  static final String[] ampms = { "", "" };

  static final String shortDateFormat = "dd.MM.yy";
  static final String defaultTimeFormat = "";
  static final String currencySymbol = "\u00A3";
  static final String intlCurrencySymbol = "IEP";
  static final String currencyFormat = "$#,###,##0.00;-$#,###,##0.00";

  private static final Object[][] contents =
  {
    { "weekdays", weekdays },
    { "shortWeekdays", shortWeekdays },
    { "shortMonths", shortMonths },
    { "months", months },
    { "ampms", ampms },
    { "shortDateFormat", shortDateFormat },
    { "defaultTimeFormat", defaultTimeFormat },
    { "currencySymbol", currencySymbol },
    { "intlCurrencySymbol", intlCurrencySymbol },
    { "currencyFormat", currencyFormat },
    { "decimalSeparator", decimalSeparator },
    { "groupingSeparator", groupingSeparator },
    { "numberFormat", numberFormat },
    { "percentFormat", percentFormat },
  };

  public Object[][] getContents () { return contents; }
}
