#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "LineEditGeneral.h"
#include "EndToExposePrivateMember.h"

#include "LineEditColor.h" // test iconUrl is nullptr only

#include "PathTool.h"
#include "PublicVariable.h"

#include <QLineEdit>
#include <QColorDialog>
#include <QFileDialog>
#include <QAction>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class LineEditGeneralTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void create_factory_ok() {
    QWidget wid;
    LineEditGeneral *invalidLineEdit = LineEditGeneral::create(GeneralDataType::Type::ERROR_TYPE, &wid);
    QVERIFY(invalidLineEdit == nullptr);

    LineEditGeneral *colorLineEdit = LineEditGeneral::create(GeneralDataType::Type::COLOR, &wid);
    LineEditGeneral *filePathLineEdit = LineEditGeneral::create(GeneralDataType::Type::FILE_PATH, &wid);
    LineEditGeneral *imgfilePathLineEdit = LineEditGeneral::create(GeneralDataType::Type::IMAGE_PATH_OPTIONAL, &wid);
    LineEditGeneral *folderPathLineEdit = LineEditGeneral::create(GeneralDataType::Type::FOLDER_PATH, &wid);
    QVERIFY(colorLineEdit != nullptr);
    QVERIFY(filePathLineEdit != nullptr);
    QVERIFY(imgfilePathLineEdit != nullptr);
    QVERIFY(folderPathLineEdit != nullptr);
  }

  void no_action_lineEdit_ok() {
    QWidget wid;
    LineEditGeneral *noIconNoActionFileLineEdit = new LineEditColor{GeneralDataType::Type::COLOR, nullptr, &wid};
    QVERIFY(noIconNoActionFileLineEdit != nullptr);
    QList<QAction *> acts = noIconNoActionFileLineEdit->actions();
    QCOMPARE(acts.size(), 0);
  }

  void color_lineedit_onActionTriggered_ok() {
    QWidget wid;
    LineEditGeneral *colorLineEdit = LineEditGeneral::create(GeneralDataType::Type::COLOR, &wid);
    colorLineEdit->setText("invalidColorStr");

    QList<QAction *> acts = colorLineEdit->actions();
    QCOMPARE(acts.size(), 1);
    // 点击输出两次
    MOCKER(QColorDialog::getColor)             //
        .expects(exactly(2))                   //
        .will(returnValue(QColor{}))           //
        .then(returnValue(QColor{"#123456"})); // name(QColor::HexArgb) 转化为8位

    QAction *colorAction = acts.front();
    QVERIFY(colorAction != nullptr);
    colorAction->trigger(); // 第一次
    QCOMPARE(colorLineEdit->text(), "invalidColorStr");

    colorAction->trigger(); // 第二次
    QCOMPARE(colorLineEdit->text().toUpper(), "#FF123456");
  }

  void image_lineedit_onActionTriggered_ok() {
    QWidget wid;
    LineEditGeneral *fileLineEdit = LineEditGeneral::create(GeneralDataType::Type::IMAGE_PATH_OPTIONAL, &wid);
    fileLineEdit->setText("invalid/file/path");

    QList<QAction *> acts = fileLineEdit->actions();
    QCOMPARE(acts.size(), 1);

    const QString parmOpenedPath{SystemPath::HOME_PATH() + "/Pictures"};
    QString *expectSelectedFilter{nullptr};
    const QFileDialog::Options parmOptions{QFileDialog::Option::DontUseNativeDialog};
    QString backSlashPath = QString{__FILE__}.replace('/', '\\');
    const QString expectFileSelected{backSlashPath};
    const QString filePathInLineEdit{PathTool::normPath(expectFileSelected)};

    // 点击输出两次
    MOCKER(QFileDialog::getOpenFileName)                                                          //
        .expects(exactly(2))                                                                      //
        .with(any(), any(), eq(parmOpenedPath), any(), eq(expectSelectedFilter), eq(parmOptions)) //
        .will(returnValue(QString{}))                                                             //
        .then(returnValue(expectFileSelected));                                                   //

    QAction *fileAction = acts.front();
    QVERIFY(fileAction != nullptr);
    fileAction->trigger(); // 第一次
    QCOMPARE(fileLineEdit->text(), "invalid/file/path");

    fileAction->trigger(); // 第二次
    QCOMPARE(fileLineEdit->text(), filePathInLineEdit);
  }

  void folder_lineedit_onActionTriggered_ok() {
    QWidget wid;
    LineEditGeneral *folderLineEdit = LineEditGeneral::create(GeneralDataType::Type::FOLDER_PATH, &wid);
    folderLineEdit->setText("invalid/folder/path");

    QList<QAction *> acts = folderLineEdit->actions();
    QCOMPARE(acts.size(), 1);

    const QString parmOpenedPath{SystemPath::HOME_PATH()};
    const QFileDialog::Options parmOptions{QFileDialog::Option::DontUseNativeDialog | QFileDialog::Option::ShowDirsOnly};
    QString selectedFolder{QFileInfo{__FILE__}.absolutePath()};
    QString backSlashPath = selectedFolder.replace('/', '\\');
    const QString expectFolderSelected{backSlashPath};
    const QString folderPathInLineEdit{PathTool::normPath(expectFolderSelected)};

    // 点击输出两次
    MOCKER(QFileDialog::getExistingDirectory)                    //
        .expects(exactly(2))                                     //
        .with(any(), any(), eq(parmOpenedPath), eq(parmOptions)) //
        .will(returnValue(QString{}))                            //
        .then(returnValue(expectFolderSelected));                //

    QAction *fileAction = acts.front();
    QVERIFY(fileAction != nullptr);
    fileAction->trigger(); // 第一次
    QCOMPARE(folderLineEdit->text(), "invalid/folder/path");

    fileAction->trigger(); // 第二次
    QCOMPARE(folderLineEdit->text(), folderPathInLineEdit);
  }
};

#include "LineEditGeneralTest.moc"
REGISTER_TEST(LineEditGeneralTest, false)
