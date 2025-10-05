#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#include <QString>
struct AccountInfo {
 public:
  AccountInfo(const QString& _typeStr,
              const QString& _nameStr,
              const QString& _accountStr,
              const QString& _pwdStr,
              const QString& _othersStr)
      : typeStr{_typeStr}
        , nameStr{_nameStr}
        , accountStr{_accountStr}
        , pwdStr{_pwdStr}
        , othersStr{_othersStr} {}
  AccountInfo() = default;

  bool IsContainsKeyWords(const QString& keywords) const {
    return keywords.isEmpty() || typeStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive)
           || nameStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive)
           || accountStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive)
           || pwdStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive)
           || othersStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive);
  }

  QString toCsvLine() const;
  static bool FromCsvLine(const QString& csvLine, AccountInfo& acc);
  static const QStringList HORIZONTAL_HEAD;

  QString typeStr;
  QString nameStr;
  QString accountStr;
  QString pwdStr;
  QString othersStr;

  void SetDetailModified() const { mIsDetailModified = true; }
  void ClearDetailModified() const { mIsDetailModified = false; }
  bool IsDetailModified() const { return mIsDetailModified; }

 private:
  mutable bool mIsDetailModified{false};
};

#endif
