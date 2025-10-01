#ifndef AUTOROLLBACKFILECONTENTMODIFY_H
#define AUTOROLLBACKFILECONTENTMODIFY_H
#include <QString>

struct AutoRollbackFileContentModify final {
public:
  AutoRollbackFileContentModify(const QString& absFilePath, const QString& replaceeStr, const QString& replacerStr);
  AutoRollbackFileContentModify(const QString& absFilePath, const QString& newContents);
  ~AutoRollbackFileContentModify();
  bool Execute();

private:
  enum class Mode { ReplaceMode, FullReplaceMode };
  bool StartToModify(const QString& hintMsg);
  const QString mAbsFilePath;
  const QString mReplaceeStr, mReplacerStr;  // replacee->replacer
  const QString mNewContents;                // full replace
  const Mode mMode;
  QString mOriginContents;
  bool mNeedRollback{false};
};

#endif // AUTOROLLBACKFILECONTENTMODIFY_H
