#ifndef NFOXMLHELPER_H
#define NFOXMLHELPER_H

#include <QString>

namespace NfoXmlHelper {
int GenerateJsonsFromNfoXMLs(const QStringList& nfoXmlFileFullPaths);
bool GenerateJsonFromNfoXML(const QString& nfoXmlFileFullPath);
}

#endif // NFOXMLHELPER_H
