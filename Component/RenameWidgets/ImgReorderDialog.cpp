#include "ImgReorderDialog.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "PublicMacro.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

int ImgReorderDialog::execCore(ImgReorderDialog* self) {
  CHECK_NULLPTR_RETURN_INT(self, -1);
  return self->exec();
}

ImgReorderDialog::ImgReorderDialog(QWidget* parent) : QDialog(parent) {
  m_reorderListView = new ImgReorderListView(this);
  m_reorderListView->InitListView();

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(m_reorderListView);

  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  layout->addWidget(m_buttonBox);
  if (QPushButton* pOkBtn = m_buttonBox->button(QDialogButtonBox::Ok)) {
    pOkBtn->setShortcut(Qt::Key_F10);
    pOkBtn->setToolTip(QString("<b>%1 (%2)</b><br/> Finish numerize")  //
                           .arg(pOkBtn->text(), pOkBtn->shortcut().toString()));
    pOkBtn->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);
  }

  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  ReadSettings();
  setWindowIcon(QIcon{":img/RENAME_REORDER_LISTVIEW"});
  setWindowTitle("Drag to reorder images names");
  setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
}

ImgReorderDialog::~ImgReorderDialog() {
  Configuration().setValue("IMG_REORDER_DIALOG_GEOMETRY", saveGeometry());
}

void ImgReorderDialog::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void ImgReorderDialog::ReadSettings() {
  if (Configuration().contains("IMG_REORDER_DIALOG_GEOMETRY")) {
    restoreGeometry(Configuration().value("IMG_REORDER_DIALOG_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

bool ImgReorderDialog::setImagesToReorder(const QStringList& files, const QString& baseName, int startNo, const QString& pattern) {
  return m_reorderListView->setImagesToReorder(files, baseName, startNo, pattern);
}

QStringList ImgReorderDialog::getOrderedNames() const {
  return m_reorderListView->getOrderedNames();
}
