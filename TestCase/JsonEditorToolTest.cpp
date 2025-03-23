#include <QtTest>
#include <QCoreApplication>
#include "pub/MyTestSuite.h"

class JsonEditorToolTest : public MyTestSuite
{
  Q_OBJECT
 public:

 private slots:
  void initTestCase() {
    qDebug("Start to test JsonEditorToolTest ...");
  }
  void cleanupTestCase() {
    qDebug("End to test JsonEditorToolTest ...");
  }
  void test_save() {
  }
  void test_cancel() {
  }
};

//JsonEditorToolTest g_jsonEditorToolTest;

#include "JsonEditorToolTest.moc"
