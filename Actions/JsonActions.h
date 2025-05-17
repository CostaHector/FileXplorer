#ifndef JsonActions_H
#define JsonActions_H

#include <QAction>
#include <QObject>
#include <QToolBar>

class JsonActions : public QObject {
 public:
  explicit JsonActions(QObject* parent = nullptr);
  QToolBar* GetJsonRibbonToolBar();

  // **Submit**
  QAction* _SAVE_CURRENT_CHANGES{nullptr};

  // **Sync cache/file system**
  QAction* _SYNC_NAME_FIELD_BY_FILENAME{nullptr};
  QAction* _RELOAD_JSON_FROM_FROM_DISK{nullptr};
  QAction* _EXPORT_CAST_STUDIO_TO_DICTION{nullptr};

  // **Case Control**
  QAction* _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD{nullptr};
  QAction* _LOWER_ALL_WORDS{nullptr};

  // **File Operation**
  QAction* _OPEN_THIS_FILE{nullptr};
  QAction* _REVEAL_IN_EXPLORER{nullptr};
  QAction* _RENAME_JSON_AND_RELATED_FILES{nullptr};

  // **Studio/Cast/Tags Field Operation**
  QAction* _CLEAR_CAST{nullptr};
  QAction* _CLEAR_STUDIO{nullptr};
  QAction* _CLEAR_TAGS{nullptr};

  QAction* _AI_HINT_CAST_STUDIO{nullptr};
  QAction* _FORMATTER{nullptr};

  QAction* _INIT_STUDIO_CAST{nullptr};
  QAction* _STUDIO_FIELD_SET{nullptr};

  QAction* _CAST_FIELD_SET{nullptr};
  QAction* _CAST_FIELD_APPEND{nullptr};
  QAction* _CAST_FIELD_RMV{nullptr};

  QAction* _TAGS_FIELD_SET{nullptr};
  QAction* _TAGS_FIELD_APPEND{nullptr};
  QAction* _TAGS_FIELD_RMV{nullptr};

  QAction* _ADD_SELECTED_CAST_SENTENCE{nullptr};
  QAction* _EXTRACT_UPPERCASE_CAST{nullptr};
};

JsonActions& g_JsonActions();

#endif  // JsonActions_H
