#ifndef SERIALDEALER_H
#define SERIALDEALER_H

#include <QObject>
#include <QVector>
#include <QVariantList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class SerialDealer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList portList READ portListRead)
    Q_PROPERTY(QString portName READ readPortName WRITE setPortName)
    Q_PROPERTY(QString firmwareDir READ readFirmwareDir WRITE setFirmwareDir)
    Q_PROPERTY(unsigned int sliceSize READ readSliceSize WRITE setSliceSize)
    Q_PROPERTY(bool useCRC8 READ readUseCRC8 WRITE setUseCRC8)

private:
    class FirmwareBuffer
    {
    public:
        ~FirmwareBuffer(){if(BufferLength != 0)delete[] pBuffer;}
        void SetBufferContent(uint8_t* Data,uint32_t Length);
        const uint8_t *GetBufferContent();
        inline uint32_t GetBuferLength(){return BufferLength;}
        inline uint8_t GetCRC(){return CRC;}
    private:
        uint32_t BufferLength = 0;
        uint8_t* pBuffer = 0;
        uint8_t CRC = 0;

        uint8_t CRC_CalcBlockCRC8Bit(uint8_t *data, uint32_t BufferLength);
    };
    enum CommandType{DOWNLOAD_FIRMWARE = 0};

    bool UseCRC8 = true;
    uint32_t SilceSize = 0;
    QString FirmwareDir = "";

    QVector<FirmwareBuffer*> FirmwareBufferSilces;
    QSerialPort SerialPort;
    bool SerialPortSet();
    bool ReadFirmwareFile();//Read, then slice it to SilceSize pieces.

    void BufferSilcesClean();

    inline bool SerialPortWrite(const uint8_t *Buffer, uint32_t Length);
    inline uint8_t SerialPortReadByte();
public:
    Q_INVOKABLE bool firmwareDownload();
    Q_INVOKABLE QVariantList portListRead();

    explicit SerialDealer(QObject *parent = nullptr);
    ~SerialDealer();

    QString readFirmwareDir();
    void setFirmwareDir(QString& Dir);

    QString readPortName();
    void setPortName(QString& PortName);

    unsigned int readSliceSize();
    void setSliceSize(unsigned int sliceSize);

    bool readUseCRC8();
    void setUseCRC8(bool useCRC8);

signals:
    void FirmwareWritingProgress(float Progress);
public slots:
};

#endif // SERIALDEALER_H
