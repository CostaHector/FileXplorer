#include "BrowserActions.h"
#include "PublicMacro.h"

BrowserActions::BrowserActions(QObject *parent) : QObject{parent} {
  SEARCH_CUR_TEXT = new QAction{QIcon{":img/SEARCH"}, "Search Current Text", this};
  SEARCH_CUR_TEXT->setShortcut(QKeySequence(Qt::AltModifier | Qt::Key_1));
  SEARCH_CUR_TEXT->setToolTip(QString{"<b>%1 (%2)</b><br/>Search for currently selected text in database"}//
                                  .arg(SEARCH_CUR_TEXT->text(), SEARCH_CUR_TEXT->shortcut().toString()));

  SEARCH_MULTIPLE_TEXTS = new QAction{QIcon{":img/SEARCH_MULTI_KEYWORDS"}, "Search Multiple Texts", this};
  SEARCH_MULTIPLE_TEXTS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F));
  SEARCH_MULTIPLE_TEXTS->setToolTip(QString{"<b>%1 (%2)</b><br/>Search for multiple selected texts (order insensitive) in database"}//
                                        .arg(SEARCH_MULTIPLE_TEXTS->text(), SEARCH_MULTIPLE_TEXTS->shortcut().toString()));

  ADVANCED_TEXT_SEARCH = new QAction{QIcon{":img/SEARCH_CHOICE"}, "Advanced Text Search", this};
  ADVANCED_TEXT_SEARCH->setToolTip(QString{"<b>%1 (%2)</b><br/>Provides multiple search options and allows editing WHERE clause before executing search"}
                                       .arg(ADVANCED_TEXT_SEARCH->text(), ADVANCED_TEXT_SEARCH->shortcut().toString()));

  CLEAR_ALL_SELECTIONS = new QAction{QIcon{":img/SELECT_NONE"}, "Clear All Selections", this};
  CLEAR_ALL_SELECTIONS->setShortcut(QKeySequence(Qt::AltModifier | Qt::Key_2));
  CLEAR_ALL_SELECTIONS->setToolTip(QString{"<b>%1 (%2)</b><br/>Clear all text selections in current browser"}//
                                       .arg(CLEAR_ALL_SELECTIONS->text(), CLEAR_ALL_SELECTIONS->shortcut().toString()));

  EDITOR_MODE = new QAction{QIcon{":img/EDITOR_MODE"}, "Editor Mode", this};
  EDITOR_MODE->setToolTip(QString{"<b>%1 (%2)</b><br/>Toggle editor/viewer mode"}//
                              .arg(EDITOR_MODE->text(), EDITOR_MODE->shortcut().toString()));
  EDITOR_MODE->setCheckable(true);
  EDITOR_MODE->setChecked(false);

  COPY_SELECTED_TEXT = new QAction{QIcon{":img/COPY_TEXT"}, "Copy Selected Text", this};
  COPY_SELECTED_TEXT->setShortcut(QKeySequence(Qt::AltModifier | Qt::Key_3));
}

QMenu* BrowserActions::GetSearchInDBMenu(QWidget* parent) {
  QMenu* pMenu = new (std::nothrow) QMenu{"Browser search menu", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(pMenu)
  pMenu->addAction(COPY_SELECTED_TEXT);
  pMenu->addSeparator();
  pMenu->addAction(SEARCH_CUR_TEXT);
  pMenu->addAction(SEARCH_MULTIPLE_TEXTS);
  pMenu->addAction(ADVANCED_TEXT_SEARCH);
  pMenu->setToolTipsVisible(true);
  return pMenu;
}

QToolBar* BrowserActions::GetSearchInDBToolbar(QWidget* parent) {
  QToolBar* floatingSearchTb = new (std::nothrow) QToolBar{"Browser search toolbar", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(floatingSearchTb)
  floatingSearchTb->setOrientation(Qt::Orientation::Vertical);
  floatingSearchTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  floatingSearchTb->addAction(SEARCH_CUR_TEXT);
  floatingSearchTb->addAction(SEARCH_MULTIPLE_TEXTS);
  floatingSearchTb->addAction(ADVANCED_TEXT_SEARCH);
  floatingSearchTb->addSeparator();
  floatingSearchTb->addAction(COPY_SELECTED_TEXT);
  floatingSearchTb->addAction(EDITOR_MODE);
  floatingSearchTb->addSeparator();
  floatingSearchTb->addAction(CLEAR_ALL_SELECTIONS);
  return floatingSearchTb;
}
