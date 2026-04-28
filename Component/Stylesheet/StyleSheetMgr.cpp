#include "StyleSheetMgr.h"
#include "StyleSheet.h"
#include "PreferenceActions.h"
#include "PublicMacro.h"
#include "PreferenceActions.h"
#include "ToolBarWidget.h"
#include "CustomTableView.h"
#include "CustomListView.h"
#include "StyleSheetGetter.h"
#include "Configuration.h"

#include <QMainWindow>
#include <QMenuBar>
#include <QTabWidget>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QMenu>
#include <QComboBox>
#include <QToolButton>
#include <QPushButton>
#include <QStatusBar>
#include <QDockWidget>
#include <QCheckBox>
#include <QRadioButton>

StyleSheetMgr::StyleSheetMgr(QWidget* parent)
  : QDialog{parent} {
  m_searchLineEdit = new QLineEdit{this};
  {
    m_startSearchAct = m_searchLineEdit->addAction(QIcon(":img/FILE_SYSTEM_FILTER"), QLineEdit::LeadingPosition);
    m_searchLineEdit->setClearButtonEnabled(true);
    m_searchLineEdit->setPlaceholderText("Search items here");
  }

  const bool autoInitPreviewWindowWhenStartup = Configuration().value("INIT_PREVIEW_WINDOW_WHEN_STARTUP", true).toBool();

  QToolBar* ctrlTb = new QToolBar{"Control Toolbar", this};
  ctrlTb->addWidget(m_searchLineEdit);
  m_autoInitPreviewWindow = ctrlTb->addAction("Auto Init Preview");
  m_autoInitPreviewWindow->setCheckable(true);
  m_autoInitPreviewWindow->setChecked(autoInitPreviewWindowWhenStartup);
  m_initPreviewWindow = ctrlTb->addAction("Init Preview");

  m_spiltter = new QSplitter{Qt::Orientation::Horizontal, this};
  {
    m_styleSheetView = new StyleSheetTreeView{this};
    m_styleSheetView->setMinimumWidth(600);
    m_spiltter->addWidget(m_styleSheetView);
  }
  if (autoInitPreviewWindowWhenStartup) {
    initStyleSheetPreview();
  }

  const QDialogButtonBox::StandardButtons stdBtns{QDialogButtonBox::Apply | QDialogButtonBox::Cancel};
  m_dlgBtnBox = new (std::nothrow) QDialogButtonBox{stdBtns, Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(m_dlgBtnBox);

  m_layout = new QVBoxLayout{this};
  m_layout->addWidget(ctrlTb);
  m_layout->addWidget(m_spiltter);
  m_layout->addWidget(m_dlgBtnBox);
  setLayout(m_layout);

  seeChanges();

  setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
  setWindowTitle("StyleSheet Manager");
  setWindowIcon(QIcon{":/styles/STYLESHEET_MGR"});
  subscribe();
}

StyleSheetMgr::~StyleSheetMgr() {
  Configuration().setValue("INIT_PREVIEW_WINDOW_WHEN_STARTUP", m_autoInitPreviewWindow->isChecked());
}

void StyleSheetMgr::initStyleSheetPreview() {
  if (m_effectSpiltter != nullptr) {
    return;
  }
  m_initPreviewWindow->setEnabled(false);
  m_effectSpiltter = new QSplitter{Qt::Orientation::Vertical, this};
  {
    m_effectLightPreviewer = GetEffectPreviewer();
    m_effectDarkPreviewer = GetEffectPreviewer();
    m_effectSpiltter->addWidget(m_effectLightPreviewer);
    m_effectSpiltter->addWidget(m_effectDarkPreviewer);
  }
  m_spiltter->addWidget(m_effectSpiltter);
}

void StyleSheetMgr::subscribe() {
  connect(m_searchLineEdit, &QLineEdit::returnPressed, this, &StyleSheetMgr::onStartFilter);
  connect(m_startSearchAct, &QAction::triggered, this, &StyleSheetMgr::onStartFilter);
  connect(m_initPreviewWindow, &QAction::triggered, this, &StyleSheetMgr::initStyleSheetPreview);

  auto* pApplyChanges = m_dlgBtnBox->button(QDialogButtonBox::StandardButton::Apply);
  pApplyChanges->setIcon(QIcon(":img/SAVED"));
  pApplyChanges->setShortcut(QKeySequence(Qt::Key::Key_F10));
  StyleSheet::UpdateApplyPushButton(pApplyChanges);

  auto* pAbort = m_dlgBtnBox->button(QDialogButtonBox::StandardButton::Cancel);
  pAbort->setText("Abort changes");

  connect(pApplyChanges, &QPushButton::clicked, this, &StyleSheetMgr::onApplyChanges);
  connect(pAbort, &QPushButton::clicked, this, &QDialog::reject);

  connect(m_styleSheetView, &StyleSheetTreeView::reqSeeChanges, this, &StyleSheetMgr::seeChanges);
}

void StyleSheetMgr::onApplyChanges() {
  StyleSheetGetter::GetInst().WriteIntoSettings();
  const PreferenceActions& prefInst = g_PreferenceActions();
  prefInst.initStyleTheme(false);
  accept();
}

void StyleSheetMgr::onStartFilter() {
  m_styleSheetView->setFilter(m_searchLineEdit->text());
}

void StyleSheetMgr::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QWidget::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

QWidget* StyleSheetMgr::GetEffectPreviewer() {
  QMainWindow* mainWin = new QMainWindow{this};

  QTabWidget* pRibbon = new QTabWidget{mainWin};
  mainWin->setMenuWidget(pRibbon);
  {
    QToolBar* mRibbonFile = new QToolBar{pRibbon};
    pRibbon->addTab(mRibbonFile, "File");
    mRibbonFile->setOrientation(Qt::Orientation::Horizontal);
    pRibbon->addTab(new QWidget{pRibbon}, "Other");

    QToolBar* pRibbonFileBase = new QToolBar{mRibbonFile};
    mRibbonFile->addWidget(pRibbonFileBase);
    pRibbonFileBase->setOrientation(Qt::Orientation::Vertical);
    QAction* saveAct = pRibbonFileBase->addAction("Save");
    pRibbonFileBase->addSeparator();
    QAction* closeAct = pRibbonFileBase->addAction("Close");

    mRibbonFile->addSeparator();

    QToolBar* pRibbonFileOther = new QToolBar{mRibbonFile};
    mRibbonFile->addWidget(pRibbonFileOther);
    pRibbonFileOther->setOrientation(Qt::Orientation::Horizontal);
    auto* pCheckedAct = pRibbonFileOther->addAction("Checked");
    pCheckedAct->setCheckable(true);
    pCheckedAct->setChecked(true);
    pRibbonFileOther->addSeparator();
    auto* pNotCheckedAct = pRibbonFileOther->addAction("NotChecked");
    pNotCheckedAct->setCheckable(true);
    pNotCheckedAct->setChecked(false);

    mRibbonFile->addSeparator();

    {
      QToolBar* labelLineEditToolbar = new QToolBar{mRibbonFile};
      mRibbonFile->addWidget(labelLineEditToolbar);
      labelLineEditToolbar->setOrientation(Qt::Orientation::Vertical);
      labelLineEditToolbar->addWidget(new QLabel{"Label", labelLineEditToolbar});

      QLineEdit* perPageEdit = new QLineEdit{labelLineEditToolbar};
      perPageEdit->setText("Text: 40");
      perPageEdit->setToolTip("QLineEdit in QToolBar. 40 items/per page");
      labelLineEditToolbar->addWidget(perPageEdit);
    }

    mRibbonFile->addSeparator();

    QMenu* fileMenu = new QMenu{"QMenu: file", mRibbonFile};
    {
      fileMenu->addAction(saveAct);
      fileMenu->addSeparator();
      fileMenu->addAction(closeAct);
    }
    QMenu* otherMenu = new QMenu{"QMenu: Other", mRibbonFile};
    {
      otherMenu->addAction(pCheckedAct);
      otherMenu->addAction(pNotCheckedAct);
    }

    {
      QToolButton* toolButtonWithMenu{new QToolButton{mRibbonFile}};
      mRibbonFile->addWidget(toolButtonWithMenu);
      toolButtonWithMenu->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
      toolButtonWithMenu->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
      toolButtonWithMenu->setMenu(fileMenu);
      toolButtonWithMenu->setIcon(QIcon{":img/FILE"});
      toolButtonWithMenu->setText("ToolButtonWithMenu");
    }

    mRibbonFile->addSeparator();

    {
      QMenuBar* menuBar = new QMenuBar{mRibbonFile};
      mRibbonFile->addWidget(menuBar);
      menuBar->addMenu(fileMenu);
      menuBar->addMenu(otherMenu);
    }
  }

  QStackedWidget* mainWid = new QStackedWidget{mainWin};
  {
    QWidget* mainWidWid = new QWidget{mainWin};
    mainWid->addWidget(mainWidWid);

    QVBoxLayout* mainWidWidLo = new QVBoxLayout{mainWidWid};
    {
      QStackedWidget* addressBarInEditMode = new QStackedWidget{mainWid};
      addressBarInEditMode->setMaximumHeight(30);
      QComboBox* pathCombox = new QComboBox{addressBarInEditMode};
      pathCombox->setEditable(false);
      pathCombox->setToolTip("editable QComboBox in QStackedWidget");
      pathCombox->setEditable(true);
      pathCombox->addItem("path/to/home");
      pathCombox->addItem("another/path/to/home");
      addressBarInEditMode->addWidget(pathCombox);
      mainWidWidLo->addWidget(addressBarInEditMode);
    }
    {
      QStackedWidget* addressBarInClickMode = new QStackedWidget{mainWid};
      addressBarInClickMode->setMaximumHeight(30);
      QToolBar* pathSections = new QToolBar{addressBarInClickMode};
      pathSections->addAction(QIcon{":img/FOLDER_OF_DRIVES"}, "");
      pathSections->addAction("path");
      pathSections->addAction("to");
      pathSections->addAction("home");
      pathSections->addSeparator();
      QComboBox* typeFilterCombox = new QComboBox{pathSections};
      typeFilterCombox->setEditable(false);
      typeFilterCombox->setToolTip("Not editable QComboBox in QStackedWidget/QToolBar");
      typeFilterCombox->addItem(QIcon{":img/FILE"}, "File");
      typeFilterCombox->addItem(QIcon{":img/VIDEO"}, "Video");
      typeFilterCombox->addItem(QIcon{":img/IMAGE"}, "Image");
      pathSections->addWidget(typeFilterCombox);

      QLineEdit* searchLineEdit = new QLineEdit{pathSections};
      searchLineEdit->addAction(QIcon{":img/SEARCH"}, QLineEdit::LeadingPosition);
      searchLineEdit->setPlaceholderText("Placeholder: search text here");
      searchLineEdit->setToolTip("QLineEdit in QToolBar. placeholder");
      pathSections->addWidget(searchLineEdit);

      QCheckBox* checkBox = new QCheckBox{"CheckBox", pathSections};
      pathSections->addWidget(checkBox);

      QRadioButton* radioBtn = new QRadioButton{"RadioBtn", pathSections};
      pathSections->addWidget(radioBtn);

      QPushButton* notCheckableBtn = new QPushButton{"Not Checkable PushButton", pathSections};
      notCheckableBtn->setCheckable(false);
      notCheckableBtn->setToolTip("Not checkable");
      pathSections->addWidget(notCheckableBtn);

      QPushButton* checkedBtn = new QPushButton{"Checked PushButton", pathSections};
      checkedBtn->setCheckable(true);
      checkedBtn->setChecked(true);
      checkedBtn->setToolTip("checked QPushButton");
      pathSections->addWidget(checkedBtn);

      QDialogButtonBox* dlgBtnBox = new QDialogButtonBox{QDialogButtonBox::StandardButton::Yes | QDialogButtonBox::StandardButton::No, pathSections};
      dlgBtnBox->setToolTip("QDialogButtonBox in QStackedWidget/QToolBar");
      {
        QPushButton* pOkBtn = dlgBtnBox->button(QDialogButtonBox::StandardButton::Yes);
        pOkBtn->setIcon(QIcon(":img/SAVED"));
        pOkBtn->setToolTip("QPushButton in QDialogButtonBox");
        StyleSheet::UpdateApplyPushButton(pOkBtn);
      }
      pathSections->addWidget(dlgBtnBox);

      addressBarInClickMode->addWidget(pathSections);
      mainWidWidLo->addWidget(addressBarInClickMode);
    }

    QHBoxLayout* listTableTreeLayout = new QHBoxLayout;
    mainWidWidLo->addLayout(listTableTreeLayout);

    CustomListView* pListView = new CustomListView{"ListViewInStyleSheetMgr", mainWidWid};
    pListView->InitListView();
    pListView->setViewMode(QListView::ViewMode::IconMode);
    QStandardItemModel* pStdListModel = new QStandardItemModel{3, 1, pListView};
    pStdListModel->setItem(0, 0, new QStandardItem(QIcon{":img/FILE"}, "list 0"));
    pStdListModel->setItem(1, 0, new QStandardItem(QIcon{":img/VIDEO"}, "list 1"));
    pStdListModel->setItem(2, 0, new QStandardItem(QIcon{":img/IMAGE"}, "list 2"));
    pListView->setModel(pStdListModel);
    listTableTreeLayout->addWidget(pListView);

    CustomTableView* pTableView = new CustomTableView{"TableViewInStyleSheetMgr", mainWidWid};
    pTableView->InitTableView();
    QStandardItemModel* pStdTableModel = new QStandardItemModel{2, 2, pTableView};
    pStdTableModel->setHorizontalHeaderLabels({"hor1", "hor2"});
    pStdTableModel->setVerticalHeaderLabels({"ver1", "ver2"});
    pStdTableModel->setItem(0, 0, new QStandardItem(QIcon{":img/FILE"}, "table(0,0)"));
    pStdTableModel->setItem(0, 1, new QStandardItem(QIcon{":img/VIDEO"}, "table(0,1)"));
    pStdTableModel->setItem(1, 0, new QStandardItem(QIcon{":img/IMAGE"}, "table(1,0)"));
    pStdTableModel->setItem(1, 1, new QStandardItem(QIcon{":img/PLAIN_TEXT"}, "table(1,1)"));
    pTableView->setModel(pStdTableModel);
    listTableTreeLayout->addWidget(pTableView);

    CustomTreeView* pTreeView = new CustomTreeView{"TreeViewInStyleSheetMgr", mainWidWid};
    pTreeView->InitTreeView();
    pTreeView->setModel(pStdTableModel);
    listTableTreeLayout->addWidget(pTreeView);
  }
  mainWin->setCentralWidget(mainWid);

  {
    QDockWidget* pDock = new QDockWidget{mainWin};
    auto title = new QLabel{"Navi", this};
    title->setFixedHeight(12);
    pDock->setTitleBarWidget(title);

    ToolBarWidget* tbWid = new ToolBarWidget{QBoxLayout::Direction::TopToBottom, pDock};
    tbWid->addAction(QIcon(":img/EXPAND_SIDEBAR"), "Expand");
    tbWid->addSeparator();
    tbWid->addAction(QIcon(":img/FOLDER_OF_PICTURES"), "Videos");
    pDock->setWidget(tbWid);
    pDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea);
    mainWin->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, pDock);
  }

  QStatusBar* statusBar = new QStatusBar{mainWin};
  mainWin->setStatusBar(statusBar);
  statusBar->showMessage("StatusBar Here");
  return mainWin;
}

void StyleSheetMgr::seeChanges() {
  if (m_initPreviewWindow->isEnabled()) { // need inited
    return;
  }
  const QString lightQss = StyleSheetGetter::GetInst()(Style::StyleThemeE::THEME_LIGHT);
  m_effectLightPreviewer->setStyleSheet(lightQss);
  const QString darkQss = StyleSheetGetter::GetInst()(Style::StyleThemeE::THEME_DARK_MOON_FOG);
  m_effectDarkPreviewer->setStyleSheet(darkQss);
}
