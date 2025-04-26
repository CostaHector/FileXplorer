#ifndef MYTESTSUITE_H
#define MYTESTSUITE_H
#include <QObject>
#include <QVector>
// 操作内存, 不操作文件系统, 不操作数据库系统的测试套从本父类中继承, 并创建好一个全局对象
// 在main函数中调用 QTest::qExec(pQObject, argc, argv)即可
class MyTestSuite : public QObject {
 public:
  MyTestSuite(bool bExculsive=false);
  static QVector<MyTestSuite*> & suite();
  bool mExclusive{false};
  static bool bOnlyExecuteExculsive;
};

#endif // MYTESTSUITE_H
