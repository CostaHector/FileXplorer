#include "AutoRollbackFileContentModify.h"
#include "FileTool.h"
#include "Logger.h"

AutoRollbackFileContentModify::AutoRollbackFileContentModify(const QString& absFilePath, const QString& replaceeStr, const QString& replacerStr)
  : mAbsFilePath(absFilePath), mReplaceeStr(replaceeStr), mReplacerStr(replacerStr), mMode(Mode::ReplaceMode) {}

AutoRollbackFileContentModify::AutoRollbackFileContentModify(const QString& absFilePath, const QString& newContents)
  : mAbsFilePath(absFilePath), mNewContents(newContents), mMode(Mode::FullReplaceMode) {}

AutoRollbackFileContentModify::~AutoRollbackFileContentModify() {
  if (!mNeedRollback) {
    return;
  }
  const bool bRollbackResult = FileTool::ByteArrayWriter(mAbsFilePath, mOriginContents.toUtf8());
  if (!bRollbackResult) {
    LOG_W("Rollback file[%s] contents failed", qPrintable(mAbsFilePath));
  }
}

bool AutoRollbackFileContentModify::Execute() {
  if (mNeedRollback) {
    LOG_W("Already executed. Prevent modify it again");
    return false;
  }
  bool bReadOk = false;
  QString content = FileTool::TextReader(mAbsFilePath, &bReadOk);
  if (!bReadOk) {
    LOG_W("Read Content from file[%s] failed", qPrintable(mAbsFilePath));
    return false;
  }
  mOriginContents.swap(content);

  bool bSuccess = false;
  switch (mMode) {
    case Mode::ReplaceMode: {
      QString tempStr = mOriginContents;
      tempStr.replace(mReplaceeStr, mReplacerStr);
      bSuccess = FileTool::ByteArrayWriter(mAbsFilePath, tempStr.toUtf8());
      break;
    }
    case Mode::FullReplaceMode: {
      bSuccess = FileTool::ByteArrayWriter(mAbsFilePath, mNewContents.toUtf8());
      break;
    }
    default:
      LOG_W("mMode[%d] invalid", (int)mMode);
      return false;
  }

  if (!bSuccess) {
    LOG_W("File Content[%s] Modify Operation[%d] failed", qPrintable(mAbsFilePath), (int)mMode);
    return false;
  }

  mNeedRollback = true;
  return true;
}

