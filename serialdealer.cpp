#include "serialdealer.h"
#include <QtEndian>
#include <iostream>
#include <fstream>

SerialDealer::SerialDealer(QObject *parent) : QObject(parent)
{
}
const uint8_t* SerialDealer::FirmwareBuffer::GetBufferContent()
{
    return (const uint8_t*)pBuffer;
}
void SerialDealer::FirmwareBuffer::SetBufferContent(uint8_t *Data, uint32_t Length)
{
    BufferLength = Length;
    pBuffer = new uint8_t[BufferLength];
    for(uint32_t i = 0; i < BufferLength ;i++)
    {
        pBuffer[i] = Data[i];
    }
    CRC = CRC_CalcBlockCRC8Bit(pBuffer,BufferLength);
}
uint8_t SerialDealer::FirmwareBuffer::CRC_CalcBlockCRC8Bit(uint8_t *data, uint32_t BufferLength)
{
    uint8_t i;
    uint8_t crc = 0;    // Initial value
    while(BufferLength--)
    {
        crc ^= *data++; // crc ^= *data; data++;
        for ( i = 0; i < 8; i++ )
        {
            if ( crc & 0x80 )
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

SerialDealer::~SerialDealer()
{
    if(SerialPort.isOpen())
    {
        SerialPort.flush();
        SerialPort.close();
    }
    BufferSilcesClean();
}
bool SerialDealer::SerialPortSet()
{
    if( SerialPort.open(QIODevice::ReadWrite) == false)
    {
        std::cout<< "Cannot Open " << SerialPort.portName().toStdString() <<"!"<<std::endl;
        return false;
    }
    SerialPort.setParity(QSerialPort::NoParity);
    SerialPort.setDataBits(QSerialPort::Data8);
    SerialPort.setBaudRate(QSerialPort::Baud115200);
    SerialPort.setStopBits(QSerialPort::OneStop);
    SerialPort.setFlowControl(QSerialPort::NoFlowControl);
    SerialPort.setReadBufferSize(1024);
    return true;
}
bool SerialDealer::ReadFirmwareFile()
{
    std::ifstream inFile;
    inFile.open(FirmwareDir.toStdString(),std::ios::in|std::ios::binary);
    if(inFile)
    {
        BufferSilcesClean();

        inFile.seekg(0,std::ios::end);
        uint64_t FileLength = inFile.tellg();
        inFile.seekg(0,std::ios::beg);

        FirmwareBuffer* pBuffer = 0;
        uint32_t TotalBufferNum = FileLength / SilceSize;
        if(FileLength % SilceSize != 0)
        {
            TotalBufferNum++;
        }

        uint8_t* TempBuffer = new uint8_t[SilceSize];
        for(uint32_t BufferNum = 0;BufferNum < TotalBufferNum ;BufferNum ++ )
        {
            uint32_t UpperLimits = SilceSize;
            if(FileLength - BufferNum * SilceSize < SilceSize)
            {
                UpperLimits = FileLength - BufferNum * SilceSize;
            }
            inFile.read((char*)TempBuffer,UpperLimits);
            pBuffer = new FirmwareBuffer;
            pBuffer->SetBufferContent(TempBuffer,UpperLimits);
            FirmwareBufferSilces.append(pBuffer);
        }
        delete[] TempBuffer;
        return true;
    }
    return false;
}
void SerialDealer::BufferSilcesClean()
{
    foreach (FirmwareBuffer* pBuffer, FirmwareBufferSilces) {
        delete pBuffer;
    }
    FirmwareBufferSilces.clear();
}
inline uint8_t SerialDealer::SerialPortReadByte()
{
    SerialPort.clear();
    if(SerialPort.waitForReadyRead(5000))
    {
        QByteArray bufferRead = SerialPort.readAll();
        if(bufferRead.count() == 1)
        {
            return bufferRead[0];
        }
        std::cout<<"Unexpect Respond."<<std::endl;
    }
    else
    {
        std::cout<<"Timeout."<<std::endl;
    }
    SerialPort.close();
    return 0;
}
inline bool SerialDealer::SerialPortWrite(const uint8_t *Buffer, uint32_t Length)
{
    SerialPort.write((const char*)Buffer,Length);
    SerialPort.flush();
    return SerialPort.waitForBytesWritten(1000);
}
bool SerialDealer::firmwareDownload()
{
    if(ReadFirmwareFile())
    {
        if(!SerialPortSet())return false;
        this->SeqNum = 0;
        for(int32_t i = 0;i < FirmwareBufferSilces.count();i++)
        {
            uint8_t BufferHeader[5] = {0}, CommandIndex = 0;
            FirmwareBuffer* pBuffer = FirmwareBufferSilces[i];
            BufferHeader[CommandIndex++] = DOWNLOAD_FIRMWARE;
            if(UseSeqNum)
            {
                qToLittleEndian((uint16_t)SeqNum,BufferHeader + CommandIndex);
                CommandIndex +=2;
            }
            BufferHeader[CommandIndex++] = pBuffer->GetBuferLength();
            if(pBuffer->GetBuferLength() > SilceSize){std::cout<<"BufferLength is Bigger than SilceSize"<<std::endl;return false;}
            if(UseCRC8){BufferHeader[CommandIndex++] = pBuffer->GetCRC();}

            if(SerialPortWrite(BufferHeader,5))
            {
                uint8_t USART_Ret = SerialPortReadByte();
                switch (USART_Ret) {
                case 0x3C:
                {
                    if(SerialPortWrite(pBuffer->GetBufferContent(),pBuffer->GetBuferLength()))
                    {
                        USART_Ret = SerialPortReadByte();
                        switch (USART_Ret)
                        {
                        case 0x3C://ACK
                            break;
                        default:
                        {
                            SerialPort.close();
                            return false;
                            break;
                        }
                        }
                        continue;
                    }
                    break;
                }
                default:
                    break;
                }

            }
            SerialPort.close();
            return false;
        }
        uint8_t EndOfFirmware = 0xFF;
        SerialPortWrite(&EndOfFirmware,1);
        SerialPort.close();
        return true;
    }
    std::cout<<"Read Firmware Failed!"<<std::endl;
    return false;
}
QVariantList SerialDealer::portListRead()
{
    QVariantList PortList;
    foreach (const QSerialPortInfo &PortInfo, QSerialPortInfo::availablePorts()) {
        if(PortInfo.isBusy() == false)
        {
            PortList << PortInfo.portName();
        }
    }
    return PortList;
}

QString SerialDealer::readFirmwareDir()
{
    return this->FirmwareDir;
}
void SerialDealer::setFirmwareDir(QString &Dir)
{
    this->FirmwareDir = Dir;
}
QString SerialDealer::readPortName()
{
    return SerialPort.portName();
}
void SerialDealer::setPortName(QString& PortName)
{
    SerialPort.setPortName(PortName);
}
unsigned int SerialDealer::readSliceSize()
{
    return this->SilceSize;
}
void SerialDealer::setSliceSize(unsigned int sliceSize)//Need to be improved
{
    if(sliceSize > 255)
        this->SilceSize = 255;
    else
        this->SilceSize = sliceSize;
}
bool SerialDealer::readUseSeqNum()
{
    return this->UseSeqNum;
}
void SerialDealer::setUseSeqNum(bool useSeqNum)
{
    this->UseSeqNum = useSeqNum;
}
bool SerialDealer::readUseCRC8()
{
    return this->UseCRC8;
}
void SerialDealer::setUseCRC8(bool useCRC8)
{
    this->UseCRC8 = useCRC8;
}
