#ifndef DIALOGWITHSEARCHLINE_H
#define DIALOGWITHSEARCHLINE_H

#include <QDialog>
#include <QLineEdit>

class DialogWithSearchLine : public QDialog {
public:
  explicit DialogWithSearchLine(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

protected:
  QLineEdit* GetSearchLineEdit() { return m_searchLineEdit; }
  void keyPressEvent(QKeyEvent* event) override;
  void showEvent(QShowEvent* event) override;
  QSize sizeHint() const override {
    return QSize{1024, 768};
  }

private:
  void TriggerStartFilter();
  virtual void onStartFilter(const QString& searchText) {};

  static QWidget *focusWidgetCore(DialogWithSearchLine* self);

  QLineEdit* m_searchLineEdit{nullptr};
  QAction* m_searchAction{nullptr};
};

#endif // DIALOGWITHSEARCHLINE_H
