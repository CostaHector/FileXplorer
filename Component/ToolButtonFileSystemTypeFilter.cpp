#include "ToolButtonFileSystemTypeFilter.h"
#include "PublicVariable.h"

#include <QDebug>
#include <QSplitter>

const QDir::Filters ToolButtonFileSystemTypeFilter::DEFAULT_FILTER_FLAG =
    QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Drives | QDir::Filter::NoDotAndDotDot;

ToolButtonFileSystemTypeFilter::ToolButtonFileSystemTypeFilter(QWidget* parent)
    : QToolButton(parent),
      m_flagWhenFilterEnabled{PreferenceSettings().value("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(DEFAULT_FILTER_FLAG)).toInt()},
      m_isIncludingSubdirectory{PreferenceSettings().value("INCLUDING_SUBDIRECTORIES", true).toBool()} {
  FILTER_SWITCH->setCheckable(true);
  FILTER_SWITCH->setChecked(PreferenceSettings().value("FILE_SYSTEM_IS_FILTER_SWITCH_ON_RESTORED", true).toBool());
  FILTER_SWITCH->setToolTip(
      "1. QFileSystemModel->setFilter()\n"
      "2. SearchModel->setFilter()");

  HIDE_ENTRIES_DONT_PASS_FILTER->setCheckable(true);
  HIDE_ENTRIES_DONT_PASS_FILTER->setChecked(PreferenceSettings().value("HIDE_ENTRIES_DONT_PASS_FILTER", true).toBool());
  HIDE_ENTRIES_DONT_PASS_FILTER->setToolTip(
      "This property holds whether files that don't pass the name filter are hidden(true) or disabled(false)\n"
      "This property is true by default.\n"
      "1. QFileSystemModel->setNameFilters()\n"
      "2. SearchModel->setFilter()");

  INCLUDING_SUBDIRECTORIES->setCheckable(true);
  INCLUDING_SUBDIRECTORIES->setChecked(m_isIncludingSubdirectory);
  INCLUDING_SUBDIRECTORIES->setToolTip(
      "List entries inside all subdirectories as well.\n"
      "1. Skip for QFileSystemModel\n"
      "2. In searchModel QDirIterator(flags=QDirIterator::IteratorFlag::Subdirectories)");

  m_FILTER_FLAG_AGS->addAction(FILES);
  m_FILTER_FLAG_AGS->addAction(DIRS);
  m_FILTER_FLAG_AGS->addAction(DRIVES);
  m_FILTER_FLAG_AGS->addAction(HIDDEN);
  m_FILTER_FLAG_AGS->addAction(NO_DOT);
  m_FILTER_FLAG_AGS->addAction(NO_DOT_DOT);
  m_FILTER_FLAG_AGS->addAction(CASE_SENSITIVE);
  m_FILTER_FLAG_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  fileTypeFilterMenu->addActions(m_FILTER_FLAG_AGS->actions());
  fileTypeFilterMenu->addSeparator();
  fileTypeFilterMenu->addAction(HIDE_ENTRIES_DONT_PASS_FILTER);
  fileTypeFilterMenu->addSeparator();
  fileTypeFilterMenu->addAction(INCLUDING_SUBDIRECTORIES);

  fileTypeFilterMenu->setToolTipsVisible(true);

  qDebug() << "Stored switch on: " << FILTER_SWITCH->isChecked() << ", flags:" << m_flagWhenFilterEnabled;
  // restore Checkable / checked / Enabled;
  for (QAction* typeAct : m_FILTER_FLAG_AGS->actions()) {
    typeAct->setCheckable(true);
    typeAct->setChecked(m_text2FilterFlag[typeAct->text()] & m_flagWhenFilterEnabled);
  }
  setDefaultAction(FILTER_SWITCH);
  setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
  setMenu(fileTypeFilterMenu);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
}

void ToolButtonFileSystemTypeFilter::BindFileSystemModel(QAbstractItemModel* newModel) {
  if (_model != nullptr) {
    qWarning("skip. model is already not nullptr. don't rebind");
    return;
  }
  if (newModel == nullptr) {
    qWarning("skip. don't try to bind a nullptr");
    return;
  }
  _model = _fsmModel = dynamic_cast<QFileSystemModel*>(newModel);
  if (_fsmModel != nullptr) {
    m_modelType = MODEL_TYPE::FILE_SYSTEM_MODEL;
  } else {
    _model = _searchModel = dynamic_cast<MySearchModel*>(newModel);
    if (_searchModel != nullptr) {
      m_modelType = MODEL_TYPE::SEARCH_MODEL;
    } else {
      qDebug("BindFileSystemModel failed. _model cannot converted QFileSystemModel* or MySearchModel*");
      return;
    }
  }
  if (FILTER_SWITCH->isChecked()) {
    initFilterAgent(m_flagWhenFilterEnabled);
  } else {
    initFilterAgent(DEFAULT_FILTER_FLAG);
  }
  initNameFilterDisablesAgent(HIDE_ENTRIES_DONT_PASS_FILTER->isChecked());

  connect(fileTypeFilterMenu, &QMenu::triggered, this, &ToolButtonFileSystemTypeFilter::onTypeChecked);
  connect(FILTER_SWITCH, &QAction::triggered, this, &ToolButtonFileSystemTypeFilter::onSwitchChanged);
  connect(HIDE_ENTRIES_DONT_PASS_FILTER, &QAction::triggered, this, &ToolButtonFileSystemTypeFilter::onGrayOrHideChanged);
  connect(INCLUDING_SUBDIRECTORIES, &QAction::triggered, this, &ToolButtonFileSystemTypeFilter::changeSearchModelIteratorFlagAgent);
}

void ToolButtonFileSystemTypeFilter::onSwitchChanged(bool isOn) {
  PreferenceSettings().setValue("FILE_SYSTEM_IS_FILTER_SWITCH_ON_RESTORED", isOn);
  if (m_flagWhenFilterEnabled == INVALID_MODEL) {
    return;
  }
  if (not isOn) {
    m_flagWhenFilterEnabled = filterAgent();
    setFilterAgent(DEFAULT_FILTER_FLAG);
  } else {  // recover last flag
    setFilterAgent(m_flagWhenFilterEnabled);
  }
  qDebug() << "Save switch on: " << isOn << ". Keep flags:" << m_flagWhenFilterEnabled;
}

void ToolButtonFileSystemTypeFilter::onGrayOrHideChanged(bool isGray) {
  PreferenceSettings().setValue("HIDE_ENTRIES_DONT_PASS_FILTER", isGray);
  if (m_flagWhenFilterEnabled == INVALID_MODEL) {
    return;
  }
  setNameFilterDisablesAgent(isGray);
}

void ToolButtonFileSystemTypeFilter::onTypeChecked(QAction* act) {
  if (act->isChecked()) {
    m_flagWhenFilterEnabled |= m_text2FilterFlag[act->text()];
  } else {
    m_flagWhenFilterEnabled &= ~m_text2FilterFlag[act->text()];
  }
  PreferenceSettings().setValue("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(m_flagWhenFilterEnabled));
  if (FILTER_SWITCH->isChecked()) {  // take into effect only switch is on. Record changes even switch is off
    setFilterAgent(m_flagWhenFilterEnabled);
  }
  qDebug() << "Keep switch on: " << FILTER_SWITCH->isChecked() << ". Save flags:" << m_flagWhenFilterEnabled;
}

QDir::Filters ToolButtonFileSystemTypeFilter::filterAgent() const {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      return _fsmModel->filter();
    case SEARCH_MODEL:
      return _searchModel->filter();
    default:
      return QDir::Filter::NoFilter;
  }
}

void ToolButtonFileSystemTypeFilter::setFilterAgent(QDir::Filters filters) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      _fsmModel->setFilter(filters);
      return;
    case SEARCH_MODEL:
      _searchModel->setFilter(filters);
      return;
    default:
      return;
  }
}

