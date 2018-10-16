#include "maindockwindow.h"
#include "ui_maindockwindow.h"
#include <QDebug>
#include <QQuickItem>
#include <QtCore>
#include <QMargins>
#include <QRegExp>
#include <QRegularExpression>
MainDockWindow::MainDockWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainDockWindow),
    imageReviewWidget(new ImgReview()),
    interopConnected(false)
{
    ui->setupUi(this);
    ui->graphDock->hide();
    ui->connDock->hide();
    ui->timerDock->hide();

    // UDP Link for SITL
  //  link = new UdpLink();

//     Serial link for RF900
    link = new SerialLink();
    link->startLink("tty1:USB0");

    connect(ui->connWidget, &ConnectionWidget::connectTo, this, &MainDockWindow::changeLink);
    mission = new Mission();
    uavButton = new UAVButton(this);
    connect(uavButton, &UAVButton::openConn, this, &MainDockWindow::showConnWidget);
    connect(uavButton, &UAVButton::changeStatus, ui->connWidget, &ConnectionWidget::setStatus);
    decoder = new Decoder();
    decoder->setLink(link);
    connectDecoder(decoder);

    encoder = new Encoder();
    encoder->setLink(link);
    connectEncoder(encoder);

    Waypoint * waypoint = new Waypoint();
    connectWaypoint(waypoint, encoder, decoder);

    addToolBarButtons();
    //install event filter

//    imageReviewWidget->ImgNextButton()->installEventFilter(this);
    imageReviewWidget->ImgSendButton()->installEventFilter(this);
    imageReviewWidget->PropertySendButton()->installEventFilter(this);
 //   imageReviewWidget->PropertyNextButton()->installEventFilter(this);

    // my widget

    QAction*imgreview_clicked =  ui->toolBar->addAction("Image Review");
    connect(imgreview_clicked,SIGNAL(triggered()),this,SLOT(addImgReview()));
    // end

    QWidget* spacer = new QWidget();
    LoginWidget * loginWidget = new LoginWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolBar->addWidget(spacer);
    ui->toolBar->addWidget(loginWidget);
    ui->toolBar->addWidget(uavButton);

    connect(loginWidget, &LoginWidget::loginSuccess, ui->missionWidget, &MissionWidget::getMissions);
    connect(loginWidget,&LoginWidget::loginSuccess, this,&MainDockWindow::interopConnect);
    connect(decoder, &Decoder::gpsReceived, loginWidget, &LoginWidget::updateGPS);
    connect(decoder, &Decoder::gps_intReceived, loginWidget, &LoginWidget::updatePOS);
    connect(ui->missionWidget, &MissionWidget::drawMission, ui->mapWidget, &MapWidget::drawMission);
    connect(ui->missionWidget, &MissionWidget::drawObstacle, ui->mapWidget, &MapWidget::drawPolygonF);
    connect(ui->missionWidget, &MissionWidget::clearMap, ui->mapWidget, &MapWidget::clearMap);

    connect(ui->missionWidget, &MissionWidget::drawWaypoints, ui->mapWidget, &MapWidget::drawWaypoints);
    connect(ui->missionWidget, &MissionWidget::selectWaypoint, ui->mapWidget, &MapWidget::selectWaypoint);
    connect(ui->missionWidget, &MissionWidget::moveWaypointSignal, ui->mapWidget, &MapWidget::moveWaypoint);
    connect(ui->missionWidget, &MissionWidget::removeWaypointSignal, ui->mapWidget, &MapWidget::removeWaypoint);
    connect(ui->missionWidget, &MissionWidget::editMode, ui->mapWidget, &MapWidget::changeEditMode);
    connect(decoder, &Decoder::gpsReceived, ui->missionWidget, &MissionWidget::updateGPS);
    connect(decoder, &Decoder::vfrHudReceived, ui->missionWidget, &MissionWidget::updateVFR);
    connect(decoder, &Decoder::missionCurrentReceived, ui->missionWidget, &MissionWidget::updateCurrentMission);
    ui->missionWidget->updateDraw();
}

void MainDockWindow::addToolBarButtons() {
    foreach(QDockWidget * dw, this->findChildren<QDockWidget*>()) {
       if (!dw->findChild<ConnectionWidget*>())
            ui->toolBar->addAction(dw->toggleViewAction());
    }
}

