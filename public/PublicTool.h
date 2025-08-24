#ifndef PUBLICTOOL_H
#define PUBLICTOOL_H

#include <QActionGroup>
#include <QLayout>
#include <QString>
#include <QTranslator>
#include <QIODevice>

QString MoveToNewPathAutoUpdateActionText(const QString& first_path, QActionGroup* oldAG);
QString TextReader(const QString& textPath);
bool TextWriter(const QString& fileName, const QString& content = "", const QIODevice::OpenMode openMode = QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
bool ByteArrayWriter(const QString& fileName, const QByteArray& ba);
QString ChooseCopyDestination(QString defaultPath, QWidget* parent = nullptr);

void LoadCNLanguagePack(QTranslator& translator);
void LoadSysLanaguagePack(QTranslator& translator);

bool CreateUserPath();
#endif  // PUBLICTOOL_H
