#ifndef COMPLEXOPERATION_H
#define COMPLEXOPERATION_H
#include <QString>
#include <QMimedata>
#include "FileOperatorPub.h"
class QAction;

namespace ComplexOperation {
using namespace FileOperatorType;

#define FILE_STRUCTURE_MODE_FIELD_MAPPING \
  FILE_STRUCTURE_MODE_ITEM(QUERY, 0)      \
  FILE_STRUCTURE_MODE_ITEM(PRESERVE, 1)       \
  FILE_STRUCTURE_MODE_ITEM(FLATTEN, 2)

enum class FILE_STRUCTURE_MODE {  //
  BEGIN = 0,                      //
#define FILE_STRUCTURE_MODE_ITEM(enu, val) enu = val,
  FILE_STRUCTURE_MODE_FIELD_MAPPING
#undef FILE_STRUCTURE_MODE_ITEM
      BUTT
};

const QString FILE_STRUCTURE_MODE_STR[(int)FILE_STRUCTURE_MODE::BUTT + 1]{
#define FILE_STRUCTURE_MODE_ITEM(enu, val) ENUM_2_STR(enu),
    FILE_STRUCTURE_MODE_FIELD_MAPPING
#undef FILE_STRUCTURE_MODE_ITEM
        ENUM_2_STR(BUTT),
};

const QMap<QString, FILE_STRUCTURE_MODE> FILE_STRUCTURE_MODE_STR_2_ENUM{
#define FILE_STRUCTURE_MODE_ITEM(enu, val) {ENUM_2_STR(enu), FILE_STRUCTURE_MODE::enu},
    FILE_STRUCTURE_MODE_FIELD_MAPPING
#undef FILE_STRUCTURE_MODE_ITEM
};

void SetDefaultFileStructMode(const QAction* pDefaultMode);
FILE_STRUCTURE_MODE GetDefaultFileStructMode();

struct ComplexOperationBase {
  // built-in view selection from QMimedata
  static QStringList QUrls2FileAbsPaths(const QList<QUrl>& mimeData);
  static QStringList MimeData2FileAbsPaths(const QMimeData& mimeData);
  BATCH_COMMAND_LIST_TYPE FromQUrls(const QList<QUrl>& urls, const QString& dest, FILE_STRUCTURE_MODE mode);
  BATCH_COMMAND_LIST_TYPE FromMimeDataTo(const QMimeData& mimeData, const QString& dest, FILE_STRUCTURE_MODE mode);
  virtual BATCH_COMMAND_LIST_TYPE To(const QStringList& selectionAbsFilePaths, const QString& dest, FILE_STRUCTURE_MODE mode) = 0;
};

struct ComplexMove : public ComplexOperationBase {
  BATCH_COMMAND_LIST_TYPE To(const QStringList& selectionAbsFilePaths, const QString& dest, FILE_STRUCTURE_MODE mode) override;
};

struct ComplexCopy : public ComplexOperationBase {
  BATCH_COMMAND_LIST_TYPE To(const QStringList& selectionAbsFilePaths, const QString& dest, FILE_STRUCTURE_MODE mode) override;
};

struct ComplexLink : public ComplexOperationBase {
  BATCH_COMMAND_LIST_TYPE To(const QStringList& selectionAbsFilePaths, const QString& dest, FILE_STRUCTURE_MODE mode) override;
};

struct ComplexMerge {
  BATCH_COMMAND_LIST_TYPE Merge(const QString& src, const QString& dest);
};

Qt::DropAction GetCutCopyModeFromNativeMimeData(const QMimeData& native);
int DoDropAction(Qt::DropAction dropAct, const QList<QUrl>& urls, const QString& dest, FILE_STRUCTURE_MODE mode);
int DoDropAction(Qt::DropAction dropAct, const QStringList& absPaths, const QString& dest, FILE_STRUCTURE_MODE mode);
}  // namespace ComplexOperation

#endif  // COMPLEXOPERATION_H
