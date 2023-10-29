#include "Learning_Database.h"
#include <Qt>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTableView>
#include <QApplication>

void testSql() {
    {
        // default connection instance
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "notDefaultConnectName");
        db.setHostName("acidalia");
        db.setDatabaseName("customdb");
        db.setUserName("mojito");
        db.setPassword("J0a1m8");
        qDebug() << "drives() " << QSqlDatabase::drivers();
        qDebug() << "connectionNames() " << QSqlDatabase::connectionNames();
        qDebug() << "db.isOpen()" << db.isOpen();
        bool ok = db.open();
        qDebug() << "db.isOpen()" << db.isOpen();
        qDebug() << "db.tables() " << db.tables();

        if (not db.tables().contains("contacts")) {
            QSqlQuery createAndInitTableQry(db);
            createAndInitTableQry.exec(
                        "CREATE TABLE contacts ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                        "name VARCHAR(40) NOT NULL,"
                        "job VARCHAR(50),"
                        "email VARCHAR(40) NOT NULL"
                        ")");
            createAndInitTableQry.exec(QString("INSERT INTO contacts (name, job, email)"
                                               "VALUES ('%1', '%2', '%3')")
                                       .arg("Linda", "Technical Lead", "linda@example.com"));
        }
        QSqlQuery simpleQry(db);
        simpleQry.exec("SELECT name, job, email FROM contacts");
        qDebug() << simpleQry.record().indexOf("name");
        //    if (simpleQry.first()) {
        //      qDebug() << simpleQry.value("name").toString();
        //    }
        while (simpleQry.next()) {
            qDebug() << simpleQry.value(0).toString() << "," << simpleQry.value(1).toString() << "," << simpleQry.value(2).toString();
        }
        if (db.isOpen()) {
            db.close();
        }
    }

    if (QSqlDatabase::contains("notDefaultConnectName")) {
        QSqlDatabase::removeDatabase("notDefaultConnectName");
    }
}

Learning_Database::Learning_Database(QWidget* parent):QMainWindow(parent) {
    // Set up the model
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("customdb");
    qDebug() << db.isOpen() <<"|username:" <<db.userName() << "|password:" << db.password() <<"|";
    // QSqlDatabase::removeDatabase(con1.connectionName())

    auto* model = new QSqlTableModel(this);
    model->setTable("contacts");
    model->setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Name");
    model->setHeaderData(2, Qt::Horizontal, "Job");
    model->setHeaderData(3, Qt::Horizontal, "Email");
    model->select();
    // Set up the view
    auto* view = new QTableView;
    view->setModel(model);
    view->resizeColumnsToContents();
    setCentralWidget(view);

    setWindowTitle("Learning_Database Example");
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
    //    testSql();
    QApplication a(argc, argv);
    Learning_Database wid;
    wid.show();
    return a.exec();
}
#endif
