#include "JsonFileHelper.h"

JsonFileHelper::JsonFileHelper() {}

const QMap<QString, QString> JsonFileHelper::key2ValueType = {{JSONKey::Performers, "QStringList"},
                                                              {JSONKey::Tags, "QStringList"},
                                                              {JSONKey::Hot, "QIntList"},
                                                              {JSONKey::Rate, "int"}};
