#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "FdBasedDb.h"
#include "FdBasedDbModel.h"
#include "EndToExposePrivateMember.h"
#include "TableFields.h"
#include "TDir.h"
#include "PathTool.h"

bool CheckIndexesDisplayRoleIgnoreOrder(const FdBasedDbModel& movieModel,  //
                                        const QModelIndexList& indexes,
                                        QStringList expectsList) {
  QStringList actualsList;
  for (const QModelIndex& ind : indexes) {
    actualsList.push_back(movieModel.data(ind, Qt::DisplayRole).toString());
  }
  std::sort(actualsList.begin(), actualsList.end());
  std::sort(expectsList.begin(), expectsList.end());
  int n1 = actualsList.size(), n2 = expectsList.size();
  if (actualsList == expectsList) {
    return true;
  }
  if (n1 != n2) {
    LOG_W("list length[%d, %d] not equal", n1, n2);
    return false;
  }

  qDebug() << "expects:" << expectsList;
  qDebug() << "actuals:" << actualsList;
  return false;
}

class FdBasedDbModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void default_constructor_ok() {
    FdBasedDbModel fdModel;
    // call member should not leading crash down

    QModelIndex invalidIndex;
    QVERIFY(fdModel.data(invalidIndex, Qt::ItemDataRole::DisplayRole).isNull());
    QVERIFY(fdModel.absolutePath(invalidIndex).isEmpty());
    QVERIFY(fdModel.fileName(invalidIndex).isEmpty());
    QVERIFY(fdModel.filePath(invalidIndex).isEmpty());
    QVERIFY(fdModel.fileInfo(invalidIndex).filePath().isEmpty());
    QVERIFY(fdModel.fullInfo(invalidIndex).isEmpty());

    QModelIndexList emptyList;
    fdModel.SetStudio(emptyList, "Test Studio");
    fdModel.SetCastOrTags(emptyList, "Test Cast");
    fdModel.AddCastOrTags(emptyList, "Test Cast");
    fdModel.RmvCastOrTags(emptyList, "Test Cast");

    // 验证默认值
    QCOMPARE(fdModel.rootPath(), QString());
    QCOMPARE(fdModel.GUID(), QString());

    // 额外测试：非法section调用headerData
    QCOMPARE(fdModel.headerData(1, Qt::Horizontal).toInt(), 2);
    QCOMPARE(fdModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 0 + 1);
    QCOMPARE(fdModel.headerData(1, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 1 + 1);
    QCOMPARE(fdModel.headerData(1000, Qt::Vertical, Qt::TextAlignmentRole).toInt(), (int)Qt::AlignRight);

    // 测试无效索引调用siblingAtColumn
    QModelIndex sibling = invalidIndex.siblingAtColumn(0);
    QVERIFY(!sibling.isValid());
  }

