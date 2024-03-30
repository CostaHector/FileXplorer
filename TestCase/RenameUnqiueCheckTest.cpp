#include <QCoreApplication>
#include <QtTest>

#include "Tools/RenameNamesUnique.h"

const QString TEST_SRC_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_RenameNamesUnique");

class RenameUnqiueCheckTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  void occupiedNameOfrename12To45Basic() {
    const QString pre = QDir(TEST_SRC_DIR).absoluteFilePath("rename12To45Basic");
    const QSet<QString>& occupied = RenameNamesUnique::getOccupiedPostPath(pre, {"", ""}, {"1", "2"}, true);
    const QSet<QString> actual{"1", "2"};
    QCOMPARE(occupied, actual);
  }

  void rename12To45Basic() {
    bool ans = RenameNamesUnique::CheckConflict({"1", "2"}, {"", ""}, {"1", "2"}, {"4", "5"}, m_conflictNames);
    QVERIFY(ans);
  }

  void rename12To23ShouldConflict() {
    bool ans = RenameNamesUnique::CheckConflict({"1", "2"}, {"", ""}, {"1", "2"}, {"2", "3"}, m_conflictNames);
    QVERIFY(not ans);
  }

  void rename21To32ShouldNotConflictAfterwapTheSequence() {
    bool ans = RenameNamesUnique::CheckConflict({"2", "1"}, {"", ""}, {"2", "1"}, {"3", "2"}, m_conflictNames);
    QVERIFY(ans);
  }

  void rename21To23ShouldNotConflict() {
    bool ans = RenameNamesUnique::CheckConflict({"2", "1"}, {"", ""}, {"2", "1"}, {"2", "3"}, m_conflictNames);
    QVERIFY(ans);
  }

  void occupiedNameOfrenameDiretoryNameShouldNotConflict() {
    const QString pre = QDir(TEST_SRC_DIR).absoluteFilePath("renameDiretoryNameShouldNotConflict");
    const QSet<QString>& occupied = RenameNamesUnique::getOccupiedPostPath(pre, {"", "dirName", "dirName"}, {"dirName", "file1", "dirName"}, true);
    const QSet<QString> actual{"dirName", "dirName/file1", "dirName/dirName"};
    QCOMPARE(occupied, actual);
  }

  void renameDiretoryNameShouldNotConflict() {
    // dirName/file1, dirName
    // dir Name/file 1, dir Name
    bool ans = RenameNamesUnique::CheckConflict({"dirName", "dirName/file1", "dirName/dirName"}, {"", "dirName", "dirName"},
                                                {"dirName", "file1", "dirName"}, {"dir Name", "file 1", "dir Name"}, m_conflictNames);
    QVERIFY(ans);
  }

 private:
  QStringList m_conflictNames;
};

void RenameUnqiueCheckTest::initTestCase() {}

void RenameUnqiueCheckTest::cleanupTestCase() {}

void RenameUnqiueCheckTest::init() {
  m_conflictNames.clear();
}

void RenameUnqiueCheckTest::cleanup() {}

//QTEST_MAIN(RenameUnqiueCheckTest)

//#include "RenameUnqiueCheckTest.moc"