void ToolButtonFileSystemTypeFilter::initFilterAgent(QDir::Filters filters) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      // Todo: may cause double load
      _fsmModel->setFilter(filters);
      return;
    case SEARCH_MODEL:
      _searchModel->initFilter(filters);
      return;
    default:
      return;
  }
}

void ToolButtonFileSystemTypeFilter::initNameFilterDisablesAgent(bool enable) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      _fsmModel->setNameFilterDisables(enable);
      return;
    case SEARCH_MODEL:
      _searchModel->initNameFilterDisables(enable);
      return;
    default:
      return;
  }
}

void ToolButtonFileSystemTypeFilter::setNameFilterDisablesAgent(bool enable) {
  switch (m_modelType) {
    case FILE_SYSTEM_MODEL:
      _fsmModel->setNameFilterDisables(enable);
      return;
    case SEARCH_MODEL:
      _searchModel->setNameFilterDisables(enable);
      return;
    default:
      return;
  }
}

void ToolButtonFileSystemTypeFilter::initSearchModelIteratorFlagAgent() {
  if (m_modelType != SEARCH_MODEL) {
    return;
  }
  auto iteratorFlag = m_isIncludingSubdirectory ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags;
  _searchModel->initIteratorFlag(iteratorFlag);  // only change value not set
}

void ToolButtonFileSystemTypeFilter::changeSearchModelIteratorFlagAgent(const bool including) {
  PreferenceSettings().setValue("INCLUDING_SUBDIRECTORIES", including);
  m_isIncludingSubdirectory = including;
  if (m_modelType != SEARCH_MODEL) {
    return;
  }
  auto iteratorFlag = m_isIncludingSubdirectory ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags;
  _searchModel->setIteratorFlag(iteratorFlag);  // change value and set
}

#include <QTableView>
class FileSystemFilter : public QSplitter {
 public:
  FileSystemFilter(QWidget* parent = nullptr) : QSplitter(parent) {
    auto* m_tv = new QTableView(this);
    auto* m_model = new QFileSystemModel(this);
    m_model->setNameFilterDisables(false);
    m_tv->setModel(m_model);
    m_tv->setRootIndex(m_model->setRootPath("./"));

    auto* fileTypeFilterButton = new ToolButtonFileSystemTypeFilter;
    fileTypeFilterButton->BindFileSystemModel(m_model);

    addWidget(m_tv);
    addWidget(fileTypeFilterButton);
  }

  QSize sizeHint() const { return QSize(1024, 768); }
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  FileSystemFilter fsf;
  fsf.show();
  a.exec();
  return 0;
}
#endif
