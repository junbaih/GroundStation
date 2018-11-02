#ifndef QPLANTRANSLATOR_H
#define QPLANTRANSLATOR_H

#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QByteArray>


class qPlanTranslator
{
public:
    qPlanTranslator();
    void translate(const QString& missionPath);
    void setPlanDirectory(const QString& writeDirectory);
    QString planDirectory();

private:
    QString planDirPath;
    QByteArray processMission(QByteArray missionData);
};

#endif // QPLANTRANSLATOR_H
