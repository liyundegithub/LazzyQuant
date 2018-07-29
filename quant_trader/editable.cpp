#include "editable.h"

void Editable::setup() {
    model.setTable(signature);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    loadFromDB();
}

void Editable::loadFromDB() {
    model.select();
}
