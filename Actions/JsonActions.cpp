#include "JsonActions.h"
#include <QApplication>
#include <QStyle>

JsonActions::JsonActions(QObject* parent) //
  : QObject{parent}                       //
{
  // **Submit**
  _SAVE_CURRENT_CHANGES = new (std::nothrow) QAction{QIcon(":img/SAVE_CHANGES"), tr("Save"), this};
  _SAVE_CURRENT_CHANGES->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_S));
  _SAVE_CURRENT_CHANGES->setShortcutVisibleInContextMenu(true);
  _SAVE_CURRENT_CHANGES->setToolTip(QString("<b>%1 (%2)</b><br/> Commit selected row changes") //
                                        .arg(_SAVE_CURRENT_CHANGES->text())                    //
                                        .arg(_SAVE_CURRENT_CHANGES->shortcut().toString()));

  // **Sync cache/file system**
  _SYNC_NAME_FIELD_BY_FILENAME = new (std::nothrow) QAction(QIcon(":img/NAME_FROM_JSON_FILENAME"), tr("Sync Json Name Value"));
  _SYNC_NAME_FIELD_BY_FILENAME->setToolTip(
      QString("<b>%1 (%2)</b><br/>Sync Name Field from json file base name.") //
          .arg(_SYNC_NAME_FIELD_BY_FILENAME->text(), _SYNC_NAME_FIELD_BY_FILENAME->shortcut().toString()));

  _RELOAD_JSON_FROM_FROM_DISK = new (std::nothrow) QAction(QIcon(":img/RELOAD_FROM_DISK"), tr("Reload from disk"), this);
  _RELOAD_JSON_FROM_FROM_DISK->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F5));
  _RELOAD_JSON_FROM_FROM_DISK->setShortcutVisibleInContextMenu(true);
  _RELOAD_JSON_FROM_FROM_DISK->setToolTip(QString("<b>%1 (%2)</b><br/> Reload json file from disk") //
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->text())             //
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->shortcut().toString()));

  _EXPORT_CAST_STUDIO_TO_DICTION = new (std::nothrow) QAction(QIcon(":img/AI_LEARN"), tr("Export to Dictionary"), this);
  _EXPORT_CAST_STUDIO_TO_DICTION->setToolTip(
      QString(
          "<b>%1 (%2)</b><br/> Export studio/cast from field `Cast` and `Studio` in json file. \n Improve its cast/studio hint capability.")
          .arg(_EXPORT_CAST_STUDIO_TO_DICTION->text())
          .arg(_EXPORT_CAST_STUDIO_TO_DICTION->shortcut().toString()));

  // **Case Control**
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD = new (std::nothrow) QAction(QIcon{":img/CASE_TITLE"}, tr("Title"), this);
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_U));
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setShortcutVisibleInContextMenu(true);
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setToolTip(QString("<b>%1 (%2)</b><br/> Capitalize first letter of each word in a sentence.")
                                                        .arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->text())
                                                        .arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->shortcut().toString()));

  _LOWER_ALL_WORDS = new (std::nothrow) QAction(QIcon(":img/RENAME_LOWER_CASE"), tr("Lowercase"), this);
  _LOWER_ALL_WORDS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L));
  _LOWER_ALL_WORDS->setShortcutVisibleInContextMenu(true);
  _LOWER_ALL_WORDS->setToolTip(QString("<b>%1 (%2)</b><br/> Lowercase a sentence.") //
                                   .arg(_LOWER_ALL_WORDS->text())                   //
                                   .arg(_LOWER_ALL_WORDS->shortcut().toString()));

  // **File Operation**
  _OPEN_THIS_FILE = new (std::nothrow) QAction{QIcon{":img/OPEN_JSON_FILE"}, tr("Open this json file"), this};
  _OPEN_THIS_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _OPEN_THIS_FILE->setShortcutVisibleInContextMenu(true);
  _OPEN_THIS_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Open this json file in system default app.")
                                  .arg(_OPEN_THIS_FILE->text())
                                  .arg(_OPEN_THIS_FILE->shortcut().toString()));
  _REVEAL_IN_EXPLORER = new (std::nothrow) QAction{QIcon(":img/REVEAL_IN_EXPLORER"), tr("Reveal in explorer"), this};
  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::AltModifier | Qt::Key_R));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal this json in its parent folder.")
                                      .arg(_REVEAL_IN_EXPLORER->text())
                                      .arg(_REVEAL_IN_EXPLORER->shortcut().toString()));
  _RENAME_JSON_AND_RELATED_FILES = new (std::nothrow) QAction(QIcon(":img/RENAME"), tr("Rename json\n(also related files)"), this);
  _RENAME_JSON_AND_RELATED_FILES->setShortcut(QKeySequence(Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_F2));
  _RENAME_JSON_AND_RELATED_FILES->setShortcutVisibleInContextMenu(true);
  _RENAME_JSON_AND_RELATED_FILES->setToolTip(QString("<b>%1 (%2)</b><br/> Rename this json file and its related file(s)") //
                                                 .arg(_RENAME_JSON_AND_RELATED_FILES->text())
                                                 .arg(_RENAME_JSON_AND_RELATED_FILES->shortcut().toString()));

  // **Studio/Cast/Tags Field Operation**
  _INIT_STUDIO_CAST_FIELD = new (std::nothrow) QAction(QIcon(":img/CAST"), tr("Init Cast/Studio"), this);
  _INIT_STUDIO_CAST_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/>Init Cast/Studio Fields if empty.") //
                                    .arg(_INIT_STUDIO_CAST_FIELD->text(), _INIT_STUDIO_CAST_FIELD->shortcut().toString()));
  _INFER_CAST_STUDIO = new (std::nothrow) QAction(QIcon(":img/AI_IDEA"), tr("Cast/Studio Hint"), this);
  _INFER_CAST_STUDIO->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
  _INFER_CAST_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/> Give you cast/studio hint") //
                                       .arg(_INFER_CAST_STUDIO->text())
                                       .arg(_INFER_CAST_STUDIO->shortcut().toString()));

  _FORMAT_STUDIO_CAST_FIELD = new (std::nothrow) QAction(QIcon(":img/FORMAT_PAINTER"), tr("Format Cast/Studio"), this);
  _FORMAT_STUDIO_CAST_FIELD->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_I));
  _FORMAT_STUDIO_CAST_FIELD->setShortcutVisibleInContextMenu(true);
  _FORMAT_STUDIO_CAST_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/> Format Cast/Studio Field."
                                 "e.g., A,B -> A, B.")
                             .arg(_FORMAT_STUDIO_CAST_FIELD->text())
                             .arg(_FORMAT_STUDIO_CAST_FIELD->shortcut().toString()));

  _SET_CONTENTS_FIXED = new (std::nothrow) QAction(QIcon(":img/SAVED"), tr("Mark Contents Fixed"), this);
  _SET_CONTENTS_FIXED->setToolTip(QString("<b>%1 (%2)</b><br/> Mark selection record(s) fixed and will no longer changed when request hint on studio/cast field") //
                                      .arg(_SET_CONTENTS_FIXED->text())
                                      .arg(_SET_CONTENTS_FIXED->shortcut().toString()));

  _UPDATE_DURATION_FIELD = new (std::nothrow) QAction{QIcon{":img/VIDEO_DURATION"}, tr("Update duration"), this};
  _UPDATE_DURATION_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/> Read the duration information from video file and write into json file")
                                         .arg(_UPDATE_DURATION_FIELD->text())
                                         .arg(_UPDATE_DURATION_FIELD->shortcut().toString()));
  _UPDATE_SIZE_FIELD = new (std::nothrow) QAction{QIcon{":img/FILE_SIZE"}, tr("Update size"), this};
  _UPDATE_SIZE_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/> Update size field") //
                                     .arg(_UPDATE_SIZE_FIELD->text())
                                     .arg(_UPDATE_SIZE_FIELD->shortcut().toString()));
  _UPDATE_HASH_FIELD = new (std::nothrow) QAction{QIcon{":img/MD5_FILE_IDENTIFIER_PATH"}, tr("Update hash"), this};
  _UPDATE_HASH_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/> Update hash field") //
                                     .arg(_UPDATE_HASH_FIELD->text())
                                     .arg(_UPDATE_HASH_FIELD->shortcut().toString()));

  _STUDIO_FIELD_SET = new (std::nothrow) QAction(QIcon(":img/STUDIO"), tr("Set Studio"));
  _STUDIO_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input studio string and used to set Studio field") //
                                    .arg(_STUDIO_FIELD_SET->text(), _STUDIO_FIELD_SET->shortcut().toString()));
  _CLEAR_STUDIO = new (std::nothrow) QAction(QIcon(":img/STUDIO_CLEAR"), tr("Clear Studio"), this);
  _CLEAR_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Studio in selected rows.") //
                                .arg(_CLEAR_STUDIO->text(), _CLEAR_STUDIO->shortcut().toString()));
  m_studioOperationList.push_back(_STUDIO_FIELD_SET);
  m_studioOperationList.push_back(_CLEAR_STUDIO);

  _CAST_FIELD_SET = new (std::nothrow) QAction(QIcon(":img/CAST_LIST_FILE"), tr("Set Cast"), this);
  _CAST_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence used to set Cast field") //
                                  .arg(_CAST_FIELD_SET->text(), _CAST_FIELD_SET->shortcut().toString()));
  _CAST_FIELD_APPEND = new (std::nothrow) QAction(QIcon(":img/CAST_APPEND_INPUT"), tr("Add Cast"), this);
  _CAST_FIELD_APPEND->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence append to Cast field") //
                                     .arg(_CAST_FIELD_APPEND->text(), _CAST_FIELD_APPEND->shortcut().toString()));
  _CAST_FIELD_RMV = new (std::nothrow) QAction(QIcon(":img/CAST_REMOVE"), tr("Rmv Cast"), this);
  _CAST_FIELD_RMV->setToolTip(QString("<b>%1 (%2)</b><br/>Input a cast used to remove from Cast field") //
                                  .arg(_CAST_FIELD_RMV->text(), _CAST_FIELD_RMV->shortcut().toString()));
  _CLEAR_CAST = new (std::nothrow) QAction(QIcon(":img/CAST_CLEAR"), tr("Clear Cast"), this);
  _CLEAR_CAST->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Cast in selected rows.") //
                              .arg(_CLEAR_CAST->text(), _CLEAR_CAST->shortcut().toString()));
  m_castOperationList.push_back(_CAST_FIELD_SET);
  m_castOperationList.push_back(_CAST_FIELD_APPEND);
  m_castOperationList.push_back(_CAST_FIELD_RMV);
  m_castOperationList.push_back(_CLEAR_CAST);

  _TAGS_FIELD_SET = new (std::nothrow) QAction(QIcon(":img/TAGS_SET"), tr("Set Tags"), this);
  _TAGS_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence used to set Tags field") //
                                  .arg(_TAGS_FIELD_SET->text(), _TAGS_FIELD_SET->shortcut().toString()));
  _TAGS_FIELD_APPEND = new (std::nothrow) QAction(QIcon(":img/TAGS_APPEND"), tr("Add Tags"), this);
  _TAGS_FIELD_APPEND->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence append to Tags field") //
                                     .arg(_TAGS_FIELD_APPEND->text(), _TAGS_FIELD_APPEND->shortcut().toString()));
  _TAGS_FIELD_RMV = new (std::nothrow) QAction(QIcon(":img/TAGS_REMOVE"), tr("Rmv Tag"), this);
  _TAGS_FIELD_RMV->setToolTip(QString("<b>%1 (%2)</b><br/>Input a tag used to remove from Tags field") //
                                  .arg(_TAGS_FIELD_RMV->text(), _TAGS_FIELD_RMV->shortcut().toString()));
  _CLEAR_TAGS = new (std::nothrow) QAction(QIcon(":img/TAGS_CLEAR"), tr("Clear Tags"), this);
  _CLEAR_TAGS->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Tags in selected rows.") //
                              .arg(_CLEAR_TAGS->text(), _CLEAR_TAGS->shortcut().toString()));
  m_tagsOperationList.push_back(_TAGS_FIELD_SET);
  m_tagsOperationList.push_back(_TAGS_FIELD_APPEND);
  m_tagsOperationList.push_back(_TAGS_FIELD_RMV);
  m_tagsOperationList.push_back(_CLEAR_TAGS);

  _INFER_CAST_FROM_SELECTION = new (std::nothrow) QAction(QIcon(":img/CAST_APPEND_FROM_SENTENCE"), tr("Infer cast from selection"), this);
  _INFER_CAST_FROM_SELECTION->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_D));
  _INFER_CAST_FROM_SELECTION->setShortcutVisibleInContextMenu(true);
  _INFER_CAST_FROM_SELECTION->setToolTip(QString("<b>%1 (%2)</b><br/> Extract Cast from selected sentence") //
                                              .arg(_INFER_CAST_FROM_SELECTION->text())                      //
                                              .arg(_INFER_CAST_FROM_SELECTION->shortcut().toString()));
  _INFER_CAST_FROM_UPPERCASE_SELECTION = new (std::nothrow)
      QAction(QIcon(":img/CAST_APPEND_FROM_UPPERCASE_SENTENCE"), tr("Infer cast from upper selection"), this);
  _INFER_CAST_FROM_UPPERCASE_SELECTION->setToolTip(QString("<b>%1 (%2)</b><br/> Extract Cast from selected UPPERCASE sentence") //
                                          .arg(_INFER_CAST_FROM_UPPERCASE_SELECTION->text())                                    //
                                          .arg(_INFER_CAST_FROM_UPPERCASE_SELECTION->shortcut().toString()));
}

JsonActions& g_JsonActions() {
  static JsonActions ins;
  return ins;
}
