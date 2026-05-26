#include "DialogWithSearchLine.h"
#include "PublicMacro.h"
#include "StyleSheet.h"

#include <QAction>
#include <QKeyEvent>

DialogWithSearchLine::DialogWithSearchLine(QWidget *parent, Qt::WindowFlags f)
  : QDialog{parent, f} {

  m_searchLineEdit = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_searchLineEdit);
  m_searchLineEdit->setClearButtonEnabled(true);
  m_searchLineEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_searchAction = m_searchLineEdit->addAction(QIcon(":img/FILE_SYSTEM_FILTER"), QLineEdit::LeadingPosition);

  connect(m_searchLineEdit, &QLineEdit::returnPressed, this, &DialogWithSearchLine::TriggerStartFilter);
  connect(m_searchAction, &QAction::triggered, this, &DialogWithSearchLine::TriggerStartFilter);
}

QWidget* DialogWithSearchLine::focusWidgetCore(DialogWithSearchLine* self) { // test-usage only
  CHECK_NULLPTR_RETURN_NULLPTR(self);
  return self->focusWidget();
}

void DialogWithSearchLine::keyPressEvent(QKeyEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  // Return/Enter Eater/Consumer
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    QWidget* focused = focusWidgetCore(this);
    if (focused == m_searchLineEdit) {
      event->ignore();
      return;
    }
  }
  QDialog::keyPressEvent(event);
}

void DialogWithSearchLine::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void DialogWithSearchLine::TriggerStartFilter() {
  onStartFilter(m_searchLineEdit->text());
}

