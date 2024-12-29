#ifndef PUBLICTOOL_H
#define PUBLICTOOL_H

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QLayout>
#include <QString>
#include <QSqlDatabase>
#include <QTranslator>

void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);

QString ChooseCopyDestination(QString defaultPath, QWidget *parent=nullptr);
QString MoveCopyToRearrangeActionsText(const QString& first_path, QActionGroup* oldAG);

QSqlDatabase GetSqlVidsDB();

void LoadCNLanguagePack(QTranslator& translator);
void LoadSysLanaguagePack(QTranslator& translator);
#endif  // PUBLICTOOL_H
