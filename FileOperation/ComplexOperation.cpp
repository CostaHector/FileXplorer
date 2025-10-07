#include "ComplexOperation.h"
#include "PathTool.h"
#include "MemoryKey.h"
#include "PublicVariable.h"
#include "Logger.h"
#include "UndoRedo.h"

#include <QFileInfo>
#include <QDir>
#include <QUrl>

namespace ComplexOperation {

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
    LOG_W("No urls in mimedata at all");
    return {};
  }
  return QUrls2FileAbsPaths(mimeData.urls());
}

BATCH_COMMAND_LIST_TYPE ComplexOperationBase::FromQUrls(const QList<QUrl>& urls, const QString& dest, FileStuctureModeE mode) {
  return To(QUrls2FileAbsPaths(urls), dest, mode);
}

BATCH_COMMAND_LIST_TYPE ComplexMove::To(const QStringList& selectionAbsFilePaths, const QString& dest, FileStuctureModeE mode) {
  BATCH_COMMAND_LIST_TYPE lst;
  if (mode == FileStuctureModeE::FLATTEN) {
    QString prepath;
    for (const QString& pth : selectionAbsFilePaths) {
      QString name = PathTool::GetPrepathAndFileName(pth, prepath);
      lst.append(ACMD::GetInstMV(prepath, name, dest));
    }
  } else if (mode == FileStuctureModeE::PRESERVE) {
    QString rootPath;
    QStringList rel2Selections;  // indirect or direct
    std::tie(rootPath, rel2Selections) = PathTool::GetLAndRels(selectionAbsFilePaths);
    for (const QString& rel2Section : rel2Selections) {
      lst.append(ACMD::GetInstMV(rootPath, rel2Section, dest));
    }
  } else {
    LOG_W("File Structure Mode[%s] not support", c_str(mode));
  }
  return lst;
}

BATCH_COMMAND_LIST_TYPE ComplexCopy::To(const QStringList& selectionAbsFilePaths, const QString& dest, FileStuctureModeE mode) {
  BATCH_COMMAND_LIST_TYPE lst;
  if (mode == FileStuctureModeE::FLATTEN) {
    QString prepath;
    for (const QString& pth : selectionAbsFilePaths) {
      QString name = PathTool::GetPrepathAndFileName(pth, prepath);
      if (QFileInfo{pth}.isDir()) {
        lst.append(ACMD::GetInstCPDIR(prepath, name, dest));
      } else {
        lst.append(ACMD::GetInstCPFILE(prepath, name, dest));
      }
    }
  } else if (mode == FileStuctureModeE::PRESERVE) {
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
    LOG_W("File Structure Mode[%s] not support", c_str(mode));
  }
  return lst;
}

BATCH_COMMAND_LIST_TYPE ComplexLink::To(const QStringList& selectionAbsFilePaths, const QString& dest, FileStuctureModeE mode) {
  BATCH_COMMAND_LIST_TYPE lst;
  if (mode == FileStuctureModeE::FLATTEN) {
    QString prepath;
    for (const QString& pth : selectionAbsFilePaths) {
      QString name = PathTool::GetPrepathAndFileName(pth, prepath);
      lst.append(ACMD::GetInstLINK(prepath, name, dest));
    }
  } else if (mode == FileStuctureModeE::PRESERVE) {
    QString rootPath;
    QStringList rel2Selections;  // indirect or direct
    std::tie(rootPath, rel2Selections) = PathTool::GetLAndRels(selectionAbsFilePaths);
    for (const QString& rel2Section : rel2Selections) {
      lst.append(ACMD::GetInstLINK(rootPath, rel2Section, dest));
    }
  } else {
    LOG_W("File Structure Mode[%s] not support", c_str(mode));
  }

  return lst;
}

BATCH_COMMAND_LIST_TYPE ComplexMerge::Merge(const QString& src, const QString& dest) {
  ComplexMove cm;
  const QStringList files{
      QDir{src}.entryList({}, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::DirsLast | QDir::SortFlag::Name)};
  QStringList selectionAbsFilePaths;
  selectionAbsFilePaths.reserve(files.size());
  for (const QString& fileName : files) {
    selectionAbsFilePaths.append(src + '/' + fileName);
  }
  auto ans = cm.To(selectionAbsFilePaths, dest, FileStuctureModeE::FLATTEN);
  QString prepath;
  QString srcFileName = PathTool::GetPrepathAndFileName(src, prepath);
  ans.append(ACMD::GetInstRMDIR(prepath, srcFileName));
  return ans;
}

