#ifndef MODELFILTERACTIONS_H
#define MODELFILTERACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QDir>
#include <QDirIterator>

enum class ModelFilterE{
  FILE_SYSTEM,
  ADVANCE_SEARCH,
  BUTT,
};

struct ModelFilterActions: public QObject {
  static ModelFilterActions& GetInst(ModelFilterE modelE);

  QAction* FILES                           {nullptr};
  QAction* DIRS                            {nullptr};
  QAction* DRIVES                          {nullptr};
  QAction* HIDDEN                          {nullptr};
  QAction* NO_DOT                          {nullptr};
  QAction* NO_DOT_DOT                      {nullptr};
  QAction* CASE_SENSITIVE                  {nullptr};
  QActionGroup* m_FILTER_FLAG_AGS          {nullptr};
  QAction* RESET_FILTERS                   {nullptr};
  QDir::Filters getCurDirFilters() const;

  QAction* GRAY_ENTRIES_DONT_PASS_FILTER   {nullptr};

  QAction* INCLUDING_SUBDIRECTORIES        {nullptr};
  QDirIterator::IteratorFlag getCurIteratorFlag() const;

  void setActionCheckedOrNot(int flagsInt);
  void resetFiltersToInitial();

  const char* c_str() const {
    return mMODEL_FILTER_2_STR[(int)mModelE];
  }
  ModelFilterActions(ModelFilterE modelE, QObject* parent = nullptr);
  QString GetMemoryKeyName(const QString& keyBaseName) const;
  ModelFilterE GetModelE() const {return mModelE;}
private:
  ModelFilterE mModelE;
  const char* mMODEL_FILTER_2_STR[(int)ModelFilterE::BUTT];
  QHash<QAction*, QDir::Filter> mFilterAct2FilterFlag;

  bool clearAllDirFilterFlags();
};

#endif // MODELFILTERACTIONS_H
