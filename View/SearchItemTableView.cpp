#include "SearchItemTableView.h"
#include <QDesktopServices>
#include <QHeaderView>

SearchItemTableView::SearchItemTableView(const QString& rootPath, QWidget* parent)
    : QWidget(parent),
      m_rootPath(rootPath.isEmpty()
                     ? PreferenceSettings().value(MemoryKey::SEARCH_MODEL_ROOT_PATH.name, MemoryKey::SEARCH_MODEL_ROOT_PATH.v).toString()
                     : rootPath),
      m_tv(new QTableView(this)),
      _FILES(new QAction("Files", this)),
      _DIRS(new QAction("Dirs", this)),
      DIR_FILTER_AG(new QActionGroup(this)),
      _VID(new QAction("Video", this)),
      _IMG(new QAction("Picture", this)),
      _DOC(new QAction("Document", this)),
      _PLAIN_TEXT(new QAction("PlainText", this)),
      _CODE(new QAction("Code", this)),
      FREQ_TYPE_FILTER(new QActionGroup(this)),
      m_filterTB(new QToolBar("Filter", this)),
      m_rootPathCB(new QComboBox(this)),
      m_rootPathLE(new QLineEdit(this)),
      m_dirFilterCB(new QComboBox(this)),
      m_dirFilterLE(new QLineEdit(this)),
      m_typeFilterCB(new QComboBox(this)),
      m_typeFilterLE(new QLineEdit(this)),
      m_nameFilterCB(new QComboBox(this)),
      m_nameFilterLE(new QLineEdit(this)),
      m_dirFilterStr(PreferenceSettings().value(MemoryKey::SEARCH_MODEL_DIR_FILTER.name, MemoryKey::SEARCH_MODEL_DIR_FILTER.v).toString()),
      m_typeFilterStr(PreferenceSettings().value(MemoryKey::SEARCH_MODEL_TYPE_FILTER.name, MemoryKey::SEARCH_MODEL_TYPE_FILTER.v).toString()),
      m_nameFilterStr(PreferenceSettings().value(MemoryKey::SEARCH_MODEL_NAME_FILTER.name, MemoryKey::SEARCH_MODEL_NAME_FILTER.v).toString()),
      m_proxyModel(new QSortFilterProxyModel(this)),
      m_searchModel(new SearchItemModel) {
  m_proxyModel->setSourceModel(m_searchModel);
  m_tv->setModel(m_proxyModel);

  m_tv->setAlternatingRowColors(true);
  m_tv->setSortingEnabled(true);
  m_tv->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_tv->setDragDropMode(QAbstractItemView::NoDragDrop);
  m_tv->setEditTriggers(QAbstractItemView::EditKeyPressed);
  m_tv->horizontalHeader()->setStretchLastSection(true);

  m_proxyModel->setFilterKeyColumn(0);

  m_rootPathCB->setLineEdit(m_rootPathLE);
  m_dirFilterCB->setLineEdit(m_dirFilterLE);
  m_nameFilterCB->setLineEdit(m_nameFilterLE);
  m_typeFilterCB->setLineEdit(m_typeFilterLE);

  m_rootPathLE->setText(m_rootPath);
  m_dirFilterLE->setText(m_dirFilterStr);
  m_typeFilterLE->setText(m_typeFilterStr);
  m_nameFilterLE->setText(m_nameFilterStr);

  _FILES->setCheckable(true);
  _DIRS->setCheckable(true);
  DIR_FILTER_AG->addAction(_FILES);
  DIR_FILTER_AG->addAction(_DIRS);
  DIR_FILTER_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  UpdateActionChecked(m_dirFilterLE->text());

  _VID->setCheckable(true);
  _VID->setChecked(true);
  _IMG->setCheckable(true);
  _DOC->setCheckable(true);
  _PLAIN_TEXT->setCheckable(true);
  _CODE->setCheckable(true);
  FREQ_TYPE_FILTER->addAction(_VID);
  FREQ_TYPE_FILTER->addAction(_IMG);
  FREQ_TYPE_FILTER->addAction(_DOC);
  FREQ_TYPE_FILTER->addAction(_PLAIN_TEXT);
  FREQ_TYPE_FILTER->addAction(_CODE);
  FREQ_TYPE_FILTER->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  m_filterTB->addActions(DIR_FILTER_AG->actions());
  m_filterTB->addSeparator();
  m_filterTB->addActions(FREQ_TYPE_FILTER->actions());

  qDebug("%s, %s", qPrintable(m_rootPathLE->text()), qPrintable(m_dirFilterLE->text()));
  m_searchModel->ChangeRootPathOrFilter(m_rootPathLE->text(), String2DirFilters(m_dirFilterLE->text()), m_typeFilterStr.split('|'));

  auto* lo = new QFormLayout(this);

  lo->addRow(m_filterTB);
  lo->addRow("Root Path", m_rootPathCB);
  lo->addRow("Dir::Filters", m_dirFilterCB);
  lo->addRow("Type Filters", m_typeFilterCB);
  lo->addRow("Name Regex", m_nameFilterCB);
  lo->addRow(m_tv);
  setLayout(lo);

  connect(m_rootPathLE, &QLineEdit::returnPressed, this, &SearchItemTableView::onRootPathChanged);
  connect(DIR_FILTER_AG, &QActionGroup::triggered, this, &SearchItemTableView::onDirFilterChanged);
  connect(FREQ_TYPE_FILTER, &QActionGroup::triggered, this, &SearchItemTableView::onTypeFilterChanged);
  connect(m_nameFilterLE, &QLineEdit::returnPressed, this, &SearchItemTableView::onNameFilterChanged);

  connect(m_tv, &QTableView::doubleClicked, this, &SearchItemTableView::onRowDoubleClicked);

  setWindowTitle(QString("Search | %1").arg(m_rootPath));
  setWindowIcon(QIcon(":/themes/SEARCH"));
}

