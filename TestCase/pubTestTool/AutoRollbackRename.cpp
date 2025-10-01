#include "AutoRollbackRename.h"
#include "Logger.h"
#include <QFile>
#include <QDir>

AutoRollbackRename::AutoRollbackRename(QString srcPath, QString dstPath)  //
  : mSrcAbsFilePath(std::move(srcPath)), mDstAbsFilePath(std::move(dstPath)) {}

AutoRollbackRename::AutoRollbackRename(const QString& prepath, const QString& relSrc1, const QString& relDst2)  //
  : AutoRollbackRename{QDir{prepath}.absoluteFilePath(relSrc1), QDir{prepath}.absoluteFilePath(relDst2)} {}

bool AutoRollbackRename::Execute() {
  mNeedRollback = StartToRename("Rename to");
  if (mNeedRollback) {
    LOG_D("Rename ok. in rollback src is dst here and dst is src here");
    mSrcAbsFilePath.swap(mDstAbsFilePath);
  }
  return mNeedRollback;
}

bool AutoRollbackRename::StartToRename(const QString& hintMsg) {
  if (!QFile::exists(mSrcAbsFilePath)) {
    mNeedRollback = false;
    LOG_W("[%s] src file[%s] not exist. no need process", qPrintable(hintMsg), qPrintable(mSrcAbsFilePath));
    return false;
  }
  if (QFile::exists(mDstAbsFilePath)) {
    mNeedRollback = false;
    LOG_W("[%s] dst file name[%s] already occupied. no need process", qPrintable(hintMsg), qPrintable(mDstAbsFilePath));
    return false;
  }
  if (!QFile::rename(mSrcAbsFilePath, mDstAbsFilePath)) {
    LOG_W("[%s] Rename[%s]->[%s] to failed. no need process", qPrintable(hintMsg), qPrintable(mSrcAbsFilePath), qPrintable(mDstAbsFilePath));
    mNeedRollback = false;
    return false;
  }
  return true;
}

AutoRollbackRename::~AutoRollbackRename() {
  if (!mNeedRollback) {
    LOG_D("Before rename failed or not execute, no need rollback here");
    return;
  }
  StartToRename("Rename Rollback");
}

