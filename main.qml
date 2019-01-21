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
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

Window {
    visible: true
    width: 800
    height: 600
    title: qsTr("harfbuzz-qml")

    FocusScope {
        id: focusScope
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 5
        anchors.topMargin: 5
        anchors.fill: parent

        ColumnLayout {
            id: columnLayout
            anchors.fill: parent
            ToolBar {
                id: toolBar
                width: 360
                Layout.fillWidth: true
                Row {
                    id: row
                    anchors.fill: parent
                    Column {
                        spacing: 0
                        Label {
                            text: "General Font"
                            anchors.horizontalCenter: fontBox.horizontalCenter
                        }
                        ComboBox {
                            id: fontBox
                            editable: true
                            model: ["DejaVu Sans", "Arial", "Wingdings"]
                        }
                    }
                    Column {
                        spacing: 0
                        Label {
                            text: "Size"
                            anchors.horizontalCenter: sizeBox.horizontalCenter
                        }
                        ComboBox {
                            id: sizeBox
                            model: 20
                            currentIndex: 10
                        }
                    }
                }
            }
            Column {
                id: easySettings
                Layout.alignment: Qt.AlignHCenter
                spacing: 8
                Label {
                    text: "Select suiting font rendering"
                    padding: 2
                }
                Frame {
                    id: previewFrame
                    Grid {
                        id: previewArea
                        rows: 2
                        spacing: 4

                        Repeater {
                            model: 10
                            SelectablePreview {
                                fontFamily: fontBox.currentText
                                fontSize: sizeBox.currentText
                                antialiasing: index % 5 == 0 ? 1 : 2
                                hintstyle: index == 0 ? 0 : ((index + 2) % 4)+1
                                subpixel: index % 5 <= 2 ? 1 : 2
                            }
                        }
                        ButtonGroup {
                            id: previewButtons
                        }
                    }
                } // Column
                Row {
                    Label {
                        text: "Expert Settings"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    RoundButton {
                        id: roundButton
                        text: " + "
                        focusPolicy: Qt.TabFocus
                        display: AbstractButton.TextOnly
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        onClicked: {
                            if (expertSettings.visible) {
                                //previewFrame.visible = 1
                                expertSettings.visible = 0
                                roundButton.text = " + "
                            } else {
                                //previewFrame.visible = 0
                                expertSettings.visible = 1
                                roundButton.text = " - "
                            }
                        }
                    }
                }
            }
            Grid {
                columns: 2
                id: expertSettings
                Layout.alignment: Qt.AlignHCenter
                //visible: false
                Label {
                    text: "Anti-Aliasing"
                }
                ComboBox {
                    model: ["default", "disabled", "enabled"]
                }
                Label {
                    text: "Hinting"
                }
                ComboBox {
                    model: ["default", "none", "slight", "medium", "full"]
                }
                Label {
                    text: "Sub-Pixel Rendering"
                }
                ComboBox {
                    id: subpixelbox
                    model: ["default", "none", "rgb", "bgr", "vrgb", "vbgr"]
                    onAccepted: {
                        for (button in previewButtons.buttons) {
                            if (button.subpixel >= 1) {
                                // only set buttons where subpixel rendering is activated
                                button.subpixel = index
                            }
                        }
                    }
                }
            }
        }
    }
}

/*##^## Designer {
    D{i:4;anchors_height:100;anchors_width:100;anchors_x:140;anchors_y:78}D{i:3;anchors_height:255;anchors_width:333;anchors_x:98;anchors_y:108}
}
 ##^##*/

