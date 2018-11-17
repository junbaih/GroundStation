#include "interop.h"

QString ENDPOINT = "http://localhost:8000";

Interop::Interop(const std::string& username, const std::string& password)
{
    networkAccess = new QNetworkAccessManager();
    connect(networkAccess, &QNetworkAccessManager::finished, this, &Interop::replyFinished);

    std::vector<HeaderSet> headers;
    headers.push_back(HeaderSet{QNetworkRequest::ContentTypeHeader, "application/json"});

    QByteArray postData;
    postData.append("username=" + QString::fromStdString(username) + "&");
    postData.append("password=" + QString::fromStdString(password));

    QJsonObject RequestObj;
    RequestObj["username"] = QString::fromStdString(username);
    RequestObj["password"] = QString::fromStdString(password);
    QJsonDocument RequestData(RequestObj);

    QNetworkReply *reply = postRequest(ENDPOINT + "/api/login", RequestData.toJson(), headers);
    qDebug()<<"auto saved cookie"<<networkAccess->cookieJar()->cookiesForUrl(ENDPOINT);
    // want to raise an error here if failure
    waitForResponse(reply);
    if (reply->error()) {
        throw reply->error();
    }
}

void Interop::replyFinished(QNetworkReply* reply)
{
    qDebug()<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()<<reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reply->error()) {
        qCritical() << "Something bad happened to one of our requests: " << reply->errorString();
        qCritical() << reply->error();
    }
    else {
//        qDebug() << "ContentType: " << reply->header(QNetworkRequest::ContentTypeHeader).toString();
//        qDebug() << "Last Modified: " << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString();;
//        qDebug() << "Content Length: " << reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
//        qDebug() << "HTTP Code: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//        qDebug() << "HTTP Reason: " << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
//        qDebug() << "Response: " << reply->peek(reply->bytesAvailable());
    }
    reply->deleteLater();
}

void Interop::waitForResponse(QNetworkReply* reply) {
    while(!reply->isFinished()) {
        qApp->processEvents();
    } // spin
}

QNetworkReply* Interop::sendRequest(const QNetworkAccessManager::Operation& operation, const QString& url, const QByteArray& data, const std::vector<HeaderSet>& headers) {
    QNetworkRequest req{QUrl(url)};
    for(HeaderSet headerSet : headers) {
        req.setHeader(headerSet.header, headerSet.value);
    }
    QNetworkReply* reply;

    switch (operation) {
    case QNetworkAccessManager::GetOperation:
        reply = networkAccess->get(req);
        waitForResponse(reply);
        break;
    case QNetworkAccessManager::PostOperation:
        reply = networkAccess->post(req, data);
        waitForResponse(reply);
        break;
    case QNetworkAccessManager::DeleteOperation:
        reply = networkAccess->deleteResource(req);
        break;
    case QNetworkAccessManager::PutOperation:
        reply = networkAccess->put(req, data);
        waitForResponse(reply);
        break;
    default:
        throw std::invalid_argument("operation not implemented");
    }
//    waitForResponse(reply); // Might be needed for delete and put operations but mission write/read breaks w/ Post
//    qDebug() << reply->request().url();
//    qDebug() << "Response: " << reply->peek(reply->bytesAvailable());
    return reply;
}

QNetworkReply* Interop::getRequest(const QString& url)
{
    return sendRequest(QNetworkAccessManager::GetOperation, url);
}

QNetworkReply* Interop::postRequest(const QString& url, const QByteArray& data, const std::vector<HeaderSet>& headers)
{
    return sendRequest(QNetworkAccessManager::PostOperation, url, data, headers);
}

QNetworkReply* Interop::deleteRequest(const QString& url)
{
    return sendRequest(QNetworkAccessManager::DeleteOperation, url);
}

QNetworkReply* Interop::putRequest(const QString& url, const QByteArray& data, const std::vector<HeaderSet>& headers)
{
    return sendRequest(QNetworkAccessManager::PutOperation, url, data, headers);
}

