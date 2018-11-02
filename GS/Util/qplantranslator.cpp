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
    if (!planFile.open(QIODevice::ReadWrite))
       {
        qDebug()<<"plan file not opened";
        QMessageBox(QMessageBox::Warning, "plan file not opened", "check the path");
        return;
    }
    QTextStream stream(&planFile);
    stream<<processMission(missionFile.readAll());
    planFile.close();
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
    return missionData.toUpper();
}
