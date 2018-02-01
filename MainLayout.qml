import QtQuick 2.8
import QtQuick.Dialogs 1.2
import io.qt.serialportdealer 1.0
MainLayoutForm {
    property string firmwareDirectory
    width: parent.width
    height: parent.height
    Timer{
        interval: 500
        running: true
        repeat: true
        onTriggered: {
            serialPortCombo.model = portDealer.portList
        }
    }
    MessageDialog{
        id:msgBox
        visible: false
        function showCriticalMessage(title,text)
        {
            msgBox.setTitle(title)
            msgBox.setText(text)
            msgBox.icon = StandardIcon.Critical
            msgBox.standardButtons = MessageDialog.Ok
            msgBox.visible = true
        }
        function showOKMessage(title,text)
        {
            msgBox.setTitle(title)
            msgBox.setText(text)
            msgBox.icon = StandardIcon.Information
            msgBox.standardButtons = MessageDialog.Ok
            msgBox.visible = true
        }
    }
    firmwareChooseButton.onClicked: {
        fileDialog.open()
    }
    downloadButton.onClicked: {
        if(serialPortCombo.count === 0)
        {
            msgBox.showCriticalMessage("错误","请插入串口调试器！")
        }
        else
        {
            portDealer.useSeqNum = sequentialNumberCheckBox.checked
            portDealer.useCRC8 = crc8CheckCheckBox.checked
            portDealer.firmwareDir = firmwareDirectory
            portDealer.portName = serialPortCombo.currentText;
            portDealer.sliceSize = parseInt(packetLengthCombo.currentText)
            if(portDealer.firmwareDownload() !== true)
            {
                msgBox.showCriticalMessage("下载失败","请确认没有其他程序占用该端口，同时确认！")
            }
            else
            {
                msgBox.showOKMessage("下载成功","下载完成！")
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "选择准备烧录的固件"
        folder: shortcuts.desktop
        nameFilters: ["Bin Files (*.bin)"]
        selectExisting: true
        selectFolder: false
        selectMultiple: false
        onAccepted: {
            firmwareDirectory = fileUrl.toString().slice(8)
            fileInfo.text = firmwareDirectory
            downloadButton.enabled = true
        }
    }
    SerialPortDealer{
        id:portDealer
    }
}
