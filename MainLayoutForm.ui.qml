import QtQuick 2.8
import QtQuick.Controls 2.2
import QtQuick.Controls.Universal 2.2

Item {
    property alias firmwareChooseButton: firmwareChooseButton
    property alias downloadProgressBar: downloadProgressBar
    property alias downloadButton: downloadButton
    property alias serialPortCombo: serialPortCombo
    property alias packetLengthCombo: packetLengthCombo
    property alias crc8CheckCheckBox: crc8CheckCheckBox
    property alias sequentialNumberCheckBox: sequentialNumberCheckBox
    property alias fileInfo: fileInfo
    id: item1
    width: 480
    Button {
        id: firmwareChooseButton
        width: 160
        height: 50
        text: qsTr("选择固件")
        font.family: "微软雅黑"
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
    }
    Button {
        id: downloadButton
        width: firmwareChooseButton.width
        height: firmwareChooseButton.height
        text: qsTr("下载固件")
        font.family: firmwareChooseButton.font
        enabled: false
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.right: parent.right
        anchors.rightMargin: 20
    }
    ProgressBar {
        id: downloadProgressBar
        width: parent.width
        height: 6
        anchors.bottom: firmwareChooseButton.top
        anchors.bottomMargin: -80
    }
    Text {
        id: fileInfo
        text: qsTr("请选择需要烧写的固件")
        anchors.verticalCenterOffset: -120
        horizontalAlignment: Text.AlignHCenter
        anchors.centerIn: parent
        wrapMode: Text.WordWrap
        font.family: "微软雅黑"
        font.pixelSize: 20
    }
    ComboBox {
        id: serialPortCombo
        width: 160
        height: 32
        anchors.left: packetLengthCombo.left
        anchors.leftMargin: 0
        anchors.bottom: packetLengthCombo.top
        anchors.bottomMargin: 30
    }
    ComboBox {
        id: packetLengthCombo
        model: ["1024", "512", "256", "128", "64", "32", "16"]
        width: 160
        height: 32
        anchors.left: firmwareChooseButton.left
        anchors.leftMargin: 0
        anchors.bottom: firmwareChooseButton.top
        anchors.bottomMargin: 30
    }
    CheckBox {
        id: crc8CheckCheckBox
        width: 160
        height: 32
        text: qsTr("CRC-8分片校验")
        checked: true
        anchors.bottom: sequentialNumberCheckBox.top
        anchors.bottomMargin: 30
        anchors.right: sequentialNumberCheckBox.right
        anchors.rightMargin: 0
        font.family: "微软雅黑"
    }
    CheckBox {
        id: sequentialNumberCheckBox
        width: 160
        height: 32
        text: qsTr("未来拓展")
        enabled: false
        checked: false
        anchors.left: downloadButton.left
        anchors.leftMargin: 0
        anchors.bottom: downloadButton.top
        anchors.bottomMargin: 30
        font.family: "微软雅黑"
    }
}
