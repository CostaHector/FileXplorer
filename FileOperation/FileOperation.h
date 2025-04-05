#ifndef FILEOPERATION_H
#define FILEOPERATION_H
#include "public/PublicVariable.h"
#include "FileOperatorPub.h"

namespace FileOperation {

using namespace FileOperatorType;

RETURN_TYPE rmpathAgent(const QStringList& parms);

RETURN_TYPE rmpath(const QString& pre, const QString& rel);

RETURN_TYPE mkpathAgent(const QStringList& parms);
RETURN_TYPE mkpath(const QString& pre, const QString& rel);

RETURN_TYPE rmfileAgent(const QStringList& parms);

RETURN_TYPE rmfile(const QString& pre, const QString& rel);

RETURN_TYPE rmdirAgent(const QStringList& parms);

RETURN_TYPE rmdir(const QString& pre, const QString& rel);

RETURN_TYPE moveToTrashAgent(const QStringList& parms);
RETURN_TYPE moveToTrash(const QString& pres, const QString& rels);

RETURN_TYPE renameAgent(const QStringList& parms);

RETURN_TYPE rename(const QString& pre, const QString& rel, const QString& to, const QString& toRel);
RETURN_TYPE cpfileAgent(const QStringList& parms);
RETURN_TYPE cpfile(const QString& pre, const QString& rel, const QString& to);
RETURN_TYPE cpdirAgent(const QStringList& parms);

RETURN_TYPE cpdir(const QString& pre, const QString& rel, const QString& to);

RETURN_TYPE touchAgent(const QStringList& parms);

RETURN_TYPE touch(const QString& pre, const QString& rel);

RETURN_TYPE executer(const BATCH_COMMAND_LIST_TYPE& aBatch, BATCH_COMMAND_LIST_TYPE& srcCommand);

RETURN_TYPE linkAgent(const QStringList& parms);

RETURN_TYPE link(const QString& pre, const QString& rel, const QString& to = SystemPath::starredPath);

RETURN_TYPE unlinkAgent(const QStringList& parms);

RETURN_TYPE unlink(const QString& pre, const QString& rel, const QString& to = SystemPath::starredPath);
}  // namespace FileOperation

#endif  // FILEOPERATION_H
