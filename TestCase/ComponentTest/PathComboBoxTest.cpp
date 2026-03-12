#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "PathComboBox.h"
#include "EndToExposePrivateMember.h"

class PathComboBoxTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void focusChanged_ok() {
    PathComboBox pathComboBox;
    pathComboBox.clearFocus();
    QVERIFY(!pathComboBox.hasFocus());

    QFocusEvent inEvent{QEvent::Type::MouseButtonPress, Qt::FocusReason::MouseFocusReason};
    QSignalSpy focusChangeSpy{&pathComboBox, &PathComboBox::focusChanged};
    pathComboBox.focusInEvent(&inEvent);
    QCOMPARE(focusChangeSpy.count(), 1);
    QCOMPARE(focusChangeSpy.takeLast(), (QVariantList{true}));

    QFocusEvent outEvent{QEvent::Type::FocusOut, Qt::FocusReason::NoFocusReason};
    pathComboBox.focusOutEvent(&outEvent);
    QCOMPARE(focusChangeSpy.count(), 1);
    QCOMPARE(focusChangeSpy.takeLast(), (QVariantList{false}));
  }
};

#include "PathComboBoxTest.moc"
REGISTER_TEST(PathComboBoxTest, false)
