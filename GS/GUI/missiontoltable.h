#ifndef MISSIONTOLTABLE_H
#define MISSIONTOLTABLE_H

#include <QTableView>
#include <QKeyEvent>
#include <QHeaderView>
#include <QStandardItemModel>

class MissionTOLTable : public QTableView
{
    Q_OBJECT
public:
    MissionTOLTable();
    explicit MissionTOLTable(QWidget *parent = nullptr);

    void selectChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void setTableModel(QStandardItemModel * model);

    int current;

};

#endif // MISSIONTOLTABLE_H
