#ifndef QPLANTRANSLATOR_H
#define QPLANTRANSLATOR_H

#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QByteArray>
#include "mission.h"

class qPlanTranslator
{
public:
    qPlanTranslator();
    void translate(const QString& missionPath);
    void translate(Mission* mission);
    void setPlanDirectory(const QString& writeDirectory);
    QString planDirectory();

private:
    QString planDirPath;
    QByteArray processMission(QByteArray missionData);
};

#endif // QPLANTRANSLATOR_H
