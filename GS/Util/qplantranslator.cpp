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
        QMessageBox(QMessageBox::Warning, "mission file not opened", "check the path");
        return;
    }
    QFile planFile(planDirPath+"AutoLoad1.plan");
    if (!planFile.open(QFile::ReadWrite|QFile::Truncate))
       {
        qDebug()<<"plan file not opened";
        QMessageBox(QMessageBox::Warning, "plan file not opened", "check the path");
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
    QJsonDocument jdoc = QJsonDocument::fromJson(missionData);
    qDebug()<<jdoc.isNull()<<jdoc.isArray()<<jdoc.isEmpty();
    QJsonObject temp = jdoc.array().at(0).toObject();
    QJsonArray temp2 = QJsonDocument::fromJson(missionData).array();
    QJsonArray temp3={1,2,"plan"};

    qDebug()<<"jarray3"<<QJsonDocument(temp3).toJson();
    qDebug()<<"jarray3 keys"<<QJsonDocument(temp3).object().keys();
    qDebug()<<"jarray"<<temp2.isEmpty()<<temp2.last();
    qDebug()<<"jobject"<<temp.isEmpty()<<temp.keys();

    QJsonObject planObj
    {
        {"fileType","Plan"},
        {"groundStation","QGroundControl"},
        {"version",1},
        {"rallyPoints",QJsonObject
            {
                {"version",2},{"points",QJsonArray()}
            }
        },
        {"geoFence",QJsonObject
            {
                {"circles",QJsonArray()},
                {"polygons",QJsonArray()},
                {"version",1}
            }
        }
    };
    QJsonObject missionItem
    {
        {"items",QJsonArray()},
        {"plannedHomePosition",QJsonArray{38.14792,-76.427995,200}},
        {"curiseSpeed",15},
        {"firmwareType",12},  // 12 PX4, 3 Ardupilot
        {"hoverSpeed",5},
        {"vehicleType",1},  //  1 fixed wing, 2 quadcopter
        {"version",1}
    };
    planObj["mission"]=missionItem;
    return QJsonDocument(planObj).toJson();
}
