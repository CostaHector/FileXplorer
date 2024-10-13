#include "DuplicateVideosFinderActions.h"

#include <QLineEdit>
#include <QMenu>
#include "PublicVariable.h"
#include <QLabel>

DuplicateVideosFinderActions& g_dupVidFinderAg() {
  static DuplicateVideosFinderActions ins;
  return ins;
}

DuplicateVideosFinderActions::DuplicateVideosFinderActions(QObject* parent) : QObject{parent} {
  DIFFER_BY_DURATION->setToolTip("Value in [a-dev/2, a+dev/2) will be classified to a\nUnit: ms");
  DIFFER_BY_SIZE->setToolTip("Value in [a-dev/2, a+dev/2) will be classified to a\nUnit: Byte");

  DIFFER_BY->addAction(DIFFER_BY_DURATION);
  DIFFER_BY->addAction(DIFFER_BY_SIZE);
  DIFFER_BY->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  RECYCLE_ONE_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key_Delete));
  RECYCLE_ONE_FILE->setToolTip(QString("(%1) Can Only Record 1 item at a time").arg(RECYCLE_ONE_FILE->shortcut().toString()));

  for (auto* act : DIFFER_BY->actions()) {
    act->setCheckable(true);
  }
  DIFFER_BY_SIZE->setChecked(true);

  SCAN_A_PATH->setToolTip("Skip if table already exist. Create a table then insert records into it.");
  ANALYSE_THESE_TABLES->setToolTip("Analyse this tables");
  AUDIT_AI_MEDIA_TABLE->setToolTip("Will delete record if driver online and abspath item is no longer exist.");
  DROP_TABLE->setToolTip("Will drop a table.");
  DROP_THEN_REBUILD_THIS_TABLE->setToolTip("Will drop a table then rebuild it.");
}

QToolBar* DuplicateVideosFinderActions::GetAiMediaToolBar(QWidget* parent) {
  QToolBar* m_tb{new QToolBar{"Duplicator finder toolbar", parent}};
  m_tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  m_tb->addAction(DIFFER_BY_DURATION);
  if (durationDevLE == nullptr) {
    int durDev =
        PreferenceSettings().value(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.v).toInt();
    durationDevLE = new QLineEdit{QString::number(durDev), m_tb};
  }
  m_tb->addWidget(durationDevLE);
  m_tb->addSeparator();
  m_tb->addAction(DIFFER_BY_SIZE);
  if (sizeDevLE == nullptr) {
    int szDev =
        PreferenceSettings().value(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.v).toInt();
    sizeDevLE = new QLineEdit{QString::number(szDev), m_tb};
  }
  m_tb->addWidget(sizeDevLE);
  m_tb->addSeparator();
  m_tb->addAction(RECYCLE_ONE_FILE);
  m_tb->addSeparator();
  m_tb->addWidget(new QLabel{"Filter:", m_tb});
  if (tblKWFilter == nullptr) {
    tblKWFilter = new QLineEdit{"", m_tb};
    tblKWFilter->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  }
  m_tb->addWidget(tblKWFilter);
  m_tb->addSeparator();
  return m_tb;
}

QMenu* DuplicateVideosFinderActions::GetMenu(QWidget* parent) {
  auto* menu = new QMenu{"Ai Media Duplicate Menu", parent};
  menu->addAction(ANALYSE_THESE_TABLES);
  menu->addSeparator();
  menu->addAction(SCAN_A_PATH);
  menu->addSeparator();
  menu->addAction(AUDIT_AI_MEDIA_TABLE);
  menu->addSeparator();
  menu->addAction(DROP_TABLE);
  menu->addAction(DROP_THEN_REBUILD_THIS_TABLE);
  menu->addSeparator();
  menu->addAction(CANCEL_ANALYSE);
  return menu;
}
