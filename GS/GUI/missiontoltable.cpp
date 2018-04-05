#include "missiontoltable.h"

MissionTOLTable::MissionTOLTable(QWidget *parent) : QTableView(parent) {
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    horizontalHeader()->show();
}

void MissionTOLTable::setTableModel(QStandardItemModel *model) {
    setModel(model);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int c = 1; c < horizontalHeader()->count(); ++c) {
        horizontalHeader()->setSectionResizeMode(
            c, QHeaderView::ResizeToContents);
    }
    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MissionTOLTable::selectChanged);
}

void MissionTOLTable::selectChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    if (selected.indexes().length() > 0) {
        current = selected.indexes().at(0).row();
        // EMIT
    }
}
