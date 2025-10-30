#ifndef PUBLICTOOL_H
#define PUBLICTOOL_H

#include <QActionGroup>
#include <QLayout>
#include <QString>
#include <QTranslator>
#include <QIODevice>

namespace FileTool {
QByteArray GetLastNLinesOfFile(const QString& logFilePath, const int maxLines = 100);
QString TextReader(const QString& textPath, bool* bReadOk=nullptr);
QByteArray ByteArrayReader(const QString& baFilePath, bool* bReadOk=nullptr);
bool TextWriter(const QString& fileName, const QString& content = "", const QIODevice::OpenMode openMode = QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
bool ByteArrayWriter(const QString& fileName, const QByteArray& ba);
bool OpenLocalFileUsingDesktopService(const QString& localFilePath);
}

QString ChooseCopyDestination(QString defaultPath, QWidget* parent = nullptr);
QString MoveToNewPathAutoUpdateActionText(const QString& first_path, QActionGroup* oldAG);

bool LoadCNLanguagePack(QTranslator& translator, QString qmName="");
bool CreateUserPath();
#endif  // PUBLICTOOL_H
