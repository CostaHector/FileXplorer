#ifndef CONFLICTSRECYCLE_H
#define CONFLICTSRECYCLE_H

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QCursor>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QObject>
#include <QPoint>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSize>
#include <QSizePolicy>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTableWidgetSelectionRange>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>

#include "FileOperation/FileOperation.h"
#include "PublicVariable.h"
#include "Tools/ConflictsItemHelper.h"

class FileInfoType {
 public:
  QString name;
  QString size;
  QString dateModified;
};

class ConflictsRecycle : public QDialog {
  Q_OBJECT
 public:
  const ConflictsItemHelper itemIF;  // dont user reference. object point to will be release after statement
  const QString leftFolderPath;
  const QString rightFolderPath;
  const int ROW_COUNT;
  const CCMMode OP;
  QList<FileInfoType> lCommonInfo;
  QList<FileInfoType> rCommonInfo;

  QPushButton* leftFolderBtn;
  QPushButton* rightFolderBtn;
  QTableWidget* leftConflict;
  QTableWidget* rightConflict;
  QTableWidget* leftRestore;
  QTableWidget* rightRestore;
  QDialogButtonBox* buttonBox;

  QAction* RECYCL_LEFT;
  QAction* RECYCL_RIGHT;
  QAction* RESTORE_LEFT;
  QAction* RESTORE_RIGHT;

  static constexpr int CONFLICT_NAME_COLUMN_INDEX = 2;
  static const QStringList COLUMNS_NAME_LIST;
  static const int COLUMNS_NAME_LIST_LEN;

