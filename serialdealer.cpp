#include "serialdealer.h"
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
    firmwareBufferSilcesClean();
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
bool SerialDealer::SendSliceBuffer(int Index)
{
    uint8_t Header[3] = {0};
    uint8_t HeaderLength = 0;
    if(Index < FirmwareBufferSilces.count())
    {
        FirmwareBuffer* pBuffer = FirmwareBufferSilces[Index];
        if(pBuffer->GetBuferLength() > SilceSize)
        {
            std::cout<<"BufferLength is Bigger than SilceSize"<<std::endl;
            return false;
        }
        Header[HeaderLength++] = pBuffer->GetBuferLength();

        if(this->UseSeqNum)//Use sequence number
        {
            if(this->SeqNum > 255)
            {
                std::cout<<"SeqNum is Bigger than 255"<<std::endl;
                return false;
            }
            Header[HeaderLength++] = this->SeqNum;
            this->SeqNum ++;
        }
        if(this->UseCRC8)// Use CRC8
        {
            Header[HeaderLength++] = pBuffer->GetCRC();
        }
        SerialPort.write((const char*)Header,HeaderLength);
        SerialPort.write((const char*)pBuffer->GetBufferContent(),pBuffer->GetBuferLength());
        SerialPort.flush();
        return SerialPort.waitForBytesWritten(1000);
    }
    return false;
}
bool SerialDealer::ReadFirmwareFile()
{
    std::ifstream inFile;
    inFile.open(FirmwareDir.toStdString(),std::ios::in|std::ios::binary);
    if(inFile)
    {
        firmwareBufferSilcesClean();

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
void SerialDealer::firmwareBufferSilcesClean()
{
    foreach (FirmwareBuffer* pBuffer, FirmwareBufferSilces) {
        delete pBuffer;
    }
    FirmwareBufferSilces.clear();
}
bool SerialDealer::firmwareDownload()
{
    if(ReadFirmwareFile())
    {
        if(!SerialPortSet())return false;

        if(UseSeqNum)
            this->SeqNum = 0;

        for(int32_t i = 0;i < FirmwareBufferSilces.count();i++)
        {
            if(SendSliceBuffer(i))
            {
                SerialPort.clear();
                if(SerialPort.waitForReadyRead(5000))
                {
                    QByteArray bufferRead = SerialPort.readAll();
                    if(bufferRead.count() != 1)
                    {
                        std::cout<<"Unexpect Respond."<<std::endl;
                        SerialPort.close();
                        return false;
                    }
                    switch (bufferRead[0])
                    {
                    case 0x3C://ACK
                        break;
                    case 0x0F://CRC Error
                    default:
                    {
                        uint8_t Timeout = 0;
                        for(; Timeout < 5 && (!SendSliceBuffer(i));Timeout++);
                        if(Timeout == 5)
                        {
                            std::cout<<"Please Check the cable."<<std::endl;
                            SerialPort.close();
                            return false;
                        }
                        break;
                    }
                    }
                }
                else
                {
                    SerialPort.close();
                    return false;
                }
            }
            else
            {
                SerialPort.close();
                return false;
            }
        }
        uint8_t EndOfFirmware = 0x00;
        SerialPort.write((char*)&EndOfFirmware,1);
        SerialPort.flush();
        SerialPort.close();
    }
    else
    {
        std::cout<<"Read Firmware Failed!"<<std::endl;
        return false;
    }
    return true;
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
