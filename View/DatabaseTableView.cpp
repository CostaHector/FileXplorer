#include "DatabaseTableView.h"
#include <QDesktopServices>
#include <QHeaderView>
#include "Component/DatabaseToolBar.h"
#include "PublicVariable.h"
#include "Tools/SubscribeDatabase.h"
#include "View/DragDropTableView.h"

DatabaseTableView::DatabaseTableView() : dbModel(nullptr) {
  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

  QSqlDatabase con = GetSqlDB();

  dbModel = new MyQSqlTableModel(this, con);
  dbModel->setTable(TABLE_NAME);
  dbModel->setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
  dbModel->select();

  this->setModel(dbModel);

  this->InitViewSettings();
  subscribe();
}

auto DatabaseTableView::InitViewSettings() -> void {
  setShowGrid(false);
  setAlternatingRowColors(true);
  setSortingEnabled(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  verticalHeader()->setVisible(false);
  verticalHeader()->setDefaultSectionSize(ROW_SECTION_HEIGHT);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
  horizontalHeader()->setStretchLastSection(false);
  horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  DatabaseTableView::SetViewColumnWidth();
  DatabaseTableView::UpdateItemViewFontSize();
}

auto DatabaseTableView::SetViewColumnWidth() -> void {
  const auto columnWidth = PreferenceSettings().value(MemoryKey::NAME_COLUMN_WIDTH.name, MemoryKey::NAME_COLUMN_WIDTH.v).toInt();
  qDebug("%d:%d", DB_HEADER_KEY::DB_NAME_INDEX, columnWidth);
  setColumnWidth(DB_HEADER_KEY::DB_NAME_INDEX, columnWidth);
}

auto DatabaseTableView::UpdateItemViewFontSize() -> void {
  View::UpdateItemViewFontSizeCore(this);
}

void DatabaseTableView::subscribe() {
  connect(this, &QTableView::doubleClicked, this, &DatabaseTableView::on_cellDoubleClicked);
}

auto DatabaseTableView::on_cellDoubleClicked(QModelIndex clickedIndex) -> bool {
  if (not clickedIndex.isValid()) {
    return false;
  }
  if (not dbModel) {
    return false;
  }

  QFileInfo fi = dbModel->fileInfo(clickedIndex);
  qDebug("Enter(%d, %d) [%s]", clickedIndex.row(), clickedIndex.column(), fi.fileName().toStdString().c_str());
  if (not fi.exists()) {
    qDebug("[path inexists] %s", fi.absoluteFilePath().toStdString().c_str());
    return false;
  }
  if (fi.isSymLink()) {
#ifdef _WIN32
    QString tarPath = fi.symLinkTarget();
#else  // ref: https://doc.qt.io/qt-6/qfileinfo.html#isSymLink
    QString tarPath(fi.absoluteFilePath());
#endif
    fi = QFileInfo(tarPath);
    if (not fi.exists()) {
      qDebug("[link inexists] %s", fi.absoluteFilePath().toStdString().c_str());
      return false;
    }
  }
  QString path(fi.absoluteFilePath());
  return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
}

#include <QMainWindow>
class MoviesDatabase : public QMainWindow {
 public:
  DatabaseToolBar* databaseTB;
  MyQSqlTableModel* dbModel;
  DatabaseTableView* view;
  explicit MoviesDatabase(QWidget* parent = nullptr)
      : QMainWindow(parent), databaseTB(new DatabaseToolBar("Movies Database Toolbar", this)), dbModel(nullptr), view(new DatabaseTableView) {
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, databaseTB);
    this->setCentralWidget(this->view);
    this->setWindowTitle("QTableView Example");
    this->resize(1400, 768);
  }
};

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
//  QWidget widget;
//  QMessageBox::warning(&widget, "FILE_INFO_DATABASE path", SystemPath::FILE_INFO_DATABASE);
//  widget.show();
  MoviesDatabase win;
  win.show();
  auto* eventImplementer = new SubscribeDatabase(win.view, win.view->dbModel, win.databaseTB->sqlSearchLE);
  return a.exec();
}
#endif
