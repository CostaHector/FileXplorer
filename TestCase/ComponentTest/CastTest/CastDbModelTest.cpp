#include <QtTest/QtTest>
#include <QSqlRecord>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "CastDbModel.h"
#include "FdBasedDb.h"
#include "EndToExposePrivateMember.h"
#include "CastBaseDb.h"

#include "PublicStringMacro.h"

#include "CastPsonFileHelper.h"
#include "JsonHelper.h"
#include "PublicVariable.h"
#include "TDir.h"

#include "SqlRecordTestHelper.h"
#include "SqlTableTestPreconditionTool.h"

using namespace SqlTableTestPreconditionTool;

class CastDbModelTest : public PlainTestSuite {
  Q_OBJECT
public:
private:
  TDir mTDir;

private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());

    const QList<FsNodeEntry> nodes
        {
         {"MigrateOk", true, ""},
         {"data_retrieve", true, ""},
         {"set_data", true, ""},
         {"refresh_fields", true, ""},
         {"insert_delete_row", true, ""},
         {"dump_records", true, ""},
         {"submit_revert", true, ""},
         };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());
  }

  void Migrate_ok() {
    // 允许残留空文件夹
    using namespace CastDbModelErrorCode;

    // 修改 ActorName, 修改 Ori 字段的值， 将触发创建前置路径, 触发迁移
    QString imgHostFullPath{mTDir.itemPath("MigrateOk")};

    // 场景1： 新插入一行， ActorName"", Ori"", 修改 ActorName""->"newActorName", 由于 Ori 是空的, 不创建任何路径
    {
      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, ""/*Ori*/, ""/*oldActorName*/, "newActorName"/*newActorName*/), 0);
      QVERIFY(!mTDir.exists("MigrateOk//newActorName"));
    }

    // 场景2. 新插入一行， ActorName"", 修改 Ori""->"newOri"->"newNewOri"
    {
      QCOMPARE(CastDbModel::MigrateCastsOriFolder(imgHostFullPath, ""/*oldOri*/, ""/*newOri*/, ""/*actorName*/), 0);                              // 空值改空值

      QCOMPARE(CastDbModel::MigrateCastsOriFolder(imgHostFullPath, ""/*oldOri*/, "newOri"/*newOri*/, ""/*actorName*/), 0);
      QVERIFY(mTDir.exists("MigrateOk/newOri"));
      QCOMPARE(CastDbModel::MigrateCastsOriFolder(imgHostFullPath, "newOri", "newNewOri", ""), 0);
      QVERIFY(mTDir.exists("MigrateOk/newNewOri"));

      QCOMPARE(CastDbModel::MigrateCastsOriFolder(imgHostFullPath, "newNewOri", "newNewOri", ""), 0);                                             // 相等
      QCOMPARE(CastDbModel::MigrateCastsOriFolder(imgHostFullPath, "newNewOri", "**/\\", ""), MigrateOriErrorCode::ORI_CONTAINS_INVALID_CHAR);    // 非法字符
      QCOMPARE(CastDbModel::MigrateCastsOriFolder(imgHostFullPath, "newNewOri", "", ""), MigrateOriErrorCode::ORI_EMPTY);                         // 不可以从非空值删空
    }

    // 场景3. Ori值非空 "newNewOri"， 修改 ActorName， ""->"newActorName"->"MichaelFassbender"
    // imgHostFullPath/newNewOri已经存在, 进行迁移
    {
      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, "newNewOri", "", ""), 0);                      // 无变更
      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, "newNewOri", "", "**/\\"), MigrateCastNameErrorCode::NEW_ACTOR_NAME_CONTAINS_INVALID_CHAR); // 非法字符

      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, "newNewOri", "", "newActorName"), 0);          // 创建成功， 但是没有需要迁移的
      QVERIFY(mTDir.exists("MigrateOk/newNewOri/newActorName"));
      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, "newNewOri", "newActorName", ""), MigrateCastNameErrorCode::NEW_ACTOR_NAME_EMPTY);                 // 不可以从非空值删空

      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, "newNewOri", "newActorName", "newNewActorName"), 1); // 迁移成功， 只有actor folder迁移
      QVERIFY(!mTDir.exists("MigrateOk/newNewOri/newActorName"));
      QVERIFY(mTDir.exists("MigrateOk/newNewOri/newNewActorName"));

      // 只要文件名startsWith(ActorName)就会改名
      const QList<FsNodeEntry> nodes
          {
           {"MigrateOk/newNewOri/newNewActorName/newNewActorName.mp4", false, "mp4"},
           {"MigrateOk/newNewOri/newNewActorName/newNewActorName.json", false, "json contents"},
           {"MigrateOk/newNewOri/newNewActorName/newNewActorName.pson", false, "pson contents"},
           {"MigrateOk/newNewOri/newNewActorName/newNewActorName.jpg", false, "jpg here"},
           {"MigrateOk/newNewOri/newNewActorName/newNewActorName 0.jpg", false, "0.jpg here"},
           {"MigrateOk/newNewOri/newNewActorName/newNewActorName 0 100x200.jpg", false, "0 100x200.jpg"},
           {"MigrateOk/newNewOri/newNewActorName/not start with folder 0.jpg", false, "not start with folder 0.jpg"},
           };
      QCOMPARE(mTDir.createEntries(nodes), nodes.size());
      const int migrateItemsCnt{nodes.size() + 1 - 1}; // 包含文件夹, 但是最后一个文件不会重命名
      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, "newNewOri", "newNewActorName", "MichaelFassbender"), migrateItemsCnt); // 迁移成功， 只有actor folder迁移
      QVERIFY(!mTDir.exists("MigrateOk/newNewOri/newActorName")); // 已经不存在了
      QVERIFY(mTDir.exists("MigrateOk/newNewOri/MichaelFassbender"));
      const QSet<QString> michaelFassbenderFiles {
          "MichaelFassbender.mp4",
          "MichaelFassbender.json",
          "MichaelFassbender.pson",
          "MichaelFassbender.jpg",
          "MichaelFassbender 0.jpg",
          "MichaelFassbender 0 100x200.jpg",
          "not start with folder 0.jpg"
      };
      QCOMPARE(TDir::SnapshotAtPath(mTDir.itemPath("MigrateOk/newNewOri/MichaelFassbender")), michaelFassbenderFiles);
    }

    // 场景3: 假设表内的Ori/Actor是过时的, 但是有 目的Ori/目的Actor (newNewOri, MichaelFassbender)存在
    // 场景4: 假设表内的Ori/Actor是过时的, 但是有 目的Ori/目的Actor 不存在
    {
      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, "newNewOri", "InexistActorName", "MichaelFassbender"), 0);
      QCOMPARE(CastDbModel::MigrateCastNameFolder(imgHostFullPath, "newNewOri", "InexistActorName", "StillInexistMichaelFassbender"), MigrateCastNameErrorCode::BOTH_OLD_ACTOR_AND_NEW_ACTOR_FOLDERS_INEXIST);

      QCOMPARE(CastDbModel::MigrateCastsOriFolder(imgHostFullPath, "InexistOri", "newNewOri", "MichaelFassbender"), 0);
      QCOMPARE(CastDbModel::MigrateCastsOriFolder(imgHostFullPath, "InexistOri", "StillInexistNewNewOri", "MichaelFassbender"), MigrateOriErrorCode::BOTH_OLD_ORI_ACTOR_AND_NEW_ORI_ACTOR_FOLDERS_INEXIST);
    }
  }


  void default_initialized_ok() {
    CastDbModel castModel{"Path/to/inexists/path"};
    { // precondition
      QVERIFY(!QFileInfo("Path/to/inexists/path").isDir());
      QVERIFY(!castModel.database().isValid());
      QVERIFY(!castModel.database().isOpen());
      QVERIFY(!CastDbModel::isDbValidAndOpened(castModel.database()));
    }

    { // basic setting and property
      QCOMPARE(castModel.editStrategy(), QSqlTableModel::EditStrategy::OnManualSubmit);
      QCOMPARE(castModel.m_imageHostPath, "Path/to/inexists/path");
      QCOMPARE(castModel.rootPath(), "Path/to/inexists/path");
      QCOMPARE(castModel.rowCount(), 0);
      QCOMPARE(castModel.columnCount(), 0);
      QVERIFY(castModel.GetAllRowsIndexes().isEmpty());
      QVERIFY(!castModel.isDirty());
    }

    { // data retrieve
      QModelIndex invalidIndex;
      QVERIFY(!invalidIndex.isValid());
      QVERIFY(castModel.data(invalidIndex, Qt::ItemDataRole::DisplayRole).isNull());
      QVERIFY(!castModel.setData(invalidIndex, "new value", Qt::ItemDataRole::EditRole));

      QCOMPARE(castModel.rootPath(), "Path/to/inexists/path");

      QCOMPARE(castModel.fileName(invalidIndex), "");
      QCOMPARE(castModel.filePath(invalidIndex), "");
      QCOMPARE(castModel.oriPath(invalidIndex), "");
      QCOMPARE(castModel.psonFilePath(invalidIndex), "");
      QCOMPARE(castModel.foldersRelativePath2RootPath({}), (QStringList{}));
      QCOMPARE(castModel.oriAtIndex(invalidIndex), "");
      QCOMPARE(castModel.tagsAtIndex(invalidIndex), "");
      QCOMPARE(castModel.rateAtIndex(invalidIndex), -1);
      QCOMPARE(castModel.sizeAtIndex(invalidIndex), -1);
      QCOMPARE(castModel.heightAtIndex(invalidIndex), -1);
      QCOMPARE(castModel.coverImageFullPathAtIndex(invalidIndex, true), "");
      QCOMPARE(castModel.coverImageFullPathAtIndex(invalidIndex, false), "");

    }

    { // batch modification
      QModelIndexList emptyIndexes;
      QItemSelection emptySelectionList;
      QCOMPARE(castModel.RefreshImgsFieldValue(emptyIndexes), 0);
      QCOMPARE(castModel.DumpRecordsIntoPsonFile(emptyIndexes), 0);
      QCOMPARE(castModel.DeleteSelectionRange(emptySelectionList), 0);
      QSqlDatabase emptyMovieDb;
      QCOMPARE(castModel.RefreshVidsFieldValue(emptyIndexes, emptyMovieDb), 0);
    }

    { // property
      QVERIFY(!castModel.isDirty());
      QVERIFY(castModel.submitSaveAllChanges());
    }
    { // call me in the view, not in the model itself
      QVERIFY(!castModel.revertAllChanges()); // no need
    }
  }


  void data_retrieve_ok() {
    const QString dbName{mTDir.itemPath("data_retrieve/CaseDbModelTest.db")};
    const QString connName{"CaseDbModelTestConn"};

    const QString imageHostPath{mTDir.itemPath("data_retrieve")};
    const QString tableName{DB_TABLE::PERFORMERS};

    CastBaseDb castDb{dbName, connName};
    {
      QVERIFY(castDb.IsValid());
      // database exist but no table created
      CastDbModel castModelNoTable{imageHostPath, nullptr, castDb.GetDb()};
      QVERIFY(castModelNoTable.database().isValid());
      QVERIFY(castModelNoTable.database().isOpen());
      QCOMPARE(castModelNoTable.tableName(), "");
      QCOMPARE(castModelNoTable.rowCount(), 0);
      QCOMPARE(castModelNoTable.rootPath(), imageHostPath);
    }

    // create table exist
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(castDb.IsTableExist(tableName));
    QVERIFY(castDb.IsTableEmpty(tableName)); // empty table

    CastDbModel castModel(imageHostPath, nullptr, castDb.GetDb());
    {
      QVERIFY(castModel.database().isValid());
      QVERIFY(castModel.database().isOpen());
      QCOMPARE(castModel.tableName(), tableName);
      QCOMPARE(castModel.rowCount(), 0);
      QCOMPARE(castModel.rootPath(), imageHostPath);
    }

    // prepare folder precondition
    QVERIFY(CreateFileStructure(mTDir, "data_retrieve/"));
    QCOMPARE(castDb.ReadFromImageHost(imageHostPath), 5); // 5 cast in total, insert all items into tables

    QCOMPARE(castModel.rowCount(), 0);
    QVERIFY(castModel.select()); // aka select
    QCOMPARE(castModel.rowCount(), 5);
    QCOMPARE(castModel.GetAllRowsIndexes().size(), 5);

    using namespace CastDbModelField;


    QSignalSpy headerDataChangedSpy{&castModel, &SqlTableModelPub::headerDataChanged};
    QVERIFY(headerDataChangedSpy.isValid());
    castModel.sort(CastDbModelField::Name, Qt::SortOrder::DescendingOrder);
    castModel.sort(CastDbModelField::Name, Qt::SortOrder::AscendingOrder);
    QCOMPARE(headerDataChangedSpy.count(), 0);

    QCOMPARE(JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V, 0);

    QModelIndex firstLineInd{castModel.index(0, 0)};
    QCOMPARE(castModel.fileName(firstLineInd), "Chris Evans");
    QCOMPARE(castModel.filePath(firstLineInd), mTDir.itemPath("data_retrieve/SuperHero/Chris Evans"));
    QCOMPARE(castModel.fileInfo(firstLineInd), (QFileInfo{mTDir.itemPath("data_retrieve/SuperHero/Chris Evans")}));
    QCOMPARE(castModel.oriPath(firstLineInd), mTDir.itemPath("data_retrieve/SuperHero"));
    QCOMPARE(castModel.psonFilePath(firstLineInd), mTDir.itemPath("data_retrieve/SuperHero/Chris Evans/Chris Evans.pson"));
    QCOMPARE(castModel.foldersRelativePath2RootPath({firstLineInd}), (QStringList{"SuperHero/Chris Evans"}));
    QCOMPARE(castModel.oriAtIndex(firstLineInd), "SuperHero");
    QCOMPARE(castModel.tagsAtIndex(firstLineInd), ""); // no tags
    QCOMPARE(castModel.rateAtIndex(firstLineInd), 0);  // default value
    QCOMPARE(castModel.sizeAtIndex(firstLineInd), -1);
    QCOMPARE(castModel.heightAtIndex(firstLineInd), -1);
    QCOMPARE(castModel.coverImageFullPathAtIndex(firstLineInd, true), mTDir.itemPath("data_retrieve/SuperHero/Chris Evans/Chris Evans_tn.jpg"));  // not check Imgs Field Value
    QCOMPARE(castModel.coverImageFullPathAtIndex(firstLineInd, false), mTDir.itemPath("data_retrieve/SuperHero/Chris Evans/Chris Evans 1.jpg")); // check Imgs Field value
    {
      QModelIndexList indexesNames{GetIndexessAtOneRow(castModel, 0, 5, CastDbModelField::Name)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesNames, QVariantList{"Chris Evans",
                                                                                       "Chris Hemsworth",
                                                                                       "Chris Pine",
                                                                                       "Cristiano Ronaldo",
                                                                                       "Michael Fassbender"}));

      QModelIndexList indexesOris{GetIndexessAtOneRow(castModel, 0, 5, CastDbModelField::Ori)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesOris, QVariantList{"SuperHero",
                                                                                      "SuperHero",
                                                                                      "SuperHero",
                                                                                      "X-MEN",
                                                                                      "Football"}));


      // Rate DecorationRole correct
      const QVariant pixmapVar = castModel.data(castModel.index(0, CastDbModelField::Rate), Qt::DecorationRole);
      QVERIFY(pixmapVar.isValid());
      QVERIFY(pixmapVar.canConvert<QPixmap>());
      const QPixmap pixmapVarRate = pixmapVar.value<QPixmap>();
      QVERIFY(!pixmapVarRate.isNull());
    }

    {
      // flags ok
      QVERIFY(castModel.flags(castModel.index(0, CastDbModelField::Rate)).testFlag(Qt::ItemIsEditable));
      QVERIFY(castModel.flags(castModel.index(0, CastDbModelField::Rate)).testFlag(Qt::ItemIsSelectable));
      QVERIFY(castModel.flags(castModel.index(0, CastDbModelField::Rate)).testFlag(Qt::ItemIsEnabled));
      QCOMPARE(castModel.fieldIndex(ENUM_2_STR(Rate)), CastDbModelField::Rate);
    }

    { // after db closed
      QVERIFY(castModel.setData(castModel.index(0, CastDbModelField::Name), "ChrisEvans.jpg", Qt::EditRole));
      QVERIFY(castModel.isDirty());
      castModel.database().close();
      QVERIFY(!CastDbModel::isDbValidAndOpened(castModel.database()));
      QVERIFY(!castModel.submitSaveAllChanges());
    }
  }

  void set_data_ok() {
    const QString dbName{mTDir.itemPath("set_data/CaseDbModelTest.db")};
    const QString connName{"CaseDbModelTestConn_setData"};

    const QString imageHostPath{mTDir.itemPath("set_data")};
    const QString tableName{DB_TABLE::PERFORMERS};

    CastBaseDb castDb{dbName, connName};
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(castDb.IsTableExist(tableName));
    QVERIFY(CreateFileStructure(mTDir, "set_data/"));
    QCOMPARE(castDb.ReadFromImageHost(imageHostPath), 5); // 5 cast in total, insert all items into tables

    CastDbModel castModel(imageHostPath, nullptr, castDb.GetDb());
    QVERIFY(!castModel.isDirty());
    QCOMPARE(castModel.rowCount(), 5);
    castModel.sort(CastDbModelField::Name, Qt::SortOrder::AscendingOrder);

    { // Rate ok
      const QModelIndexList indexesRates{GetIndexessAtOneRow(castModel, 0, 5, CastDbModelField::Rate)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(castModel, indexesRates, QVariantList{0, 0, 0, 0, 0}, Qt::DisplayRole));

      QVERIFY(!castModel.isDirty());
      QSignalSpy dirtyStatesChangedSpy{&castModel, &SqlTableModelPub::dirtyStatesChanged};
      QVERIFY(dirtyStatesChangedSpy.isValid());
      QVERIFY(castModel.setData(castModel.index(0, CastDbModelField::Rate), 0, Qt::EditRole)); // already 0, will return true. but not dirty
      QVERIFY(!castModel.isDirty());
      QCOMPARE(dirtyStatesChangedSpy.count(), 0);

      // JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V-> 5 save -> 9 revert -> 5
      {
        const QModelIndex line4RateIndex{castModel.index(4, CastDbModelField::Rate)};
        QCOMPARE(line4RateIndex.data(Qt::DisplayRole).toInt(), JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V);

        QVERIFY(castModel.setData(line4RateIndex, 5, Qt::EditRole));  // rate: 5
        QCOMPARE(line4RateIndex.data(Qt::DisplayRole).toInt(), 5);
        QCOMPARE(castModel.rateAtIndex(line4RateIndex), 5);
        QVERIFY(castModel.isDirty());
        QCOMPARE(dirtyStatesChangedSpy.count(), 1);
        dirtyStatesChangedSpy.clear();
        QVERIFY(castModel.submitSaveAllChanges());
        QVERIFY(!castModel.isDirty());

        QVERIFY(castModel.setData(line4RateIndex, 9, Qt::EditRole)); // rate: 9
        QCOMPARE(line4RateIndex.data(Qt::DisplayRole).toInt(), 9);
        QCOMPARE(castModel.rateAtIndex(line4RateIndex), 9);
        QVERIFY(castModel.isDirty());
        QVERIFY(castModel.revertAllChanges()); // revertAllChanges ok
        QVERIFY(!castModel.isDirty());

        QCOMPARE(line4RateIndex.data(Qt::DisplayRole).toInt(), 5);
        QCOMPARE(castModel.rateAtIndex(line4RateIndex), 5);
      }
    }

// {rel2FileEndWithSlash + "SuperHero/Chris Evans/Chris Evans 1.jpg", false, "Chris Evans 1"},
// {rel2FileEndWithSlash + "SuperHero/Chris Evans/Chris Evans 2.jpg", false, "Chris Evans 2"},
// {rel2FileEndWithSlash + "SuperHero/Chris Hemsworth/Chris Hemsworth.jpg", false, "Chris Hemsworth"},
// {rel2FileEndWithSlash + "SuperHero/Chris Pine/Chris Pine.jpg", false, "Chris Pine"},
// {rel2FileEndWithSlash + "X-MEN/Michael Fassbender/Michael Fassbender.jpg", false, "Michael Fassbender"},
// {rel2FileEndWithSlash + "Football/Cristiano Ronaldo/Cristiano Ronaldo.jpg", false, "Cristiano Ronaldo"},

    // 测试改名 Chris Evans->XChris Evans->Chris Evans, 预期保存才会触发重新排序
    castModel.sort(CastDbModelField::Name, Qt::SortOrder::AscendingOrder);
    {
      {
        const QModelIndex line0ChrisEvansNameIndex{castModel.index(0, CastDbModelField::Name)};
        QCOMPARE(line0ChrisEvansNameIndex.data(Qt::DisplayRole).toString(), "Chris Evans");

        QCOMPARE(castModel.setData(line0ChrisEvansNameIndex, "XChris Evans", Qt::EditRole), true);
        QCOMPARE(line0ChrisEvansNameIndex.data(Qt::DisplayRole).toString(), "XChris Evans"); // 提交修改时才会触发重新排序
        QVERIFY(castModel.isDirty());
        QVERIFY(castModel.submitSaveAllChanges());
        QVERIFY(!castModel.isDirty());
      }
      //  排序后XChris Evans 在4行
      {
        const QModelIndex line4ChrisEvansNameIndex{castModel.index(4, CastDbModelField::Name)}; // line0ChrisEvansNameIndex已失效
        QCOMPARE(line4ChrisEvansNameIndex.data(Qt::DisplayRole).toString(), "XChris Evans");

        QVERIFY(castModel.setData(line4ChrisEvansNameIndex, "Chris Evans", Qt::EditRole)); // 改回原来的名字
        QCOMPARE(line4ChrisEvansNameIndex.data(Qt::DisplayRole).toString(), "Chris Evans"); // 提交修改时才会触发重新排序
        QVERIFY(castModel.isDirty());
        QVERIFY(castModel.submitSaveAllChanges());
        QVERIFY(!castModel.isDirty());
      }

      QCOMPARE(castModel.index(0, CastDbModelField::Name).data(Qt::DisplayRole).toString(), "Chris Evans");
    }

    // 测试改Ori Football->ZFootball->Football, 预期保存才会触发重新排序
    castModel.sort(CastDbModelField::Ori, Qt::SortOrder::AscendingOrder); // 排序后Football 在0行
    {
      {
        const QModelIndex line0FootballOriIndex{castModel.index(0, CastDbModelField::Ori)};
        QCOMPARE(line0FootballOriIndex.data(Qt::DisplayRole).toString(), "Football");

        QCOMPARE(castModel.setData(line0FootballOriIndex, "XXXFootball", Qt::EditRole), true);
        QCOMPARE(line0FootballOriIndex.data(Qt::DisplayRole).toString(), "XXXFootball"); // 提交修改时才会触发重新排序
        QVERIFY(castModel.isDirty());
        QVERIFY(castModel.submitSaveAllChanges());
        QVERIFY(!castModel.isDirty());
      }
      //  排序后XFootball 在4行
      {
        const QModelIndex line4FootballOriIndex{castModel.index(4, CastDbModelField::Ori)};
        QCOMPARE(line4FootballOriIndex.data(Qt::DisplayRole).toString(), "XXXFootball");

        QVERIFY(castModel.setData(line4FootballOriIndex, "Football", Qt::EditRole));   // 改回原来的名字
        QCOMPARE(line4FootballOriIndex.data(Qt::DisplayRole).toString(), "Football"); // 提交修改时才会触发重新排序
        QVERIFY(castModel.isDirty());
        QVERIFY(castModel.submitSaveAllChanges());
        QVERIFY(!castModel.isDirty());
      }
      
      QCOMPARE(castModel.index(0, CastDbModelField::Ori).data(Qt::DisplayRole).toString(), "Football");
    }
  }

  void refresh_fields_ok() {
    const QString dbName{mTDir.itemPath("refresh_fields/CaseDbModelTest.db")};
    const QString connName{"CaseDbModelTestConn_refresh_fields"};

    const QString imageHostPath{mTDir.itemPath("refresh_fields")};
    const QString tableName{DB_TABLE::PERFORMERS};

    CastBaseDb castDb{dbName, connName};
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(castDb.IsTableExist(tableName));
    QVERIFY(CreateFileStructure(mTDir, "refresh_fields/"));
    QCOMPARE(castDb.ReadFromImageHost(imageHostPath), 5); // 5 cast in total, insert all items into tables

    CastDbModel castModel(imageHostPath, nullptr, castDb.GetDb());
    QVERIFY(!castModel.isDirty());
    QCOMPARE(castModel.rowCount(), 5);
    castModel.sort(CastDbModelField::Name, Qt::SortOrder::AscendingOrder);

// {rel2FileEndWithSlash + "SuperHero/Chris Evans/Chris Evans 1.jpg", false, "Chris Evans 1"},
// {rel2FileEndWithSlash + "SuperHero/Chris Evans/Chris Evans 2.jpg", false, "Chris Evans 2"},
// {rel2FileEndWithSlash + "SuperHero/Chris Hemsworth/Chris Hemsworth.jpg", false, "Chris Hemsworth"},
// {rel2FileEndWithSlash + "SuperHero/Chris Pine/Chris Pine.jpg", false, "Chris Pine"},
// {rel2FileEndWithSlash + "X-MEN/Michael Fassbender/Michael Fassbender.jpg", false, "Michael Fassbender"},
// {rel2FileEndWithSlash + "Football/Cristiano Ronaldo/Cristiano Ronaldo.jpg", false, "Cristiano Ronaldo"},
    
    {
      // RefreshImgsFieldValue ok
      QModelIndex chrisEvansIndex{castModel.index(0, CastDbModelField::Name)};
      QVERIFY(castModel.setData(chrisEvansIndex.siblingAtColumn(CastDbModelField::Imgs), ""));
      QCOMPARE(chrisEvansIndex.siblingAtColumn(CastDbModelField::Imgs).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(castModel.RefreshImgsFieldValue({}), 0);
      QCOMPARE(castModel.RefreshImgsFieldValue({chrisEvansIndex}), 1); // 1/5 row Imgs field affected
      QCOMPARE(castModel.RefreshImgsFieldValue({chrisEvansIndex}), 0); // 0/5 row Imgs field no change
      QCOMPARE(chrisEvansIndex.siblingAtColumn(CastDbModelField::Imgs).data(Qt::DisplayRole).toString(), "Chris Evans 1.jpg\nChris Evans 2.jpg");

      QCOMPARE(castModel.coverImageFullPathAtIndex(chrisEvansIndex, true), mTDir.itemPath("refresh_fields/SuperHero/Chris Evans/Chris Evans_tn.jpg")); // yes, use thumbnail
      QCOMPARE(castModel.coverImageFullPathAtIndex(chrisEvansIndex, false), mTDir.itemPath("refresh_fields/SuperHero/Chris Evans/Chris Evans 1.jpg")); // no, use Imgs field value first one
    }

    {
      // RefreshVidsFieldValue ok

      const QString movieDbName{mTDir.itemPath("refresh_fields/MovieCaseDbModelTest.db")}; // create database and table here
      const QString movieConnName{"MovieCaseDbModelTestConn_refresh_fields"};
      const QString movieTableName{DB_TABLE::MOVIES};

      FdBasedDb movieDb{movieDbName, movieConnName};
      {
        QVERIFY(movieDb.IsValid());
        QVERIFY(movieDb.CreateTable(movieTableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
        QVERIFY(movieDb.IsTableExist(movieTableName));
        QVERIFY(movieDb.IsTableEmpty(movieTableName));
      }
      
      QModelIndex chrisEvansIndex{castModel.index(0, CastDbModelField::Name)};
      QModelIndex chrisHemsworthIndex{castModel.index(1, CastDbModelField::Name)};
      QCOMPARE(chrisEvansIndex.siblingAtColumn(CastDbModelField::Vids).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(chrisHemsworthIndex.siblingAtColumn(CastDbModelField::Vids).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(castModel.RefreshVidsFieldValue({chrisEvansIndex, chrisHemsworthIndex}, movieDb.GetDb()), 0); // no records in table at all. no update
      QCOMPARE(chrisEvansIndex.siblingAtColumn(CastDbModelField::Vids).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(chrisHemsworthIndex.siblingAtColumn(CastDbModelField::Vids).data(Qt::DisplayRole).toString(), "");

      {
        QSet<QByteArray> needInsertFds = {"1001", "1002", "1003"};
        QHash<QByteArray, QString> newFd2Pth = {
                                                // 3 records, 2 of Chris Evans and 1 of Chris Hemsworth
                                                {"1001", "videos/superhero/Chris Evans 1.mp4"},
                                                {"1002", "videos/superhero/Chris Evans Captain America.mp4"},
                                                {"1003", "videos/superhero/Chris Hemsworth.mp4"},
                                                };
        // 调用Insert函数插入记录
        int insertCnt = 0;
        FD_ERROR_CODE errorCode = movieDb.Insert(movieTableName, needInsertFds, newFd2Pth, insertCnt);
        QVERIFY(errorCode == FD_OK);
        QCOMPARE(insertCnt, 3);

        QCOMPARE(movieDb.CountRow(movieTableName, ""), 3);
      }

      QCOMPARE(castModel.RefreshVidsFieldValue({chrisEvansIndex, chrisHemsworthIndex}, movieDb.GetDb()), 2); // affected rows count = 2. and sorted
      QCOMPARE(castModel.RefreshVidsFieldValue({chrisEvansIndex, chrisHemsworthIndex}, movieDb.GetDb()), 0); // unchanged. no row affected
      QCOMPARE(chrisEvansIndex.siblingAtColumn(CastDbModelField::Vids).data(Qt::DisplayRole).toString(), "videos/superhero/Chris Evans 1.mp4\n"
                                                                                                         "videos/superhero/Chris Evans Captain America.mp4");
      QCOMPARE(chrisHemsworthIndex.siblingAtColumn(CastDbModelField::Vids).data(Qt::DisplayRole).toString(), "videos/superhero/Chris Hemsworth.mp4");
    }    
  }

  void insert_delete_row_ok() {
    const QString dbName{mTDir.itemPath("insert_delete_row/CaseDbModelTest.db")};
    const QString connName{"CaseDbModelTestConn_insert_delete_row"};

    const QString imageHostPath{mTDir.itemPath("insert_delete_row")};
    const QString tableName{DB_TABLE::PERFORMERS};

    CastBaseDb castDb{dbName, connName};
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(castDb.IsTableExist(tableName));

    CastDbModel castModel(imageHostPath, nullptr, castDb.GetDb());
    QVERIFY(!castModel.isDirty());
    QCOMPARE(castModel.rowCount(), 0);
    castModel.sort(CastDbModelField::Name, Qt::SortOrder::AscendingOrder);

    castModel.insertRow(0);
    QCOMPARE(castModel.rowCount(), 1);
    QCOMPARE(castModel.index(0, CastDbModelField::Name).data(Qt::DisplayRole).toString(), "");

    QVERIFY(castModel.isDirty());
    QVERIFY(!castModel.submitAll()); // primary key restrict: 'Name' must not null
    QVERIFY(castModel.isDirty()); // 没有启用transaction, 不需要rollback

    QVERIFY(castModel.setData(castModel.index(0, CastDbModelField::Name), "Kaka", Qt::EditRole));
    QVERIFY(castModel.setData(castModel.index(0, CastDbModelField::Name), "Football", Qt::EditRole));
    QVERIFY(castModel.submitAll()); // succeed now
    QVERIFY(!castModel.isDirty());

    QItemSelection selectionRangeList{castModel.index(0, CastDbModelField::Name), castModel.index(0, CastDbModelField::Detail)};
    QCOMPARE(castModel.DeleteSelectionRange(selectionRangeList), 1); // 删除时只是标记， 不是真正删除
    QCOMPARE(castModel.rowCount(), 1);
    QCOMPARE(castModel.headerData(0, Qt::Orientation::Vertical, Qt::DisplayRole).toString(), "!"); // 预期标记为header中的!号
    QVERIFY(castModel.isDirty());
    QVERIFY(castModel.submitAll());

    QCOMPARE(castModel.rowCount(), 0);
    QVERIFY(!castModel.isDirty());
  }

  void dump_records_ok() {
    const QString dbName{mTDir.itemPath("dump_records/CaseDbModelTest.db")};
    const QString connName{"CaseDbModelTestConn_dump_records"};

    const QString imageHostPath{mTDir.itemPath("dump_records")};
    const QString tableName{DB_TABLE::PERFORMERS};

    CastBaseDb castDb{dbName, connName};
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(castDb.IsTableExist(tableName));
    // 直接调用castDb的接口创建一条记录。Name/Ori 必须都指定好, 否则导出时算不出 pson 文件所在的前置路径
    const QString& insertCmd{QString{"INSERT INTO `%1` (`Name`, `Ori`, `Rate`, `Tags`, `Height`, `Imgs`) "
                                     "VALUES ('Chris Evans', 'SuperHero', 5, 'hero,movie star', 180, 'Chris Evans 1.jpg')"}
                                 .arg(tableName)};
    QCOMPARE(castDb.UpdateForTest(insertCmd), 1);

    CastDbModel castModel(imageHostPath, nullptr, castDb.GetDb());
    QCOMPARE(castModel.rowCount(), 1);

    QModelIndex ind = castModel.index(0, CastDbModelField::Name);

    const QString psonPath{castModel.psonFilePath(ind)};
    QCOMPARE(psonPath, mTDir.itemPath("dump_records/SuperHero/Chris Evans/Chris Evans.pson"));

    // 导出前不存在； 导出时会自动创建缺失的前置目录
    QVERIFY(!QFileInfo::exists(psonPath));
    QCOMPARE(castModel.DumpRecordsIntoPsonFile({ind}), 1);
    QVERIFY(QFileInfo::exists(psonPath));
    QCOMPARE(castModel.DumpRecordsIntoPsonFile({ind}), 0); // 二次调用时，pson文件内容已经和dump内容一致， 无需

    // 直接用JsonHelper::MovieJsonLoader检查pson文件内容， 只要一致就ok了
    const QVariantHash actualPson{JsonHelper::MovieJsonLoader(psonPath)};
    using namespace CastDbModelField;
    QVERIFY(actualPson.count() >= 6); // at least 6 key
    QCOMPARE(actualPson.value(ENUM_2_STR(Name)).toString(), "Chris Evans");
    QCOMPARE(actualPson.value(ENUM_2_STR(Ori)).toString(), "SuperHero");
    QCOMPARE(actualPson.value(ENUM_2_STR(Rate)).toInt(), 5);
    QCOMPARE(actualPson.value(ENUM_2_STR(Tags)).toString(), "hero,movie star");
    QCOMPARE(actualPson.value(ENUM_2_STR(Height)).toInt(), 180);
    QCOMPARE(actualPson.value(ENUM_2_STR(Imgs)).toString(), "Chris Evans 1.jpg");
  }

  void submit_revert_ok() {
    const QString dbName{mTDir.itemPath("submit_revert/CaseDbModelTest.db")};
    const QString connName{"CaseDbModelTestConn_submit_revert"};

    const QString imageHostPath{mTDir.itemPath("submit_revert")};
    const QString tableName{DB_TABLE::PERFORMERS};

    CastBaseDb castDb{dbName, connName};
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    QVERIFY(castDb.IsTableExist(tableName));

    CastDbModel castModel(imageHostPath, nullptr, castDb.GetDb());
    QVERIFY(!castModel.isDirty());
    QCOMPARE(castModel.rowCount(), 0);

    { // 没有改动时, submit 直接跳过(返回true), revert 无事可做(返回false)
      QVERIFY(castModel.submitSaveAllChanges());
      QVERIFY(!castModel.revertAllChanges());
      QCOMPARE(castModel.rowCount(), 0);
    }

    { // 插入一条记录并提交。 Name/Ori 必须都填, 否则 submit 会失败
      QSqlRecord chrisEvansRecord = SqlRecordTestHelper::GetACastRecordLine("Chris Evans", "SuperHero", ""/*No Imgs*/, ""/*No Vids*/);
      QVERIFY(castModel.insertRecord(0, chrisEvansRecord));
      QCOMPARE(castModel.rowCount(), 1);
      QVERIFY(castModel.isDirty());
      QCOMPARE(castModel.fileName(castModel.index(0, 0)), "Chris Evans");
      QCOMPARE(castModel.oriAtIndex(castModel.index(0, 0)), "SuperHero");
      QCOMPARE(castDb.CountRow(tableName, ""), 0); // 验证插入行chrisEvansRecord还未入库

      // 基类的sort排序会导致丢失暂存区, 这里验证 SqlTableModelPub::sort override 拦截下来， 避免丢数据
      castModel.sort(CastDbModelField::Name, Qt::AscendingOrder);
      QCOMPARE(castModel.rowCount(), 1);

      QVERIFY(castModel.submitSaveAllChanges());
      QVERIFY(!castModel.isDirty());
      QCOMPARE(castModel.rowCount(), 1);
      QCOMPARE(castDb.CountRow(tableName, ""), 1); // 验证插入行chrisEvansRecord已经入库
    }

    { // 未设置的字段落到建表时的DEFAULT值
      QCOMPARE(castModel.rateAtIndex(castModel.index(0, CastDbModelField::Height)), JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V);
    }

    { // 只改Height(不会触发文件迁移的字段), 提交后落库
      QVERIFY(castModel.setData(castModel.index(0, CastDbModelField::Height), 180, Qt::EditRole));
      QVERIFY(castModel.isDirty());
      QVERIFY(castModel.submitSaveAllChanges());
      QVERIFY(!castModel.isDirty());
      QCOMPARE(castModel.heightAtIndex(castModel.index(0, CastDbModelField::Height)), 180);
    }

    { // 再改一次 Height 180->190, 不提交, revert 回退到上次提交的值
      QVERIFY(castModel.setData(castModel.index(0, CastDbModelField::Height), 190, Qt::EditRole));
      QCOMPARE(castModel.heightAtIndex(castModel.index(0, CastDbModelField::Height)), 190);
      QVERIFY(castModel.isDirty());

      QVERIFY(castModel.revertAllChanges());
      QVERIFY(!castModel.isDirty());
      QCOMPARE(castModel.heightAtIndex(castModel.index(0, CastDbModelField::Height)), 180);
    }

    { // 多个字段同时改, revert 应当全部还原
      QVERIFY(castModel.setData(castModel.index(0, CastDbModelField::Height), 200, Qt::EditRole));
      QVERIFY(castModel.setData(castModel.index(0, CastDbModelField::Rate), 9, Qt::EditRole));
      QVERIFY(castModel.isDirty());

      QVERIFY(castModel.revertAllChanges());
      QVERIFY(!castModel.isDirty());
      QCOMPARE(castModel.heightAtIndex(castModel.index(0, CastDbModelField::Height)), 180);
      QCOMPARE(castModel.rateAtIndex(castModel.index(0, CastDbModelField::Rate)), JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V);
    }
  }
};

#include "CastDbModelTest.moc"
REGISTER_TEST(CastDbModelTest, false)
