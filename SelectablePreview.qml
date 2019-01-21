/*
 * Copyright 2018 Max Harmathy
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.11
import QtQuick.Controls 2.3
import QtQml 2.11


Button {
    property string fontFamily: "Sans"
    property double fontSize: 10
    property int antialiasing: 0
    property int hintstyle: 0
    property int subpixel: 0
    icon.color: "transparent" // makes the actual image visible
    icon.source: "image://renderpreview/" + fontFamily + "/" + fontSize + "/" + antialiasing + "/" + hintstyle + "/" + subpixel
}
