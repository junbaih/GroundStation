#include <QCoreApplication>
#include <iostream>
#include "interop.h"
#include <string>
#include <QTimer>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::string username = "testuser";
    std::string password = "testpass";
    Interop *interop = nullptr;
    try {
        interop = new Interop(username,password);

    }
    catch (QNetworkReply::NetworkError err) {
        qDebug() << "Interop server connection Failure";
    }
    interop->getMissions();

}

void reConnectInterop()
{
    return;
}
