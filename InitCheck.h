#ifndef INITCHECK_H
#define INITCHECK_H

#include "PublicVariable.h"

namespace InitCheck{
bool VerifyOneFilePath(const KV& kv, const QString& fileType = "txt");
bool VerifyOneFolderPath(const KV& kv);
bool InitOutterPlainTextPath();
}

#endif // INITCHECK_H
