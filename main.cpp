#include <QCoreApplication>
#include <iostream>
#include "interop.h"
#include <string>
#include <QTimer>
#include <QVector>
#include <QDebug>
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
    QVector<float> lat{33.77146530151367,-117.69239807128906,45,-138.6};
    QVector<float> lon{75.6,25.6,-75.6,64.6};
    QVector<float> alt{50.3,51.3,42.3,12.5};
    QVector<float> head{180.6,80.6,50.6,23.4};
    QVector<int> result{200,200,200,400};
    while (true)
    {
        QNetworkReply* reply = interop->sendTelemetry(lat[0],lon[0],alt[0],head[0]);
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QTextStream(stdout) << (status==result[0]) << "with status:" <<status <<"expectation:"<<result[0]<<endl;
    }

}

void reConnectInterop()
{
    return;
}
