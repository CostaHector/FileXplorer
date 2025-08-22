#ifndef FILESYSTEMTYPEFILTER_H
#define FILESYSTEMTYPEFILTER_H

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFileSystemModel>
#include <QMenu>
#include <QStyle>
#include <QToolButton>
#include <QActionGroup>

#include "AdvanceSearchModel.h"
#include "SearchProxyModel.h"

class FileSystemTypeFilter : public QToolButton {
public:
  explicit FileSystemTypeFilter(QWidget* parent = nullptr);
  void BindFileSystemModel(QFileSystemModel* newModel);
  void BindFileSystemModel(AdvanceSearchModel* newModel, SearchProxyModel* newProxyModel);
  void onSwitchChanged(bool isOn);
  void onGrayOrHideChanged(bool isGray);
  void onTypeChecked(QAction* act);

  QDir::Filters filterAgent() const;
  void initFilterAgent(QDir::Filters filters);
  void setFilterAgent(QDir::Filters filters);

  void initNameFilterDisablesAgent(bool enable);
  void setNameFilterDisablesAgent(bool enable);

  void initSearchModelIteratorFlagAgent();
  void changeSearchModelIteratorFlagAgent(const bool including);

  static const QDir::Filters DEFAULT_FILTER_FLAG;

private:
  QAction* FILTER_SWITCH = new (std::nothrow) QAction{QIcon{":img/FILE_SYSTEM_FILTER"}, "Filter", this};

  QMenu* fileTypeFilterMenu{new (std::nothrow) QMenu{"Filter Menu", this}};
  QAction* FILES {new (std::nothrow) QAction{QIcon{":img/FILE"}, "Files", fileTypeFilterMenu}};
  QAction* DIRS  {new (std::nothrow) QAction{QIcon{":img/FOLDER"}, "Dirs", fileTypeFilterMenu}};
  QAction* DRIVES{new (std::nothrow) QAction{QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DriveHDIcon), "Drives", fileTypeFilterMenu}};
  QAction* HIDDEN{new (std::nothrow) QAction{QIcon{":img/HIDDEN"}, "Hidden", fileTypeFilterMenu}};
  QAction* NO_DOT{new (std::nothrow) QAction{QIcon{":img/NO_DOT"}, "NoDot", fileTypeFilterMenu}};
  QAction* NO_DOT_DOT{new (std::nothrow) QAction{QIcon{":img/NO_DOT_DOT"}, "NoDotDot", fileTypeFilterMenu}};
  QAction* CASE_SENSITIVE{new (std::nothrow) QAction{QIcon{":img/NAME_STR_CASE"}, "Case Sensitive", fileTypeFilterMenu}};
  QAction* DISABLE_ENTRIES_DONT_PASS_FILTER{new (std::nothrow) QAction{"disable the entries don't pass name filter", fileTypeFilterMenu}};
  QAction* INCLUDING_SUBDIRECTORIES{new (std::nothrow) QAction{"Including Subdirs", fileTypeFilterMenu}};

  QActionGroup* m_FILTER_FLAG_AGS{new (std::nothrow) QActionGroup{this}};

  const QHash<QString, QDir::Filter> m_text2FilterFlag {{FILES->text(), QDir::Files},
                                                       {DIRS->text(), QDir::Dirs},
                                                       {DRIVES->text(), QDir::Drives},
                                                       {HIDDEN->text(), QDir::Hidden},
                                                       {NO_DOT->text(), QDir::NoDot},
                                                       {NO_DOT_DOT->text(), QDir::NoDotDot},
                                                       {CASE_SENSITIVE->text(), QDir::CaseSensitive}};

  QFileSystemModel* _fsmModel{nullptr};
  AdvanceSearchModel* _searchSourceModel{nullptr};
  SearchProxyModel* _searchProxyModel{nullptr};

  enum MODEL_TYPE { INVALID_MODEL = 0, FILE_SYSTEM_MODEL = 1, SEARCH_MODEL = 2 };
  MODEL_TYPE m_modelType = MODEL_TYPE::INVALID_MODEL;
  QDir::Filters m_flagWhenFilterEnabled;

  bool m_isIncludingSubdirectory;
};

#endif  // FILESYSTEMTYPEFILTER_H
