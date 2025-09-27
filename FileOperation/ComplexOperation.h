#ifndef COMPLEXOPERATION_H
#define COMPLEXOPERATION_H
#include <QString>
#include <QMimeData>
#include "FileOperatorPub.h"
#include "FileStructurePolicy.h"

namespace ComplexOperation {
using namespace FileOperatorType;
using namespace FileStructurePolicy;
struct ComplexOperationBase {
  // built-in view selection from QMimedata
  static QStringList QUrls2FileAbsPaths(const QList<QUrl>& mimeData);
  static QStringList MimeData2FileAbsPaths(const QMimeData& mimeData);
  BATCH_COMMAND_LIST_TYPE FromQUrls(const QList<QUrl>& urls, const QString& dest, FileStuctureModeE mode);
  virtual BATCH_COMMAND_LIST_TYPE To(const QStringList& selectionAbsFilePaths, const QString& dest, FileStuctureModeE mode) = 0;
};

struct ComplexMove : public ComplexOperationBase {
  BATCH_COMMAND_LIST_TYPE To(const QStringList& selectionAbsFilePaths, const QString& dest, FileStuctureModeE mode) override;
};

struct ComplexCopy : public ComplexOperationBase {
  BATCH_COMMAND_LIST_TYPE To(const QStringList& selectionAbsFilePaths, const QString& dest, FileStuctureModeE mode) override;
};

struct ComplexLink : public ComplexOperationBase {
  BATCH_COMMAND_LIST_TYPE To(const QStringList& selectionAbsFilePaths, const QString& dest, FileStuctureModeE mode) override;
};

struct ComplexMerge {
  BATCH_COMMAND_LIST_TYPE Merge(const QString& src, const QString& dest);
};

Qt::DropAction GetCutCopyModeFromNativeMimeData(const QMimeData& native);
int DoDropAction(Qt::DropAction dropAct, const QList<QUrl>& urls, const QString& dest, FileStuctureModeE mode);
int DoDropAction(Qt::DropAction dropAct, const QStringList& absPaths, const QString& dest, FileStuctureModeE mode);
}  // namespace ComplexOperation

#endif  // COMPLEXOPERATION_H
