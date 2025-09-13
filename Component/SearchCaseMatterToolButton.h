#ifndef SEARCHCASEMATTERTOOLBUTTON_H
#define SEARCHCASEMATTERTOOLBUTTON_H

#include <QToolButton>
#include <QAction>

class SearchCaseMatterToolButton : public QToolButton {
  Q_OBJECT
public:
  explicit SearchCaseMatterToolButton(QWidget* parent = nullptr);
  Qt::CaseSensitivity curNameCaseSensitive() const {return SEARCH_NAME_CASE_SENSITIVE->isChecked() ? Qt::CaseSensitivity::CaseSensitive : Qt::CaseSensitivity::CaseInsensitive; }
  Qt::CaseSensitivity curContentCaseSensitive() const {return SEARCH_CONTENTS_CASE_SENSITIVE->isChecked() ? Qt::CaseSensitivity::CaseSensitive : Qt::CaseSensitivity::CaseInsensitive; }

signals:
  void nameCaseSensitiveChanged(Qt::CaseSensitivity newCaseSensitive);
  void contentCaseSensitiveChanged(Qt::CaseSensitivity newCaseSensitive);
private:
  QAction* SEARCH_NAME_CASE_SENSITIVE{nullptr};
  QAction* SEARCH_CONTENTS_CASE_SENSITIVE{nullptr};
};

#endif  // SEARCHCASEMATTERTOOLBUTTON_H
