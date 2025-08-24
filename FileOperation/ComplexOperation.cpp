#include "ComplexOperation.h"
#include "PathTool.h"
#include "MemoryKey.h"
#include "PublicVariable.h"
#include "UndoRedo.h"
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QAction>
#include <QDebug>

namespace ComplexOperation {
FILE_STRUCTURE_MODE g_fileStructureMode{FILE_STRUCTURE_MODE::QUERY};

void SetDefaultFileStructMode(const QAction* pDefaultMode) {
  if (pDefaultMode == nullptr) {
    qWarning("pDefaultMode is nullptr");
    return;
  }
  g_fileStructureMode = FILE_STRUCTURE_MODE_STR_2_ENUM.value(pDefaultMode->text(), FILE_STRUCTURE_MODE::QUERY);
  Configuration().setValue(MemoryKey::FILE_SYSTEM_STRUCTURE_WAY.name, (int)g_fileStructureMode);
}

FILE_STRUCTURE_MODE GetDefaultFileStructMode() {
  return g_fileStructureMode;
}

QStringList ComplexOperationBase::QUrls2FileAbsPaths(const QList<QUrl>& urls) {
  QStringList lAbsPathList;
  lAbsPathList.reserve(urls.size());
  for (const QUrl& url : urls) {
    lAbsPathList.append(url.toLocalFile());
  }
  return lAbsPathList;
}

QStringList ComplexOperationBase::MimeData2FileAbsPaths(const QMimeData& mimeData) {
  if (!mimeData.hasUrls()) {
    qWarning("No urls in mimedata at all");
    return {};
  }
  return QUrls2FileAbsPaths(mimeData.urls());
}

BATCH_COMMAND_LIST_TYPE ComplexOperationBase::FromQUrls(const QList<QUrl>& urls, const QString& dest, FILE_STRUCTURE_MODE mode) {
  return To(QUrls2FileAbsPaths(urls), dest, mode);
}

BATCH_COMMAND_LIST_TYPE ComplexOperationBase::FromMimeDataTo(const QMimeData& mimeData, const QString& dest, FILE_STRUCTURE_MODE mode) {  //
  return To(MimeData2FileAbsPaths(mimeData), dest, mode);
}

BATCH_COMMAND_LIST_TYPE ComplexMove::To(const QStringList& selectionAbsFilePaths, const QString& dest, FILE_STRUCTURE_MODE mode) {
  BATCH_COMMAND_LIST_TYPE lst;
  if (mode == FILE_STRUCTURE_MODE::FLATTEN) {
    QString prepath;
    for (const QString& pth : selectionAbsFilePaths) {
      QString name = PathTool::GetPrepathAndFileName(pth, prepath);
      lst.append(ACMD::GetInstMV(prepath, name, dest));
    }
  } else if (mode == FILE_STRUCTURE_MODE::PRESERVE) {
    QString rootPath;
    QStringList rel2Selections;  // indirect or direct
    std::tie(rootPath, rel2Selections) = PathTool::GetLAndRels(selectionAbsFilePaths);
    for (const QString& rel2Section : rel2Selections) {
      lst.append(ACMD::GetInstMV(rootPath, rel2Section, dest));
    }
  } else {
    qWarning("File Structure Mode[%d] not support", (int)mode);
  }
  return lst;
}

BATCH_COMMAND_LIST_TYPE ComplexCopy::To(const QStringList& selectionAbsFilePaths, const QString& dest, FILE_STRUCTURE_MODE mode) {
  BATCH_COMMAND_LIST_TYPE lst;
  if (mode == FILE_STRUCTURE_MODE::FLATTEN) {
    QString prepath;
    for (const QString& pth : selectionAbsFilePaths) {
      QString name = PathTool::GetPrepathAndFileName(pth, prepath);
      if (QFileInfo{pth}.isDir()) {
        lst.append(ACMD::GetInstCPDIR(prepath, name, dest));
      } else {
        lst.append(ACMD::GetInstCPFILE(prepath, name, dest));
      }
    }
  } else if (mode == FILE_STRUCTURE_MODE::PRESERVE) {
    QString rootPath;
    QStringList rel2Selections;  // indirect or direct
    std::tie(rootPath, rel2Selections) = PathTool::GetLAndRels(selectionAbsFilePaths);
    for (const QString& rel2Section : rel2Selections) {
      if (QFileInfo{rootPath + '/' + rel2Section}.isDir()) {
        lst.append(ACMD::GetInstCPDIR(rootPath, rel2Section, dest));
      } else {
        lst.append(ACMD::GetInstCPFILE(rootPath, rel2Section, dest));
      }
    }
  } else {
    qWarning("File Structure Mode[%d] not support", (int)mode);
  }
  return lst;
}

BATCH_COMMAND_LIST_TYPE ComplexLink::To(const QStringList& selectionAbsFilePaths, const QString& dest, FILE_STRUCTURE_MODE /*mode*/) {
  BATCH_COMMAND_LIST_TYPE lst;
  QString rootPath;
  QStringList rel2Selections;  // indirect or direct
  std::tie(rootPath, rel2Selections) = PathTool::GetLAndRels(selectionAbsFilePaths);
  for (const QString& rel2Section : rel2Selections) {
    lst.append(ACMD::GetInstLINK(rootPath, rel2Section, dest));
  }
  return lst;
}

BATCH_COMMAND_LIST_TYPE ComplexMerge::Merge(const QString& src, const QString& dest) {
  ComplexMove cm;
  const QStringList files{QDir{src}.entryList({}, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::DirsLast | QDir::SortFlag::Name)};
  QStringList selectionAbsFilePaths;
  selectionAbsFilePaths.reserve(files.size());
  for (const QString& fileName : files) {
    selectionAbsFilePaths.append(src + '/' + fileName);
  }
  auto ans = cm.To(selectionAbsFilePaths, dest, FILE_STRUCTURE_MODE::FLATTEN);
  QString prepath;
  QString srcFileName = PathTool::GetPrepathAndFileName(src, prepath);
  ans.append(ACMD::GetInstRMDIR(prepath, srcFileName));
  return ans;
}

int DoDropAction(Qt::DropAction dropAct, const QList<QUrl>& urls, const QString& dest, FILE_STRUCTURE_MODE mode) {
  BATCH_COMMAND_LIST_TYPE aBatch;
  QString dropActionStr;
  switch (dropAct) {
    case Qt::DropAction::CopyAction: {
      ComplexCopy cc;
      aBatch = cc.FromQUrls(urls, dest, mode);
      dropActionStr = "Copy";
      break;
    }
    case Qt::DropAction::MoveAction: {
      ComplexMove cm;
      aBatch = cm.FromQUrls(urls, dest, mode);
      dropActionStr = "Move";
      break;
    }
    case Qt::DropAction::LinkAction: {
      ComplexLink cl;
      aBatch = cl.FromQUrls(urls, dest, mode);
      dropActionStr = "Link";
      break;
    }
    default: {
      qWarning("Drop action[%d] not support now", (int)dropAct);
      return 0;
    }
  }

  bool bDropResult = g_undoRedo.Do(aBatch);
  if (!bDropResult) {
    qWarning("Drop[%s] partially failed %d selection(s) %d command(s) drop into path[%s]",  //
             qPrintable(dropActionStr), urls.size(), aBatch.size(), qPrintable(dest));
    return -1;
  }
  qWarning("Drop[%s] all succeed %d selection(s) %d command(s) drop into path[%s]",  //
           qPrintable(dropActionStr), urls.size(), aBatch.size(), qPrintable(dest));
  return 0;
}

int DoDropAction(Qt::DropAction dropAct, const QStringList& absPaths, const QString& dest, FILE_STRUCTURE_MODE mode) {
  BATCH_COMMAND_LIST_TYPE aBatch;
  QString dropActionStr;
  switch (dropAct) {
    case Qt::DropAction::CopyAction: {
      ComplexCopy cc;
      aBatch = cc.To(absPaths, dest, mode);
      dropActionStr = "Copy";
      break;
    }
    case Qt::DropAction::MoveAction: {
      ComplexMove cm;
      aBatch = cm.To(absPaths, dest, mode);
      dropActionStr = "Move";
      break;
    }
    case Qt::DropAction::LinkAction: {
      ComplexLink cl;
      aBatch = cl.To(absPaths, dest, mode);
      dropActionStr = "Link";
      break;
    }
    default: {
      qWarning("Drop action[%d] not support now", (int)dropAct);
      return 0;
    }
  }

  bool bDropResult = g_undoRedo.Do(aBatch);
  if (!bDropResult) {
    qWarning("Drop[%s] partially failed %d selection(s) %d command(s) drop into path[%s]",  //
             qPrintable(dropActionStr), absPaths.size(), aBatch.size(), qPrintable(dest));
    return -1;
  }
  qWarning("Drop[%s] all succeed %d selection(s) %d command(s) drop into path[%s]",  //
           qPrintable(dropActionStr), absPaths.size(), aBatch.size(), qPrintable(dest));
  return 0;
}

Qt::DropAction GetCutCopyModeFromNativeMimeData(const QMimeData& native) {
#ifdef _WIN32
  if (native.hasFormat("Preferred DropEffect")) {
    const QByteArray& ba = native.data("Preferred DropEffect");
    if (ba[0] == 0x2) {  // # 2 for cut and 5 for copy
      return Qt::DropAction::MoveAction;
    } else if (ba[0] == 0x5) {
      return Qt::DropAction::CopyAction;
    }
    qWarning("Preferred DropEffect value[%d] invalid", (int)ba[0]);
  }
#else
  if (native.hasFormat("x-special/gnome-copied-files")) {
    QByteArray ba = native.data("x-special/gnome-copied-files");
    const QString cutOrCopyAction = QString::fromUtf8(ba);
    if (cutOrCopyAction.startsWith("cut")) {
      return Qt::DropAction::MoveAction;
    } else if (cutOrCopyAction.startsWith("copy")) {
      return Qt::DropAction::CopyAction;
    }
    qWarning("x-special/gnome-copied-files value[%s] invalid", qPrintable(cutOrCopyAction));
  } else if (native.hasFormat("XdndAction")) {
    QByteArray ba = native.data("XdndAction");
    const QString cutOrCopyAction = QString::fromUtf8(ba);
    if (cutOrCopyAction == "XdndActionMove") {  // 0xx
      return Qt::DropAction::MoveAction;
    } else if (cutOrCopyAction == "XdndActionCopy") {
      return Qt::DropAction::CopyAction;  // 0x1
    }
    qWarning("XdndAction value[%s] invalid", qPrintable(cutOrCopyAction));
  }
#endif
  qWarning() << "Action not found. Supported Available format:" << native.formats();
  return Qt::DropAction::IgnoreAction;
}

}  // namespace ComplexOperation
