#ifndef RIBBONJSON_H
#define RIBBONJSON_H

#include <QToolBar>
#include <QString>
class RibbonJson : public QToolBar {
 public:
  explicit RibbonJson(const QString& title = "Json Ribbons", QWidget* parent = nullptr);

 private:
  QToolBar* syncCacheFileSystemTB{nullptr};
  QToolBar* caseControlTB{nullptr};
  QToolBar* studioCastTagsFieldfOperationTB{nullptr};
  QToolBar* constructFieldsTB{nullptr};
  QToolBar* hintFieldsTB{nullptr};
  QToolBar* studioTB{nullptr};
  QToolBar* castEditTB{nullptr};
  QToolBar* tagsEditTB{nullptr};
  QToolBar* castFromSentenceTb{nullptr};
};

#endif  // RIBBONJSON_H