  ConflictsRecycle(const ConflictsItemHelper& itemIF_, const CCMMode& operation = CCMMode::MERGE, QWidget* parent = nullptr)
      : QDialog(parent),
        itemIF(itemIF_),
        leftFolderPath(itemIF.l),
        rightFolderPath(itemIF.r),
        ROW_COUNT(itemIF.commonList.size()),
        OP(operation),
        lCommonInfo(GetCommonInfo(itemIF.l)),
        rCommonInfo(GetCommonInfo(itemIF.r)),
        leftFolderBtn(new QPushButton(leftFolderPath)),
        rightFolderBtn(new QPushButton(rightFolderPath)),
        leftConflict(TableWidgetGetter()),
        rightConflict(TableWidgetGetter()),
        leftRestore(TableWidgetGetter()),
        rightRestore(TableWidgetGetter()),
        buttonBox(new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel)),
        RECYCL_LEFT(new QAction(QIcon(":/themes/RECYCLE"), "Recycle", leftConflict)),
        RECYCL_RIGHT(new QAction(QIcon(":/themes/RECYCLE"), "Recycle", rightConflict)),
        RESTORE_LEFT(new QAction(QIcon(":/themes/RESTORE"), "Restore", leftRestore)),
        RESTORE_RIGHT(new QAction(QIcon(":/themes/RESTORE"), "Restore", rightRestore)) {
    leftConflict->setColumnHidden(COLUMNS_NAME_LIST_LEN - 1, true);
    leftRestore->setColumnHidden(COLUMNS_NAME_LIST_LEN - 1, true);

    auto* conflictsPane = new QHBoxLayout;
    conflictsPane->addWidget(leftConflict, 3);
    conflictsPane->addWidget(rightConflict, 7);

    auto* recyclePane = new QHBoxLayout;
    recyclePane->addWidget(leftRestore, 3);
    recyclePane->addWidget(rightRestore, 7);

    buttonBox->button(QDialogButtonBox::Ok)->setText("Submit");

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(leftFolderBtn, 0, Qt::AlignLeft);
    mainLayout->addWidget(GetQuickControlToolBar());
    mainLayout->addWidget(rightFolderBtn, 0, Qt::AlignRight);
    mainLayout->addLayout(conflictsPane, 7);
    mainLayout->addLayout(recyclePane, 3);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    ConflictTableEvent(RECYCL_LEFT, leftConflict, leftRestore, rightConflict);
    ConflictTableEvent(RECYCL_RIGHT, rightConflict, rightRestore, leftConflict);
    RecycleTableEvent(RESTORE_LEFT, leftRestore, leftConflict, rightConflict);
    RecycleTableEvent(RESTORE_RIGHT, rightRestore, rightConflict, leftConflict);

    InitData();
    installEventFilter(this);
    Subscribe();
    setWindowTitle(QString("[%1] Conflicts | %2 item(s)").arg(CCMMode2QString.value(OP, "unknown")).arg(ROW_COUNT));
    setWindowIcon(QIcon(":/themes/CONFLICT"));
  }
  auto GetQuickControlToolBar() -> QToolBar*;
  auto TableWidgetGetter() const -> QTableWidget*;
  auto ConflictTableEvent(QAction* RECYCLE, QTableWidget* conflictTw, QTableWidget* recycleTw, QTableWidget* opsiteConflictTW) -> void;
  auto RecycleTableEvent(QAction* RESTORE, QTableWidget* recycleTw, QTableWidget* conflictTw, QTableWidget* opositeConflictTw) -> void;
  static auto getHorizontalSpacer() -> QWidget* {
    auto* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return spacer;
  }

  auto GetCommonInfo(const QString& prePath) -> QList<FileInfoType> {
    static auto infos = [](const QString& rel, const QString& nm) -> FileInfoType {
      QFileInfo fileInfo(QDir(rel).absoluteFilePath(nm));
      return {nm, QString::number(fileInfo.size()), fileInfo.lastModified().toString("yyyyMMdd hhmmss")};
    };
    QList<FileInfoType> commonInfo;
    for (const auto& nm : itemIF.commonList) {
      commonInfo.append(infos(prePath, nm));
    }
    return commonInfo;
  }

  static auto FindListCommon(const QStringList& lLst, const QStringList& rLst) -> QStringList {
    const QStringList* longerLst = nullptr;
    const QStringList* shortLst = nullptr;
    if (lLst.size() > rLst.size()) {
      longerLst = &lLst;
      shortLst = &rLst;
    } else {
      shortLst = &lLst;
      longerLst = &rLst;
    }
    QStringList commonList;  // setA.intersection(setB) is not stable
    QSet<QString> largeSpaceSet(longerLst->cbegin(), longerLst->cend());
    for (const QString& nm : *shortLst) {
      if (largeSpaceSet.contains(nm)) {
        commonList.append(nm);
      }
    }
    return commonList;
  }
  auto InitData() -> void;
  auto on_Size() -> void;
  auto on_Date() -> void;
  auto on_Revert() -> void;
  auto on_Check() -> QPair<QList<int>, QList<int>> {
    QList<int> invalidLineList;
    QList<int> bothRemoveLineList;
    for (auto r = 0; r < ROW_COUNT; ++r) {
      int nonElementCnt = (leftConflict->item(r, 0) == nullptr) + (rightConflict->item(r, 0) == nullptr);
      if (nonElementCnt == 0) {
        invalidLineList.append(r + 1);  // "the (r+1)th line";
      } else if (nonElementCnt == 2) {
        bothRemoveLineList.append(r + 1);
      }
    }
    return {invalidLineList, bothRemoveLineList};
  }
  static inline QString QIntStr2QString(const QList<int>& list) {
    QString joinedStr;
    for (int i = 0; i < list.size(); i++) {
      joinedStr += QString::number(list[i]);
      if (i < list.size() - 1) {
        joinedStr += ",";
      }
    }
    return joinedStr;
  }

  auto NoConflictOperation(const QStringList& selectedItems, const QSet<QString>& leftDeleteSet) const -> FileOperation::BATCH_COMMAND_LIST_TYPE;

  auto on_Submit() -> bool;
  auto Subscribe() -> void;
  auto on_OpenButtonTextFolder(const QPushButton* btn) -> bool { return QDesktopServices::openUrl(QUrl::fromLocalFile(btn->text())); }
  auto on_Open(const QModelIndex index) -> bool;
  auto eventFilter(QObject* obj, QEvent* event) -> bool override;

  auto sizeHint() const -> QSize override { return QSize(1024, 768); }
};
#endif  // CONFLICTSRECYCLE_H