void MainDockWindow::showConnWidget() {
    ui->connDock->show();
}
void MainDockWindow::changeLink(Link * link, QString hostport) {
    this->link->closeLink();
    this->link = link;
    link->startLink(hostport);
    decoder->setLink(link);
    encoder->setLink(link);
}

void MainDockWindow::addInteropLogin() {
    QWidget* spacer = new QWidget();
    LoginWidget * loginWidget = new LoginWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolBar->addWidget(spacer);
    ui->toolBar->addWidget(loginWidget);
}

void MainDockWindow::connectDecoder(Decoder * decoder) {
    connect(decoder, &Decoder::vfrHudReceived, ui->qfiWidget, &QFIWidget::updateVFR);
    connect(decoder, &Decoder::attReceived, ui->qfiWidget, &QFIWidget::updateAttitude);
    connect(decoder, &Decoder::pressureReceived, ui->qfiWidget, &QFIWidget::updatePressure);
    connect(decoder, &Decoder::vfrHudReceived, ui->movementWidget, &MovementWidget::updateTelemetry);
    connect(decoder, &Decoder::attReceived, ui->movementWidget, &MovementWidget::updateAttitude);
    connect(decoder, &Decoder::gpsReceived, ui->graphWidget, &GraphWidget::appendTelemData);
    connect(decoder, &Decoder::heartbeatReceived, ui->actionWidget, &ActionWidget::toggleModeButtons);
    connect(decoder, &Decoder::statTextReceived, ui->messageWidget, &MessageWidget::updateMessages);

    connect(decoder, &Decoder::heartbeatReceived, uavButton, &UAVButton::updateHeartbeat);
    connect(decoder, &Decoder::statusReceived, uavButton, &UAVButton::updateBattery);

    connect(decoder, &Decoder::gpsReceived, ui->mapWidget, &MapWidget::updateUAVPosition);
    connect(decoder, &Decoder::vfrHudReceived, ui->mapWidget, &MapWidget::updateUAVHeading);

    connect(decoder, &Decoder::armReceived, ui->mapWidget, &MapWidget::updateArmState);
    connect(decoder, &Decoder::armReceived, ui->actionWidget, &ActionWidget::toggleArmButton);
    connect(decoder, &Decoder::gps_intReceived, ui->movementWidget, &MovementWidget::updateGlobalPosition);
}



void MainDockWindow::connectEncoder(Encoder * encoder) {
    connect(ui->actionWidget, &ActionWidget::setArm, encoder, &Encoder::sendArm);
    connect(ui->actionWidget, &ActionWidget::setMode, encoder, &Encoder::sendMode);
    connect(ui->missionWidget, &MissionWidget::dropSignal, encoder, &Encoder::sendSetServo);
}

void MainDockWindow::connectWaypoint(Waypoint * waypoint, Encoder * encoder, Decoder * decoder) {
    connect(waypoint, &Waypoint::reqClearAll, encoder, &Encoder::sendClearAll);
    connect(waypoint, &Waypoint::reqList, encoder, &Encoder::sendMissionRequestList);
    connect(waypoint, &Waypoint::reqWP, encoder, &Encoder::sendMissionRequest);
    connect(waypoint, &Waypoint::sendAck, encoder, &Encoder::sendMissionACK);
    connect(waypoint, &Waypoint::sendWP, encoder, &Encoder::sendMissionItem);
    connect(waypoint, &Waypoint::sendWPCount, encoder, &Encoder::sendMissionCount);
    connect(waypoint, &Waypoint::sendWPSetCurrent, encoder, &Encoder::sendMissionSetCurrent);

    connect(decoder, &Decoder::mAckReceived, waypoint, &Waypoint::updateMissionAck);
    connect(decoder, &Decoder::missionCountReceived, waypoint, &Waypoint::updateMissionCount);
    connect(decoder, &Decoder::missionItemReceived, waypoint, &Waypoint::updateMissionItem);
    connect(decoder, &Decoder::missionCurrentReceived, waypoint, &Waypoint::updateCurrent);
    connect(decoder, &Decoder::mrequestReceived, waypoint, &Waypoint::updateMissionRequest);

    connect(ui->missionWidget, &MissionWidget::clearMissions, waypoint, &Waypoint::clearAllWaypoints);
    connect(ui->missionWidget, &MissionWidget::readMissionsSignal, waypoint, &Waypoint::readWaypointsList);
    connect(ui->missionWidget, &MissionWidget::writeMissionsSignal, waypoint, &Waypoint::writeWaypoints);
    connect(ui->missionWidget, &MissionWidget::setCurrentMision, waypoint, &Waypoint::setCurrentWaypoint);
    connect(waypoint, &Waypoint::waypointsReceived, ui->missionWidget, &MissionWidget::readMissions);
    connect(waypoint, &Waypoint::waypointsWriteStatus, ui->missionWidget, &MissionWidget::writeMissionsStatus);
    connect(waypoint, &Waypoint::waypointsClearStatus, ui->messageWidget, &MessageWidget::updateClearMission);
}

