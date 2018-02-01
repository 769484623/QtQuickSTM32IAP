#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "serialdealer.h"

void BondingObjectToQML(QQmlApplicationEngine* Engine)
{
    SerialDealer* portDealer = new SerialDealer();
    Engine->rootContext()->setContextProperty("portDealer", portDealer);
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    qmlRegisterType<SerialDealer>("io.qt.serialportdealer", 1, 0, "SerialPortDealer");
    QQmlApplicationEngine engine;

    BondingObjectToQML(&engine);

    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
