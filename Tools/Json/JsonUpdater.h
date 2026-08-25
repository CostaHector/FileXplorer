#ifndef JSONUPDATER_H
#define JSONUPDATER_H
#include "SceneMixed.h"
#include "JsonOp.h"
#include "JsonPr.h"
#include <QString>
#include <QVariantHash>

namespace JsonUpdater {
bool MergeTextContentsIntoDetailAndRecycleTxt(const QString& txtAbsPath, QString& detailContent);
JsonOp::Counter UpdateJsonKeyValuePair(const ScenesMixed& sMixed, QVariantHash& rawJsonDict, const QString& parentPath, const QString& jsonFileBaseName);
JsonOp::Counter UpdateJsonKeyValuePair(const ScenesMixed& sMixed, JsonPr& jsonPr);
};

#endif // JSONUPDATER_H
