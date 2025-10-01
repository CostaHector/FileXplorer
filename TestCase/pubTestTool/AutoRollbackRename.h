#ifndef AUTOROLLBACKRENAME_H
#define AUTOROLLBACKRENAME_H

#include <QString>

struct AutoRollbackRename final {
public:
  AutoRollbackRename(QString srcPath, QString dstPath);
  AutoRollbackRename(const QString& prepath, const QString& relSrc1, const QString& relDst2);
  ~AutoRollbackRename();
  bool Execute();

private:
  bool StartToRename(const QString& hintMsg);
  QString mSrcAbsFilePath, mDstAbsFilePath;
  bool mNeedRollback{false};
};

#endif // AUTOROLLBACKRENAME_H
