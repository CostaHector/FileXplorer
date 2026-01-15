#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "CopyStringListToClipboard.h"
#include "ClipboardGuard.h"

class CopyStringListToClipboardTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void path_string_list_copy() {
    ClipboardGuard clip;
    QVERIFY(clip);
    auto* pClipboard = clip.clipBoard();
    QVERIFY(pClipboard != nullptr);

    // 测试非空列表复制功能
    QStringList testList = {"Item1", "Item2", "Item3"};
    bool result = CopyStringListToClipboard::PathStringListCopy(testList, "copied");
    QVERIFY(result);
    QCOMPARE(pClipboard->text(), testList.join('\n')); // 验证剪贴板内容

    // 测试空列表情况
    QStringList emptyList;
    QString originalText = pClipboard->text(); // 保存当前剪贴板内容
    result = CopyStringListToClipboard::PathStringListCopy(emptyList, "empty");
    QVERIFY(result);
    QCOMPARE(pClipboard->text(), originalText); // 验证剪贴板内容未改变
  }
};

#include "CopyStringListToClipboardTest.moc"
REGISTER_TEST(CopyStringListToClipboardTest, false)
