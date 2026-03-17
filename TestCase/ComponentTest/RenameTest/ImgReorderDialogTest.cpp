#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include <QSignalSpy>

#include "BeginToExposePrivateMember.h"
#include "ImgReorderDialog.h"
#include "EndToExposePrivateMember.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class ImgReorderDialogTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_ok() {
    QCOMPARE(ImgReorderDialog::execCore(nullptr), -1);

    ImgReorderDialog imgReorderDlg;
    QVERIFY(imgReorderDlg.m_reorderListView != nullptr);

    QShowEvent defaultShowEvent;
    imgReorderDlg.showEvent(nullptr);
    imgReorderDlg.showEvent(&defaultShowEvent);

    QVERIFY(imgReorderDlg.m_buttonBox != nullptr);
    QSignalSpy rejectSpy(&imgReorderDlg, &QDialog::rejected);
    emit imgReorderDlg.m_buttonBox->rejected();
    QCOMPARE(rejectSpy.count(), 1);

    QSignalSpy acceptedSpy(&imgReorderDlg, &QDialog::accepted);
    emit imgReorderDlg.m_buttonBox->accepted();
    QCOMPARE(acceptedSpy.count(), 1);
  }

  void basic_function_ok() {
    const QStringList filesMixedWithImages{"/Ricardo Leite.jpg", "/Cristiano Ronaldo.txt", "/Robert Lewandowski.png"};
    const QString baseName{"Kaka"};
    const int startNo{1};
    const QString namePattern{" - %1"};

    ImgReorderDialog imgReorderDlg;
    QCOMPARE(imgReorderDlg.setImagesToReorder(filesMixedWithImages, baseName, startNo, namePattern), true);

    emit imgReorderDlg.m_buttonBox->accepted();
    QCOMPARE(imgReorderDlg.getOrderedNames(), (QStringList{"Kaka - 1", "Kaka - 2", "Kaka - 3"}));
  }
};

#include "ImgReorderDialogTest.moc"
REGISTER_TEST(ImgReorderDialogTest, false)
