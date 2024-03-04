#include "ToolButtonFileSystemTypeFilter.h"
#include "PublicVariable.h"

#include <QDebug>
#include <QSplitter>

const QDir::Filters ToolButtonFileSystemTypeFilter::DEFAULT_FILTER_FLAG =
    QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Drives | QDir::Filter::NoDotAndDotDot;

ToolButtonFileSystemTypeFilter::ToolButtonFileSystemTypeFilter(QWidget* parent)
    : QToolButton(parent),
      m_flagWhenFilterEnabled{PreferenceSettings().value("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(DEFAULT_FILTER_FLAG)).toInt()} {
  FILTER_SWITCH->setCheckable(true);
  FILTER_SWITCH->setChecked(PreferenceSettings().value("FILE_SYSTEM_IS_FILTER_SWITCH_ON_RESTORED", true).toBool());

  qDebug() << PreferenceSettings().value("FILE_SYSTEM_GRAY_IF_FILTERED", false).toBool();
  GRAY_IF_FILTERED->setCheckable(true);
  GRAY_IF_FILTERED->setChecked(true);
  GRAY_IF_FILTERED->setToolTip("Gray items meet rule when enabled. hide when Disabled");

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
  fileTypeFilterMenu->addAction(GRAY_IF_FILTERED);
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

void ToolButtonFileSystemTypeFilter::BindFileSystemModel(QFileSystemModel* fsm) {
  if (_model != nullptr) {
    qWarning("skip. model already not nullptr. don't rebind");
    return;
  }
  _model = fsm;
  if (FILTER_SWITCH->isChecked()) {
    _model->setFilter(m_flagWhenFilterEnabled);
  } else {
    _model->setFilter(DEFAULT_FILTER_FLAG);
  }
  _model->setNameFilterDisables(GRAY_IF_FILTERED->isChecked());
  connect(fileTypeFilterMenu, &QMenu::triggered, this, &ToolButtonFileSystemTypeFilter::onTypeChecked);
  connect(FILTER_SWITCH, &QAction::triggered, this, &ToolButtonFileSystemTypeFilter::onSwitchChanged);
  connect(GRAY_IF_FILTERED, &QAction::triggered, this, &ToolButtonFileSystemTypeFilter::onGrayOrHideChanged);
}

void ToolButtonFileSystemTypeFilter::onSwitchChanged(bool isOn){
  PreferenceSettings().setValue("FILE_SYSTEM_IS_FILTER_SWITCH_ON_RESTORED", isOn);
  if (_model == nullptr){
    return;
  }
  if (not isOn) {
    m_flagWhenFilterEnabled = _model->filter();
    _model->setFilter(DEFAULT_FILTER_FLAG);
  } else {  // recover last flag
    _model->setFilter(m_flagWhenFilterEnabled);
  }
  qDebug() << "Save switch on: " << isOn << ". Keep flags:" << m_flagWhenFilterEnabled;
}

void ToolButtonFileSystemTypeFilter::onGrayOrHideChanged(bool isGray){
  PreferenceSettings().setValue("FILE_SYSTEM_GRAY_IF_FILTERED", isGray);
  if (_model != nullptr) {
    _model->setNameFilterDisables(isGray);
  }
}

void ToolButtonFileSystemTypeFilter::onTypeChecked(QAction* act) {
  if (act->isChecked()) {
    m_flagWhenFilterEnabled |= m_text2FilterFlag[act->text()];
  } else {
    m_flagWhenFilterEnabled &= ~m_text2FilterFlag[act->text()];
  }
  PreferenceSettings().setValue("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(m_flagWhenFilterEnabled));
  if (FILTER_SWITCH->isChecked()) {  // take into effect only switch is on. Record changes even switch is off
    _model->setFilter(m_flagWhenFilterEnabled);
  }
  qDebug() << "Keep switch on: " << FILTER_SWITCH->isChecked() << ". Save flags:" << m_flagWhenFilterEnabled;
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