  void data_retrieve_ok() {
    TDir tDir;
    QVERIFY(tDir.IsValid());
    const QString dbName{tDir.itemPath("FdBasedMovieTest.db")};
    const QString connName{"FdBasedMovieTestConn"};
    const QString tableName{"ABCDEF12_3456_7890_ABCDEF1234567890"};  // can be converted to guid

    const QList<FsNodeEntry> nodes{
        {"Chris Evans.mp4", false, "Chris Evans"},
        {"Chris Hemsworth.mp4", false, "Chris Hemsworth"},
        {"Chris Pine.mp4", false, "Chris Pine"},
        {"Michael Fassbender.mp4", false, "Michael Fassbender"},
        {"Cristiano Ronaldo.jpg", false, "Cristiano Ronaldo"},
    };
    QCOMPARE(tDir.createEntries(nodes), 5);
    QSet<QString> movieNames;
    QSet<QString> movieSizes;
    QSet<QString> absolutePathSet;
    for (const FsNodeEntry& node : nodes) {
      if (node.relativePathToNode.endsWith("jpg")) {
        continue;
      }
      movieNames.insert(node.relativePathToNode);
      movieSizes.insert(QString::asprintf("0'0'0'%d", node.contents.size()));
      absolutePathSet.insert(tDir.path());
    }

    FdBasedDb movieDb{dbName, connName};
    QVERIFY(movieDb.IsValid());
    QVERIFY(movieDb.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
    QVERIFY(movieDb.IsTableExist(tableName));
    QVERIFY(movieDb.IsTableEmpty(tableName));

    FdBasedDbModel movieModel{nullptr, movieDb.GetDb()};
    movieModel.setTable(tableName);
    movieModel.select();
    QCOMPARE(movieModel.GUID(), "ABCDEF12-3456-7890-ABCDEF1234567890");
    QCOMPARE(movieModel.rowCount(), 0);
    {
      // .jpg is not videos. ignored
      QCOMPARE(movieDb.ReadADirectory(tableName, tDir.path()), 5 - 1);
      movieModel.select();
      QCOMPARE(movieModel.rowCount(), 5 - 1);
      // todo: check data here

      QVERIFY(!movieModel.data(movieModel.index(0, MOVIE_TABLE::Duration), Qt::DisplayRole).toString().isNull());

      QSet<QString> actualMovieNames;
      QSet<QString> actualMovieSizes;
      QSet<QString> actualAbsolutePathSet;
      QSet<QString> actualFileNameSet;
      for (int i = 0; i < movieModel.rowCount(); ++i) {
        actualMovieNames.insert(movieModel.data(movieModel.index(i, MOVIE_TABLE::Name), Qt::DisplayRole).toString());
        actualMovieSizes.insert(movieModel.data(movieModel.index(i, MOVIE_TABLE::Size), Qt::DisplayRole).toString());
        actualAbsolutePathSet.insert(movieModel.absolutePath(movieModel.index(i, MOVIE_TABLE::Name)));
        actualFileNameSet.insert(movieModel.fileName(movieModel.index(i, MOVIE_TABLE::Name)));
      }
      QCOMPARE(actualMovieNames, movieNames);
      QCOMPARE(actualMovieSizes, movieSizes);
      QCOMPARE(actualAbsolutePathSet, absolutePathSet);
      QCOMPARE(actualFileNameSet, movieNames);
    }

    QList<FsNodeEntry> nodesExtra{
        {"Raphael Varane.mp4", false, "Raphael Varane"},
        {"Alvaro Morata.mp4", false, "Alvaro Morata"},
    };
    QCOMPARE(tDir.createEntries(nodesExtra), 2);
    {
      // .jpg is not videos. ignored
      QCOMPARE(movieDb.ReadADirectory(tableName, tDir.path()), 2);
      movieModel.select();
      QCOMPARE(movieModel.rowCount(), 5 - 1 + 2);
      QSet<QString> expectSize{"0'0'0'14", "0'0'0'13"};
      QSet<QString> actualSize{movieModel.data(movieModel.index(4, MOVIE_TABLE::Size), Qt::DisplayRole).toString(),
                               movieModel.data(movieModel.index(5, MOVIE_TABLE::Size), Qt::DisplayRole).toString()};
      QCOMPARE(actualSize, expectSize);

      QSet<QString> expectFullInfo{
          QString() + "Raphael Varane.mp4" + '\t' + "0'0'0'14" + '\t' +
              PathTool::RMFComponent::FromPath(tDir.itemPath("Raphael Varane.mp4")).middlePart,
          QString() + "Alvaro Morata.mp4" + '\t' + "0'0'0'13" + '\t' +
              PathTool::RMFComponent::FromPath(tDir.itemPath("Alvaro Morata.mp4")).middlePart,
      };
      QSet<QString> actualFullInfo{
          movieModel.fullInfo(movieModel.index(4, MOVIE_TABLE::Size)),
          movieModel.fullInfo(movieModel.index(5, MOVIE_TABLE::Size)),
      };
      QCOMPARE(actualFullInfo, expectFullInfo);
    }

    // 2.1 Studio in table modified
    {
      QModelIndex index4Studio{movieModel.index(4, MOVIE_TABLE::Studio)};
      QModelIndex index5Studio{movieModel.index(5, MOVIE_TABLE::Studio)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Studio, index5Studio},  //
                                                 {"", ""}));
      movieModel.SetStudio({index4Studio, index5Studio}, "Marvel");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Studio, index5Studio},  //
                                                 {"Marvel", "Marvel"}));
      movieModel.SetStudio({index4Studio}, "Century");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Studio, index5Studio},  //
                                                 {"Century", "Marvel"}));
      movieModel.SetStudio({index5Studio}, "");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Studio, index5Studio},  //
                                                 {"Century", ""}));
    }

    // 2.2 Cast modified
    {
      QModelIndex index4Cast{movieModel.index(4, MOVIE_TABLE::Cast)};
      QModelIndex index5Cast{movieModel.index(5, MOVIE_TABLE::Cast)};
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 {"", ""}));
      movieModel.SetCastOrTags({index4Cast, index5Cast}, "Cristiano Ronaldo&Kaka");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 {"Cristiano Ronaldo,Kaka", "Cristiano Ronaldo,Kaka"}));
      movieModel.SetCastOrTags({index4Cast}, "Kaka");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 {"Cristiano Ronaldo,Kaka", "Kaka"}));
      movieModel.SetCastOrTags({index5Cast}, "");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 {"", "Kaka"}));

      movieModel.AddCastOrTags({index4Cast, index5Cast}, "");  // add empty nothing happend
      movieModel.AddCastOrTags({index4Cast, index5Cast}, "Levi,Michael Fassbender");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 {"Levi,Michael Fassbender", "Kaka,Levi,Michael Fassbender"}));

      movieModel.AddCastOrTags({index4Cast, index5Cast}, "Levi,Michael");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  // again no change
                                                 {"Levi,Michael,Michael Fassbender", "Kaka,Levi,Michael,Michael Fassbender"}));

      movieModel.RmvCastOrTags({index4Cast, index5Cast}, "");                           // remove empty nothing changed
      movieModel.RmvCastOrTags({index4Cast, index5Cast}, "Ronaldo");                    // not exists at all
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 {"Levi,Michael,Michael Fassbender", "Kaka,Levi,Michael,Michael Fassbender"}));

      movieModel.RmvCastOrTags({index4Cast, index5Cast}, "Michael");                    // remove full match. not exists at all
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Cast, index5Cast},  //
                                                 {"Levi,Michael Fassbender", "Kaka,Levi,Michael Fassbender"}));
    }
    // 2.3 Tags modified
    {
      QModelIndex index4Tags{movieModel.index(4, MOVIE_TABLE::Tags)};
      QModelIndex index5Tags{movieModel.index(5, MOVIE_TABLE::Tags)};

      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 {"", ""}));
      movieModel.SetCastOrTags({index4Tags, index5Tags}, "Cristiano Ronaldo&Kaka");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 {"Cristiano Ronaldo,Kaka", "Cristiano Ronaldo,Kaka"}));
      movieModel.SetCastOrTags({index4Tags}, "Kaka");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 {"Cristiano Ronaldo,Kaka", "Kaka"}));
      movieModel.SetCastOrTags({index5Tags}, "");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 {"", "Kaka"}));

      movieModel.AddCastOrTags({index4Tags, index5Tags}, "");  // add empty nothing happend
      movieModel.AddCastOrTags({index4Tags, index5Tags}, "Levi,Michael Fassbender");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 {"Levi,Michael Fassbender", "Kaka,Levi,Michael Fassbender"}));

      movieModel.AddCastOrTags({index4Tags, index5Tags}, "Levi,Michael");
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  // again no change
                                                 {"Levi,Michael,Michael Fassbender", "Kaka,Levi,Michael,Michael Fassbender"}));

      movieModel.RmvCastOrTags({index4Tags, index5Tags}, "");                           // remove empty nothing changed
      movieModel.RmvCastOrTags({index4Tags, index5Tags}, "Ronaldo");                    // not exists at all
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 {"Levi,Michael,Michael Fassbender", "Kaka,Levi,Michael,Michael Fassbender"}));

      movieModel.RmvCastOrTags({index4Tags, index5Tags}, "Michael");                    // remove full match. not exists at all
      QVERIFY(CheckIndexesDisplayRoleIgnoreOrder(movieModel, {index4Tags, index5Tags},  //
                                                 {"Levi,Michael Fassbender", "Kaka,Levi,Michael Fassbender"}));
    }
  }
};

#include "FdBasedDbModelTest.moc"
REGISTER_TEST(FdBasedDbModelTest, false)
