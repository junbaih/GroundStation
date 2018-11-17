#include "qplantranslator.h"
#include <QMessageBox>
#include <QDir>

qPlanTranslator::qPlanTranslator()
{

}

void qPlanTranslator::translate(const QString& missionPath)
{
    QFile missionFile(missionPath);
    if (! missionFile.open(QIODevice::ReadOnly))
    {
        //QMessageBox(QMessageBox::Warning, "mission file not opened", "check the path");
        qDebug()<<"mission file not opened";
        return;
    }
    QFile planFile(QDir::rootPath()+"home/junbaih/Documents/QGroundControl/Missions/AutoLoad1.plan");
    qDebug()<<QDir::rootPath()+"home/junbaih/Documents/QGroundControl/Missions/AutoLoad1.plan";
    if (!planFile.open(QFile::ReadWrite|QFile::Truncate))
       {
        qDebug()<<"plan file not opened";
        //QMessageBox(QMessageBox::Warning, "plan file not opened", "check the path");
        return;
    }

    QTextStream stream(&planFile);

    stream<<processMission(missionFile.readAll());
    planFile.close();
}

void qPlanTranslator::translate(Mission *mission)
{
    return ;
}

void qPlanTranslator::setPlanDirectory(const QString& writeDirectory)
{
    planDirPath = writeDirectory;
}

QString qPlanTranslator::planDirectory()
{
    return planDirPath;
}

QByteArray qPlanTranslator::processMission(QByteArray missionData)
{
    QJsonDocument missionDoc = QJsonDocument::fromJson(missionData);
    //qDebug()<<missionDoc.isNull()<<missionDoc.isArray()<<missionDoc.isEmpty();
    QJsonObject missionJObj = missionDoc.array().at(0).toObject();
    QJsonArray temp2 = QJsonDocument::fromJson(missionData).array();
    QJsonArray temp3={1,2,"plan"};

    qDebug()<<"jarray3"<<QJsonDocument(temp3).toJson();
    qDebug()<<"jarray3 keys"<<QJsonDocument(temp3).object().keys();
    qDebug()<<"jarray"<<temp2.isEmpty()<<temp2.last();
   // qDebug()<<"jobject"<<temp.isEmpty()<<temp.keys();

    Mission* pMission = new Mission(missionJObj,QJsonObject());

    QJsonArray flyZoneWayptsJArray;
    for(QVector2D boundary:pMission->fly_zones[0].boundary_points)
    {
        flyZoneWayptsJArray.append(QJsonArray{boundary[0],boundary[1]});
    }
    QJsonArray wayPointsJArray;
    int i = 1;

    for (Waypt wpt:pMission->generatedPath.waypoints)

    {
        wayPointsJArray.append(
            QJsonObject{
                        {"AMSLAltAboveTerrain",QJsonValue()},
                        {"Altitude",wpt.coords.z()},
                        {"AltitudeMode",0},
                        {"type","SimpleItem"},
                        {"autoContinue",true},
                        {"frame",0},
                        {"doJumpId",i},

                        {"command",wpt.action},

                        {"params",QJsonArray{wpt.param1,wpt.param2,wpt.param3,wpt.param4,wpt.coords.x(),wpt.coords.y(),wpt.coords.z()}}
                    }
                               );
        i++;
    }
    QJsonObject planObj
    {
        {"fileType","Plan"},
        {"groundStation","QGroundControl"},
        {"version",1},
        {"rallyPoints",QJsonObject
            {

                {"version",2},{"points",QJsonArray{QJsonArray{38.084120,-76.254590, 20}}}

            }
        },
        {"geoFence",QJsonObject
            {
                {"circles",QJsonArray()},

                {"polygons",QJsonArray
                    {
                    QJsonObject
                    {
                        {"inclusion",true},
                        {"polygon",flyZoneWayptsJArray},
                        {"version",1}
                    }
                    }
                },
                {"version",2}

            }
        }
    };

    QJsonObject missionItem
    {
        {"items",wayPointsJArray},

        {"plannedHomePosition",QJsonArray{pMission->home_pos.x(),pMission->home_pos.y(),200}},

        {"curiseSpeed",15},
        {"firmwareType",12},  // 12 PX4, 3 Ardupilot
        {"hoverSpeed",5},
        {"vehicleType",1},  //  1 fixed wing, 2 quadcopter
        {"version",1}
    };
    planObj["mission"]=missionItem;
    return QJsonDocument(planObj).toJson();
}