void SearchItemTableView::onRowDoubleClicked(QModelIndex index) {
  auto srcIndex = m_proxyModel->mapToSource(index);
  const QString& pth = m_searchModel->filePath(srcIndex);
  const bool openRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
  qDebug("Direct open file [%s]: [%d]", qPrintable(pth), openRet);
}

void SearchItemTableView::onDirFilterChanged(QAction* dirFilterActTriggered) {
  QStringList lst;
  for (const auto* act : DIR_FILTER_AG->actions()) {
    if (act->isChecked()) {
      lst << act->text();
    }
  }
  m_dirFilterStr = lst.join('|');
  m_dirFilterLE->setText(m_dirFilterStr);
  PreferenceSettings().setValue(MemoryKey::SEARCH_MODEL_DIR_FILTER.name, m_dirFilterStr);
  m_searchModel->ChangeRootPathOrFilter(m_rootPathLE->text(), String2DirFilters(m_dirFilterStr), m_typeFilterStr.split('|'), false);
}

void SearchItemTableView::onTypeFilterChanged() {
  QStringList typeLst;
  if (_VID->isChecked()) {
    typeLst += TYPE_FILTER::VIDEO_TYPE_SET;
  }
  if (_IMG->isChecked()) {
    typeLst += TYPE_FILTER::IMAGE_TYPE_SET;
  }
  if (_PLAIN_TEXT->isChecked()) {
    typeLst += TYPE_FILTER::TEXT_TYPE_SET;
  }
  if (typeLst.isEmpty()) {
    typeLst << "*";
  }
  m_typeFilterStr = typeLst.join('|');
  m_typeFilterLE->setText(m_typeFilterStr);
  PreferenceSettings().setValue(MemoryKey::SEARCH_MODEL_TYPE_FILTER.name, m_typeFilterStr);
  m_searchModel->ChangeRootPathOrFilter(m_rootPathLE->text(), String2DirFilters(m_dirFilterStr), typeLst, false);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  SearchItemTableView win;
  win.show();
  return a.exec();
}

#endif