bool MainDockWindow::eventFilter(QObject *watched, QEvent *event)
{

    if(watched== imageReviewWidget->PropertySendButton())
    {

        if(event->type()==QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseevent = static_cast<QMouseEvent*>(event);
            if(mouseevent->button()==Qt::LeftButton)
            {
                if(!interopConnected)
                    QMessageBox(QMessageBox::Warning, "interop not connected", "connect interop first and try again");
                else

                {
                QFile file;
                file.setFileName(imageReviewWidget->PropertyContainer->propertyfileName());
                file.open(QIODevice::ReadOnly | QIODevice::Text);
                //qDebug()<<"opened!";
                //QString json = file.readAll();
                QString json = imageReviewWidget->PropertyContainer->newDoc->toPlainText();
                json.remove(QRegExp("[\\n\\t\\r ]"));
                file.close();

                qDebug()<<"ya "<<json;
                //QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8());
                //QString tempstr = QString::fromStdString(jdoc.toBinaryData().toStdString());
                QJsonObject obj{{"alphanumeric","A"},{"color","orange"},{"type","standard"}};
                //if(loginWidget.)
                QByteArray jsonbyte= json.toUtf8();
                qDebug()<<"json byte:" << jsonbyte;
                QJsonDocument response = interop->sendODLC(QJsonDocument::fromJson(jsonbyte));
                qDebug()<<"response"<<response.toJson();
                QJsonDocument tempjdoc = QJsonDocument(obj);
                QJsonDocument retrieved= interop->getUploadedODLCs();
                qDebug()<<"retrieve"<< retrieved.toJson();
                //interop->sendODLC(QJsonDocument(obj));
                }
            }
        }
    }
    else if(watched== imageReviewWidget->ImgSendButton())
    {
        if(event->type()==QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseevent = static_cast<QMouseEvent*>(event);
            if(mouseevent->button()==Qt::LeftButton)
            {
                 qDebug()<<"image send!";
                 interop->updateODLCThumbnail(80,QImage("img1.jpeg"));
            }
        }
    }
    return QMainWindow::eventFilter(watched,event);
}

void MainDockWindow::addImgReview()
{
    imageReviewWidget->show();
    //qDebug()<<"triggered!";
}

void MainDockWindow::interopConnect(Interop *i)
{
    interop = i;
    interopConnected=true;
}

void MainDockWindow::test() {
    emit (what(2));
}

QDockWidget * MainDockWindow::createDockWidget(const QString &title, Qt::DockWidgetArea area, QWidget * child, QWidget * parent) {
    QDockWidget * dock = new QDockWidget(title, parent);
    dock->setWidget(child);
    addDockWidget(area, dock);
    dock->setVisible(false);
    ui->toolBar->addAction(dock->toggleViewAction());
    return dock;
}

QQuickWidget * MainDockWindow::createQmlWidget(QUrl qmlSource, QWidget * parent) {
    QQuickWidget * quickWidget = new QQuickWidget(parent);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(qmlSource);
    return quickWidget;
}

void MainDockWindow::testFind() {
    qDebug() << this->findChild<GraphWidget*>();
}

void MainDockWindow::hideDockWidgets() {
    foreach(QDockWidget * dw, this->findChildren<QDockWidget*>()) {
        dw->setVisible(false);
    }
}
void MainDockWindow::closeDockWidgets() {
    foreach(QDockWidget * dw, this->findChildren<QDockWidget*>()) {
        delete dw;
    }
}


//void MainDockWindow::updateMovingObjects(QQuickWidget * mapWidget){
//    Obstacles obstacles = interop->getObstacles();
//    obstacles.updateMovingObjects(mapWidget);
//}

MainDockWindow::~MainDockWindow()
{
    delete ui;
}
