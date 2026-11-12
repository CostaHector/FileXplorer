#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#include <QString>
struct AccountInfo {
 public:
  AccountInfo(const QString& _typeStr, const QString& _nameStr, const QString& _accountStr, const QString& _pwdStr, const QString& _othersStr)
      : typeStr{_typeStr}, nameStr{_nameStr}, accountStr{_accountStr}, pwdStr{_pwdStr}, othersStr{_othersStr} {}
  AccountInfo() = default;

  bool IsContainsKeyWords(const QString& keywords) const {
    return keywords.isEmpty()//
           || typeStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive)//
           || nameStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive)//
           || accountStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive)//
           || pwdStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive)//
           || othersStr.contains(keywords, Qt::CaseSensitivity::CaseInsensitive);
  }

  bool operator==(const AccountInfo& rhs) const {
    return typeStr == rhs.typeStr && nameStr == rhs.nameStr && accountStr == rhs.accountStr && pwdStr == rhs.pwdStr && othersStr == rhs.othersStr;
  }

  void swap(AccountInfo& rhs) {
    typeStr.swap(rhs.typeStr);
    nameStr.swap(rhs.nameStr);
    accountStr.swap(rhs.accountStr);
    pwdStr.swap(rhs.pwdStr);
    othersStr.swap(rhs.othersStr);
    std::swap(mIsModified, rhs.mIsModified);
    std::swap(mIsDeleted, rhs.mIsDeleted);
  }

  QString toCsvLine() const;
  static bool FromCsvLine(const QString& csvLine, AccountInfo& acc);

  enum Role {
    DEF_BEGIN_ROLE = Qt::UserRole + 1,
    TYPE_STR_ROLE = DEF_BEGIN_ROLE,
    NAME_STR_ROLE,
    ACCOUNT_STR_ROLE,
    PWD_STR_ROLE,
    OTHERS_STR_ROLE,
    IS_MODIFIED_ROLE,
    IS_DELETED_ROLE,
    INVALID_BUTT_ROLE,
  };

  QString typeStr;
  QString nameStr;
  QString accountStr;
  QString pwdStr;
  QString othersStr;
  mutable bool mIsModified{false};
  mutable bool mIsDeleted{false};

  void SetModified(bool modified) const { mIsModified = modified; }
  bool IsModified()               const { return mIsModified; }
  void   SetDeleted(bool deleted) const { mIsDeleted = deleted; }
  bool   IsDeleted()              const { return mIsDeleted; }

  static bool IsColumnCannotEditByUser(int column) {
    return column == Role2Column(Role::IS_MODIFIED_ROLE) || column == Role2Column(Role::IS_DELETED_ROLE);
  }
  static constexpr int Role2Column(Role role) {
    return role - Role::DEF_BEGIN_ROLE;
  }

  static constexpr int COLUMN_COUNT = INVALID_BUTT_ROLE - DEF_BEGIN_ROLE;
  static constexpr const char* HOR_HEADER_TITLES[COLUMN_COUNT]{"Type", "Name", "Account", "Pwd", "Others", "IsModified", "IsDeleted"};
  static constexpr int SORT_COLUMN = 0;
  static constexpr Role DEF_SORT_ROLE = TYPE_STR_ROLE;
  static constexpr int DATA_TYPE_ROLE = Role::INVALID_BUTT_ROLE + 1;
};

#endif
