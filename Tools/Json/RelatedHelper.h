#ifndef RELATEDHELPER_H
#define RELATEDHELPER_H

#include <QStringList>
#include <QSet>

namespace RelatedHelper {
bool getJsonPathFromFile(QString fileAbsPath, QString& jsonPath);
QSet<QString> GetRelatedJsonAbsPaths(const QStringList& paths);
QString getBaseNameForImage(const QString& imagePath);
}

#endif // RELATEDHELPER_H
