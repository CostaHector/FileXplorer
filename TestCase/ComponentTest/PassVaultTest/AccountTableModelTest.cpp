#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "AccountTableModel.h"
#include "EndToExposePrivateMember.h"
#include "AccountSortFilterProxyModel.h"

#include "SimpleAES.h"
#include "TDir.h"

class AccountTableModelTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
  const QList<AccountInfo> mAccList //
      {
       AccountInfo{"E-mail", "Gmail", "MichaelFassbender@gmail.com", "123456", ""},
       AccountInfo{"Game", "Minecraft", "MichaelFassbender", "654321", "Include:\nJava Edition\nBedrock Edition"},
       };
  QString mEncCsvFileName{"test_accounts.enc.csv"};
  QString mAES_KEY{"0123456789012345"};
private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());  //
    QString plainCSV = AccountFileHelper::GetExportCSVRecords(mAccList);
    QString encryptedCSV;
    QVERIFY(SimpleAES(mAES_KEY).encrypt_GCM(plainCSV, encryptedCSV));
    QVERIFY(encryptedCSV.size() > 0);
    QVERIFY(mTDir.touch(mEncCsvFileName, encryptedCSV.toUtf8()));
    QVERIFY(mTDir.exists(mEncCsvFileName));
  }

  void defaultConstructor_ok() {
    // call following member function should not crash down
    AccountTableModel model{"Inexist/Path/To/InexistFile", "INCORECT_AES_KEY"};
    QVERIFY(!model.mAccountsList.IsValid());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), AccountInfo::COLUMN_COUNT);
    QCOMPARE(model.IsDecryptedOk(), false);

    QModelIndex invalidIndex;
    QVERIFY(!model.data(invalidIndex).isValid());

    /*"Index", "Type", "Name", "Account"*/
    QCOMPARE(model.headerData(AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE), Qt::Horizontal, Qt::DisplayRole).toString(), "Type");
    QCOMPARE(model.headerData(AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE), Qt::Horizontal, Qt::DisplayRole).toString(), "Name");
    QCOMPARE(model.headerData(AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE), Qt::Horizontal, Qt::DisplayRole).toString(), "Account");
    QCOMPARE(model.headerData(AccountInfo::Role2Column(AccountInfo::Role::PWD_STR_ROLE), Qt::Horizontal, Qt::DisplayRole).toString(), "Pwd");
    QCOMPARE(model.headerData(AccountInfo::Role2Column(AccountInfo::Role::OTHERS_STR_ROLE), Qt::Horizontal, Qt::DisplayRole).toString(), "Others");
    QCOMPARE(model.headerData(AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE), Qt::Horizontal, Qt::DisplayRole).toString(), "IsModified");
    QCOMPARE(model.headerData(AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE), Qt::Horizontal, Qt::DisplayRole).toString(), "IsDeleted");

    // section out of range display role, return value directly
    QCOMPARE(model.headerData(-4, Qt::Horizontal, Qt::DisplayRole).toInt(), -4 + 1);
    QCOMPARE(model.headerData(99, Qt::Horizontal, Qt::DisplayRole).toInt(), 99 + 1);
    QCOMPARE(model.headerData(-4, Qt::Vertical, Qt::DisplayRole).toInt(), -4 + 1);
    QCOMPARE(model.headerData(99, Qt::Vertical, Qt::DisplayRole).toInt(), 99 + 1);

    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::TextAlignmentRole).toInt(), static_cast<int>(Qt::AlignRight));
    QVERIFY(model.headerData(0, Qt::Horizontal, Qt::ItemDataRole::ForegroundRole).isNull());

    // 因为前置路径不存在, 解析认为非法, IsValid()=false拒绝写入
    QVERIFY(!model.onSave());
    QVERIFY(!model.ExportToPlainCSVFile());
  }

  void encFileFullPath_AesKey_changed_ok() {
    AccountTableModel model{"Inexist/Path/To/InexistFile", "INCORECT_AES_KEY"};

    QSignalSpy decryptedResultChangedSpy{&model, &AccountTableModel::decryptedResultChanged};
    QVERIFY(decryptedResultChangedSpy.isValid());

    QCOMPARE(model.IsDecryptedOk(), false);
    // 允许初始化后主动切换路径和AesKey
    model.SetEncFileFullPath(mTDir.itemPath(mEncCsvFileName)); // 路径对, 但是aeskey不对
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.IsDecryptedOk(), false);
    QCOMPARE(decryptedResultChangedSpy.count(), 0); // false -> false

    model.SetAesKey(mAES_KEY);                                 // 同时对
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.IsDecryptedOk(), true);
    QCOMPARE(decryptedResultChangedSpy.count(), 1); // false -> true
    QVariantList decryptedAfterBool = decryptedResultChangedSpy.takeLast();
    QCOMPARE(decryptedAfterBool.size(), 1);
    QCOMPARE(decryptedAfterBool.front().toBool(), true);

    model.SetEncFileFullPathAndAesKey("Inexist/Path/To/InexistFile", "INCORECT_AES_KEY"); // 同时不对
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.IsDecryptedOk(), false);
    QCOMPARE(decryptedResultChangedSpy.count(), 1); // true -> false
    decryptedAfterBool = decryptedResultChangedSpy.takeLast();
    QCOMPARE(decryptedAfterBool.front().toBool(), false);

    model.SetEncFileFullPathAndAesKey(mTDir.itemPath(mEncCsvFileName), mAES_KEY); // 同时对
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.IsDecryptedOk(), true);
    QCOMPARE(decryptedResultChangedSpy.count(), 1); // false -> true
    decryptedAfterBool = decryptedResultChangedSpy.takeLast();
    QCOMPARE(decryptedAfterBool.front().toBool(), true);
  }

  void behavior_ok() {
    AccountTableModel model{mTDir.itemPath(mEncCsvFileName), mAES_KEY};
    QCOMPARE(model.rowCount(), 2);
    QVERIFY(model.mAccountsList.IsValid());

    // column: IS_MODIFIED_ROLE and IS_DELETED_ROLE cannot Edit
    QVERIFY(model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE))).testFlag(Qt::ItemFlag::ItemIsEnabled));
    QVERIFY(model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE))).testFlag(Qt::ItemFlag::ItemIsSelectable));
    QVERIFY(model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE))).testFlag(Qt::ItemFlag::ItemIsEditable));

    QVERIFY(model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE))).testFlag(Qt::ItemFlag::ItemIsEnabled));
    QVERIFY(model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE))).testFlag(Qt::ItemFlag::ItemIsSelectable));
    QVERIFY(!model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE))).testFlag(Qt::ItemFlag::ItemIsEditable));

    QVERIFY(model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE))).testFlag(Qt::ItemFlag::ItemIsEnabled));
    QVERIFY(model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE))).testFlag(Qt::ItemFlag::ItemIsSelectable));
    QVERIFY(!model.flags(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE))).testFlag(Qt::ItemFlag::ItemIsEditable));

    // 3. 测试数据访问
    QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE))).toString(), "E-mail");
    QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE))).toString(), "Gmail");
    QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE))).toString(), "MichaelFassbender@gmail.com");
    QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::PWD_STR_ROLE))).toString(), "123456");
    QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::OTHERS_STR_ROLE))).toString(), "");

    QCOMPARE(model.data(model.index(1, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE))).toString(), "Game");
    QCOMPARE(model.data(model.index(1, AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE))).toString(), "Minecraft");
    QCOMPARE(model.data(model.index(1, AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE))).toString(), "MichaelFassbender");
    QCOMPARE(model.data(model.index(1, AccountInfo::Role2Column(AccountInfo::Role::PWD_STR_ROLE))).toString(), "654321");
    QCOMPARE(model.data(model.index(1, AccountInfo::Role2Column(AccountInfo::Role::OTHERS_STR_ROLE))).toString(), "Include:\nJava Edition\nBedrock Edition");

    QSignalSpy dataChangedSpy{&model, &AccountTableModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    // 4. 测试数据修改 依次修改Type, Name, Account
    {
      dataChangedSpy.clear();
      {
        // 未变更, 认为已经修改好了, 不会标记以修改, 不会触发dataChanged
        QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE))).toBool(), false);
        QVERIFY(model.setData(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE)), "E-mail"));
        QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE))).toBool(), false);
        QCOMPARE(dataChangedSpy.count(), 0);
      }
      dataChangedSpy.clear();

      QVERIFY(model.setData(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE)), "Social Media")); // 标记已修改
      QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE))).toString(), "Social Media");
      QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE))).toBool(), true);

      QVERIFY(model.setData(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE)), "E-mail"));
      QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE))).toString(), "E-mail"); // 改回去
    }

    dataChangedSpy.clear();

    // 5. 测试标记删除行
    {
      QCOMPARE(model.MarkDeletedIndexes({}, true), 0);
      QCOMPARE(model.MarkDeletedIndexes({}, false), 0);

      QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE))).toBool(), false);
      QCOMPARE(model.data(model.index(1, AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE))).toBool(), false);

      QCOMPARE(dataChangedSpy.count(), 0);
      QModelIndex ind01 = model.index(0, AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE));
      QModelIndex ind11 = model.index(1, AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE));
      QModelIndexList allIndexes{ind01, ind11, ind01, QModelIndex{}}; // 含有重复的index和一个非法的index

      QCOMPARE(model.MarkDeletedIndexes(allIndexes, true), 2);  // there is no 12 rows at all
      QCOMPARE(model.rowCount(), 2); // row count unchange
      QCOMPARE(dataChangedSpy.count(), 1);
      const QVariantList actualParms = dataChangedSpy.takeLast();
      QCOMPARE(actualParms.size(), 3);
      QCOMPARE(actualParms[0], (QVariant{model.index(0, 0)}));
      QCOMPARE(actualParms[1], (QVariant{model.index(1, model.columnCount() - 1)}));

      QCOMPARE(model.data(model.index(0, AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE))).toBool(), true);
      QCOMPARE(model.data(model.index(1, AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE))).toBool(), true);

      dataChangedSpy.clear();
    }

    // 6. 测试插入/标记删除行
    {
      // acc0, acc1
      QCOMPARE(model.rowCount(), 2);
      QCOMPARE(model.InsertNRows(1, 0), 0); // 0 rows inserted
      QCOMPARE(model.rowCount(), 2);

      // empty, acc0, acc1
      QCOMPARE(model.InsertNRows(-1, 1), 1); // 1 rows inserted at first
      QCOMPARE(model.rowCount(), 3);

      // empty, acc0, acc1, empty
      QCOMPARE(model.InsertNRows(999, 1), 1); // 1 rows inserted at back
      QCOMPARE(model.rowCount(), 4);

      dataChangedSpy.clear();
      const QModelIndexList emptyElementIndexes {
          model.index(0, AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE)),
          model.index(3, AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE)),
      };
      QCOMPARE(model.MarkDeletedIndexes(emptyElementIndexes, false), 2); // row count unchange
      QCOMPARE(dataChangedSpy.count(), 2);
      dataChangedSpy.clear();
    }

    // empty, acc0, acc1, empty
    QCOMPARE(model.rowCount(), 4);

    // 7. 测试添加账户记录
    {
      QCOMPARE(model.AppendAccountRecords({}), 0);
      QCOMPARE(model.rowCount(), 4);

      // empty, acc0, acc1, empty, newAccounts
      QList<AccountInfo> newAccounts{{"Game", "Steam", "steam_user", "steam_pass", "Gaming account"}};
      QCOMPARE(model.AppendAccountRecords(newAccounts), 1);
      QCOMPARE(model.rowCount(), 5);

      QCOMPARE(model.data(model.index(4, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE))).toString(), "Game");
      QCOMPARE(model.data(model.index(4, AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE))).toString(), "Steam");
      QCOMPARE(model.data(model.index(4, AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE))).toString(), "steam_user");
    }

    {
      // 8. 保存成功后, 需要清掉isModified标志
      dataChangedSpy.clear();
      QCOMPARE(model.onSave(), true);
      QCOMPARE(dataChangedSpy.count(), 1);
    }

    // 9. 测试导出为明文CSV (在enc.csv在同一路径下)
    {
      QVERIFY(model.ExportToPlainCSVFile());
      QVERIFY(mTDir.exists("accounts_export.csv"));
    }

    // 10. 测试代理模型过滤
    {
      // empty, acc0, acc1, empty, newAccounts
      // empty
      // AccountInfo{"E-mail", "Gmail", "MichaelFassbender@gmail.com", "123456", ""}
      // AccountInfo{"Game", "Minecraft", "MichaelFassbender", "654321", "Include:\nJava Edition\nBedrock Edition"}
      // empty
      // AccountInfo{"Game", "Steam", "steam_user", "steam_pass", "Gaming account"}

      AccountSortFilterProxyModel proxyModel;
      proxyModel.setSourceModel(&model);

      // 设置过滤关键词
      proxyModel.setFilterFixedString("Minecraft");
      QCOMPARE(proxyModel.rowCount(), 1);  // 应只匹配一行

      QModelIndex proxyIndex = proxyModel.index(0, AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE));
      QCOMPARE(proxyModel.data(proxyIndex).toString(), "Minecraft");

      QModelIndex srcIndex = proxyModel.mapToSource(proxyIndex);
      QCOMPARE(model.data(srcIndex).toString(), "Minecraft");
    }

    {
      // 11. 测试行数据访问
      const AccountInfo* rowData1 = model.rowDataAt(1);   QVERIFY(rowData1 != nullptr);       QCOMPARE(rowData1->nameStr, "Gmail");
      const AccountInfo* rowData2 = model.rowDataAt(2);   QVERIFY(rowData2 != nullptr);       QCOMPARE(rowData2->nameStr, "Minecraft");
      const AccountInfo* rowData4 = model.rowDataAt(4);   QVERIFY(rowData4 != nullptr);       QCOMPARE(rowData4->nameStr, "Steam");

      AccountInfo* invalidRowData = model.rowDataAt(100); QVERIFY(invalidRowData == nullptr);
    }

    {
      // 12. 测试图标显示
      const QModelIndex iconIndex = model.index(0, AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE));
      QVariant iconVariant = model.data(iconIndex, Qt::DecorationRole);
    }
  }
};

#include "AccountTableModelTest.moc"
REGISTER_TEST(AccountTableModelTest, false)
