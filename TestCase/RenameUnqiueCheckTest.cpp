#include <QCoreApplication>
#include <QtTest>

#include "Tools/RenameNamesUnique.h"
#include "pub/FileSystemTestSuite.h"

class RenameUnqiueCheckTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  RenameUnqiueCheckTest() : FileSystemTestSuite{"TestEnv_RenameNamesUnique", false, false} {}

 private slots:
  void initTestCase() {
    /*
    rename12To45Basic{1, 2}
    renameDiretoryNameShouldNotConflict{dirName{dirName, file1}}
    */
    m_rootHelper << FileSystemNode{"rename12To45Basic"} << FileSystemNode{"renameDiretoryNameShouldNotConflict"};
    m_rootHelper.GetSubHelper("rename12To45Basic") << FileSystemNode{"1"} << FileSystemNode{"2"};
    m_rootHelper.GetSubHelper("renameDiretoryNameShouldNotConflict") << FileSystemNode{"dirName"};
    m_rootHelper.GetSubHelper("renameDiretoryNameShouldNotConflict").GetSubHelper("dirName") << FileSystemNode{"dirName"} << FileSystemNode{"file1"};
  }

  void cleanupTestCase() {  //
    FileSystemHelper(mTestPath).EraseFileSystemTree(true);
  }

  void occupiedNameOfrename12To45Basic() {
    const QString pre = QDir(mTestPath).absoluteFilePath("rename12To45Basic");
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
    const QString pre = QDir(mTestPath).absoluteFilePath("renameDiretoryNameShouldNotConflict");
    const QSet<QString>& occupied = RenameNamesUnique::getOccupiedPostPath(pre, {"", "dirName", "dirName"}, {"dirName", "file1", "dirName"}, true);
    const QSet<QString> actual{"dirName", "dirName/file1", "dirName/dirName"};
    QCOMPARE(occupied, actual);
  }

  void renameDiretoryNameShouldNotConflict() {
    // dirName/file1, dirName
    // dir Name/file 1, dir Name
    bool ans = RenameNamesUnique::CheckConflict({"dirName", "dirName/file1", "dirName/dirName"},  //
                                                {"", "dirName", "dirName"},                       //
                                                {"dirName", "file1", "dirName"},                  //
                                                {"dir Name", "file 1", "dir Name"},               //
                                                m_conflictNames);
    QVERIFY(ans);
  }

 private:
  QStringList m_conflictNames;
};

#include "RenameUnqiueCheckTest.moc"
RenameUnqiueCheckTest g_RenameUnqiueCheckTest;
