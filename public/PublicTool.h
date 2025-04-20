#ifndef PUBLICTOOL_H
#define PUBLICTOOL_H

#include <QActionGroup>
#include <QLayout>
#include <QString>
#include <QSqlDatabase>
#include <QTranslator>

QString MoveToNewPathAutoUpdateActionText(const QString& first_path, QActionGroup* oldAG);
QString TextReader(const QString& textPath);
bool TextWriter(const QString& fileName, const QString& content="", const QIODevice::OpenMode openMode = QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);
QString ChooseCopyDestination(QString defaultPath, QWidget *parent=nullptr);
QSqlDatabase GetSqlVidsDB();

void LoadCNLanguagePack(QTranslator& translator);
void LoadSysLanaguagePack(QTranslator& translator);

#endif  // PUBLICTOOL_H
