#include "MyQSqlTableModel.h"

MyQSqlTableModel::MyQSqlTableModel(QObject *parent, QSqlDatabase con)
    : QSqlTableModel{parent, con}
{

}
