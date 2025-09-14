#include "ModelFilterActions.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include <QHash>


ModelFilterActions& ModelFilterActions::GetInst(ModelFilterE modelE) {
  static ModelFilterActions insts[(int)ModelFilterE::BUTT]
      {//
          {ModelFilterE::FILE_SYSTEM, nullptr}, //
          {ModelFilterE::ADVANCE_SEARCH, nullptr}, //
          // follow enum ModelFilterE order strictly
      };
  return insts[(int)modelE];
}

ModelFilterActions::ModelFilterActions(ModelFilterE modelE, QObject* parent) //
  : QObject{parent}, //
  mModelE{modelE},//
  mMODEL_FILTER_2_STR//
  {//
      ENUM_2_STR(ModelFilterE::ADVANCE_SEARCH),//
      ENUM_2_STR(ModelFilterE::FILE_SYSTEM)//
  }//
{
  FILES                            = new (std::nothrow) QAction{QIcon{":img/FILE"}, "Files", this};
  DIRS                             = new (std::nothrow) QAction{QIcon{":img/FOLDER"}, "Dirs", this};
  DRIVES                           = new (std::nothrow) QAction{QIcon{":img/DISKS"}, "Drives", this};
  HIDDEN                           = new (std::nothrow) QAction{QIcon{":img/HIDDEN"}, "Hidden", this};
  NO_DOT                           = new (std::nothrow) QAction{QIcon{":img/NO_DOT"}, "NoDot", this};
  NO_DOT_DOT                       = new (std::nothrow) QAction{QIcon{":img/NO_DOT_DOT"}, "NoDotDot", this};
  CASE_SENSITIVE                   = new (std::nothrow) QAction{QIcon{":img/NAME_STR_CASE"}, "Case Sensitive", this};

  m_FILTER_FLAG_AGS                = new (std::nothrow) QActionGroup{this};
  m_FILTER_FLAG_AGS->addAction(FILES);
  m_FILTER_FLAG_AGS->addAction(DIRS);
  m_FILTER_FLAG_AGS->addAction(DRIVES);
  m_FILTER_FLAG_AGS->addAction(HIDDEN);
  m_FILTER_FLAG_AGS->addAction(NO_DOT);
  m_FILTER_FLAG_AGS->addAction(NO_DOT_DOT);
  m_FILTER_FLAG_AGS->addAction(CASE_SENSITIVE);
  m_FILTER_FLAG_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  RESET_FILTERS = new (std::nothrow) QAction{"Reset to initial flag", this};

  decltype(mFilterAct2FilterFlag) tempFilterAct2FilterFlag//
      {//
          {FILES, QDir::Files},
          {DIRS  , QDir::Dirs},
          {DRIVES, QDir::Drives},
          {HIDDEN, QDir::Hidden},
          {NO_DOT, QDir::NoDot},
          {NO_DOT_DOT, QDir::NoDotDot},
          {CASE_SENSITIVE, QDir::CaseSensitive}
      };
  mFilterAct2FilterFlag.swap(tempFilterAct2FilterFlag);
  const QString dirFilterKeyName = GetMemoryKeyName(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name);
  const QDir::Filters initialDirFlag{Configuration().value(dirFilterKeyName, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt()};
  LOG_D("%s filter flags: %d", qPrintable(dirFilterKeyName), (int)initialDirFlag);
  // restore Checkable / checked / Enabled;
  setActionCheckedOrNot(initialDirFlag);

  const bool bGrayOrHide{Configuration().value(GetMemoryKeyName(MemoryKey::GRAY_ENTRIES_DONT_PASS_FILTER.name), MemoryKey::GRAY_ENTRIES_DONT_PASS_FILTER.v).toBool()};
  GRAY_ENTRIES_DONT_PASS_FILTER = new (std::nothrow) QAction{"Gray the entries if don't pass name filter", this};
  GRAY_ENTRIES_DONT_PASS_FILTER->setCheckable(true);
  GRAY_ENTRIES_DONT_PASS_FILTER->setChecked(bGrayOrHide);
  GRAY_ENTRIES_DONT_PASS_FILTER->setToolTip("This property holds whether files that don't pass the name filter are disable gray(true) or hide(false). (true by default)");

  if (modelE == ModelFilterE::ADVANCE_SEARCH) {
    const bool isIncludingSubdirectory{Configuration().value(GetMemoryKeyName(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name), MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.v).toBool()};
    INCLUDING_SUBDIRECTORIES         = new (std::nothrow) QAction{"Including Subdirs", this};
    INCLUDING_SUBDIRECTORIES->setCheckable(true);
    INCLUDING_SUBDIRECTORIES->setChecked(isIncludingSubdirectory);
    INCLUDING_SUBDIRECTORIES->setToolTip("Penetrate inside items under all subdirectories as well .\n"
                                         "i.e., QDirIterator(flags=QDirIterator::IteratorFlag::Subdirectories)");
  }

}

void ModelFilterActions::setActionCheckedOrNot(int flagsInt) {
  foreach (QAction* filterAct, m_FILTER_FLAG_AGS->actions()) {
    filterAct->setCheckable(true);
    filterAct->setChecked(flagsInt & (int)mFilterAct2FilterFlag[filterAct]);
  }
}

void ModelFilterActions::resetFiltersToInitial() {
  setActionCheckedOrNot(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v.toInt());
}

QString ModelFilterActions::GetMemoryKeyName(const QString& keyBaseName) const {return keyBaseName + "/" + c_str();}

QDir::Filters ModelFilterActions::getCurDirFilters() const {
  decltype(ModelFilterActions::getCurDirFilters()) ansFilterFlags;
  for (auto it = mFilterAct2FilterFlag.cbegin(); it != mFilterAct2FilterFlag.cend(); ++it) {
    ansFilterFlags.setFlag(it.value(), it.key()->isChecked());
  }
  return ansFilterFlags;
}

bool ModelFilterActions::getCurGrayOrHideUpassItem() const  {
  return GRAY_ENTRIES_DONT_PASS_FILTER->isChecked();
}

QDirIterator::IteratorFlag ModelFilterActions::getCurIteratorFlag() const {
  if (mModelE != ModelFilterE::ADVANCE_SEARCH) {
    LOG_W("%s not support iterator flag", c_str());
    return QDirIterator::IteratorFlag::NoIteratorFlags;
  }
  if (INCLUDING_SUBDIRECTORIES == nullptr) {
    LOG_E("%s INCLUDING_SUBDIRECTORIES nullptr met", c_str());
    return QDirIterator::IteratorFlag::NoIteratorFlags;
  }
  return INCLUDING_SUBDIRECTORIES->isChecked() ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags;
}

bool ModelFilterActions::clearAllDirFilterFlags() {
  for (auto pAct: m_FILTER_FLAG_AGS->actions()) {
    CHECK_NULLPTR_RETURN_FALSE(pAct);
    pAct->setChecked(false);
  }
  return true;
}
