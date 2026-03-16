#ifndef PLAINTESTSUITE_H
#define PLAINTESTSUITE_H
#include <QObject>
#include <QVector>
// 操作内存, 不操作文件系统, 不操作数据库系统的测试套从本父类中继承, 并创建好一个全局对象
// 在main函数中调用 QTest::qExec(pQObject, argc, argv)即可
#include <functional>

using FTestCaseObjCreater = std::function<QObject*()>;
struct TestCaseHelper {
  FTestCaseObjCreater pCreater;
  QString locatedIn;
};
class PlainTestSuite : public QObject {
public:
  friend int main(int argc, char* argv[]);
  explicit PlainTestSuite(QObject *parent=nullptr);
  static int AddATestCase(FTestCaseObjCreater fObjCrt, bool bExclusive=false, const QString& locatedIn="");
  static int mTotalTestCaseCount;
private:
  static QVector<TestCaseHelper>& sharedSuite();
  static QVector<TestCaseHelper>& exclusiveSuite();
};

#define REGISTER_TEST(testClassName, bExclusive) \
namespace {\
    int _auto_reg_##testClassName = PlainTestSuite::AddATestCase(\
                                   []()->QObject*{return new (std::nothrow) testClassName;},\
                                   bExclusive,\
                                   __FILE__);\
}\

#endif // PLAINTESTSUITE_H