int DoDropAction(Qt::DropAction dropAct, const QList<QUrl>& urls, const QString& dest, FileStuctureModeE mode) {
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
      LOG_W("Drop action[%d] not support now", (int)dropAct);
      return 0;
    }
  }

  bool bDropResult = UndoRedo::GetInst().Do(aBatch);
  if (!bDropResult) {
    LOG_W("Drop[%s] partially failed %d selection(s) %d command(s) drop into path[%s]",  //
          qPrintable(dropActionStr), urls.size(), aBatch.size(), qPrintable(dest));
    return -1;
  }
  LOG_W("Drop[%s] all succeed %d selection(s) %d command(s) drop into path[%s]",  //
        qPrintable(dropActionStr), urls.size(), aBatch.size(), qPrintable(dest));
  return 0;
}

int DoDropAction(Qt::DropAction dropAct, const QStringList& absPaths, const QString& dest, FileStuctureModeE mode) {
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
      LOG_W("Drop action[%d] not support now", (int)dropAct);
      return 0;
    }
  }

  bool bDropResult = UndoRedo::GetInst().Do(aBatch);
  if (!bDropResult) {
    LOG_W("Drop[%s] partially failed %d selection(s) %d command(s) drop into path[%s]",  //
          qPrintable(dropActionStr), absPaths.size(), aBatch.size(), qPrintable(dest));
    return -1;
  }
  LOG_W("Drop[%s] all succeed %d selection(s) %d command(s) drop into path[%s]",  //
        qPrintable(dropActionStr), absPaths.size(), aBatch.size(), qPrintable(dest));
  return 0;
}

Qt::DropAction GetCutCopyModeFromNativeMimeData(const QMimeData& native) {
#ifdef _WIN32
  if (native.hasFormat("Preferred DropEffect")) {
    const QByteArray& ba = native.data("Preferred DropEffect");
    if (ba[0] == 0x2) {  // # 2 for cut and 5 for copy
      return Qt::DropAction::MoveAction;
    } else if (ba[0] == 0x1) {
      return Qt::DropAction::CopyAction;
    } else if (ba[0] == 0x4) {
      return Qt::DropAction::LinkAction;
    } else {
      LOG_W("Preferred DropEffect value[%d] invalid", (int)ba[0]);
    }
  }
#else
  if (native.hasFormat("x-special/gnome-copied-files")) {
    QByteArray ba = native.data("x-special/gnome-copied-files");
    const QString cutOrCopyAction = QString::fromUtf8(ba);
    if (cutOrCopyAction.startsWith("cut")) {
      return Qt::DropAction::MoveAction;
    } else if (cutOrCopyAction.startsWith("copy")) {
      return Qt::DropAction::CopyAction;
    } else if (cutOrCopyAction.startsWith("link")) {
      return Qt::DropAction::LinkAction;
    }
    LOG_W("x-special/gnome-copied-files value[%s] invalid", qPrintable(cutOrCopyAction));
  } else if (native.hasFormat("XdndAction")) {
    QByteArray ba = native.data("XdndAction");
    const QString cutOrCopyAction = QString::fromUtf8(ba);
    if (cutOrCopyAction == "XdndActionMove") {  // 0xx
      return Qt::DropAction::MoveAction;
    } else if (cutOrCopyAction == "XdndActionCopy") {
      return Qt::DropAction::CopyAction;  // 0x1
    } else if (cutOrCopyAction == "XdndActionLink") {
      return Qt::DropAction::LinkAction;  // 0x1
    }
    LOG_W("XdndAction value[%s] invalid", qPrintable(cutOrCopyAction));
  }
#endif
  const QStringList& formats = native.formats();
  const QString& supportAvails = formats.join(',');
  LOG_W("Action not found. Supported Available format: %s.", qPrintable(supportAvails));
  return Qt::DropAction::IgnoreAction;
}

}  // namespace ComplexOperation