QJsonDocument Interop::getMissions() {
    QNetworkReply* reply = getRequest(ENDPOINT + "/api/missions");
    QByteArray readData = reply->readAll();
    QJsonDocument mission = QJsonDocument::fromJson(readData);
    qDebug()<<"mission jdoc content:"<<mission.toJson();
    qDebug()<<"readData content after saving jdoc"<<readData;
    QFile newfile(QDir::currentPath() + "/../QInterop_Client/res/FlyMission.json");
    if ( newfile.open(QIODevice::WriteOnly) )
     {
        qDebug()<<"write file open";
         QTextStream stream( &newfile );
         stream << mission.toJson()<< endl;
         newfile.close();
     }
    qPlanTranslator translator;
    //translator.setPlanDirectory(QDir::currentPath() +"/../../GroundStation/GS/res/");
    translator.translate(QDir::currentPath() + "/../QInterop_Client/res/FlyMission.json");

    return mission;
}

QJsonDocument Interop::getMission(int id) {
    QNetworkReply* reply = getRequest(ENDPOINT + "/api/missions/" + QString::number(id));
    return QJsonDocument::fromJson(reply->readAll());
}

QJsonDocument Interop::getObstacles()
{
    QNetworkReply* reply = getRequest(ENDPOINT + "/api/obstacles");
    return QJsonDocument::fromJson(reply->readAll());
}

void Interop::sendTelemetry(float latitude, float longitude, float altitude_msl, float uas_heading) {
    std::vector<HeaderSet> headers;
    headers.push_back(HeaderSet{QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded"});

//    qDebug() << latitude << longitude << altitude_msl << uas_heading;
    QByteArray postData;
    postData.append("latitude=" + QString::number(latitude) + "&");
    postData.append("longitude=" + QString::number(longitude) + "&");
    postData.append("altitude_msl=" + QString::number(altitude_msl) + "&");
    postData.append("uas_heading=" + QString::number(uas_heading));

    QNetworkReply *reply = postRequest(ENDPOINT + "/api/telemetry", postData, headers);
}

QJsonDocument Interop::sendODLC(const QJsonDocument& odlc) {
    std::vector<HeaderSet> headers;
    headers.push_back(HeaderSet{QNetworkRequest::ContentTypeHeader, "application/json"});
    //QByteArray temp = odlc.toJson(QJsonDocument::Compact);
  //  QNetworkReply *reply = postRequest(ENDPOINT + "/api/odlcs", temp, headers);
    QNetworkReply *reply = postRequest(ENDPOINT + "/api/odlcs", odlc.toJson(), headers);
    return QJsonDocument::fromJson(reply->readAll());
}

QJsonDocument Interop::getUploadedODLCs() {
    QNetworkReply* reply = getRequest(ENDPOINT + "/api/odlcs");
    return QJsonDocument::fromJson(reply->readAll());
}

QJsonDocument Interop::getUploadedODLC(int id) {
    QNetworkReply* reply = getRequest(ENDPOINT + "/api/odlcs/" + QString::number(id));
    return QJsonDocument::fromJson(reply->readAll());
}

QJsonDocument Interop::updateODLC(int id, const QJsonDocument& data) {
    std::vector<HeaderSet> headers;
    headers.push_back(HeaderSet{QNetworkRequest::ContentTypeHeader, "application/json"});

    QNetworkReply* reply = putRequest(ENDPOINT + "/api/odlcs/" + QString::number(id), data.toJson(), headers);
    return QJsonDocument::fromJson(reply->readAll());
}

void Interop::deleteODLC(int id) {
    QNetworkReply* reply = deleteRequest(ENDPOINT + "/api/odlcs/" + QString::number(id));
}

QImage Interop::getODLCThumbnail(int id) {
    QNetworkReply* reply = getRequest(ENDPOINT + "/api/odlcs/" + QString::number(id) + "/image");
    return QImage::fromData(reply->readAll());
}

void Interop::updateODLCThumbnail(int id, const QImage& image) {
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG");

    std::vector<HeaderSet> headers;
    headers.push_back(HeaderSet{QNetworkRequest::ContentTypeHeader, "image/jpeg"});

    QNetworkReply *reply = postRequest(ENDPOINT + "/api/odlcs/" + QString::number(id) + "/image", ba, headers);
    // do something on error
}

void Interop::deleteODLCThumbnail(int id) {
    QNetworkReply* reply = deleteRequest(ENDPOINT + "/api/odlcs/" + QString::number(id));
}
