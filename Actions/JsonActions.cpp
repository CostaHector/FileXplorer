#include "JsonActions.h"

JsonActions::JsonActions(QObject* parent) //
  : QObject{parent}                       //
{
  // **Submit**
  _SAVE_CURRENT_CHANGES = new (std::nothrow) QAction{QIcon(":/JsonEditor/SAVE_CHANGES"), tr("Save"), this};
  _SAVE_CURRENT_CHANGES->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_S));
  _SAVE_CURRENT_CHANGES->setShortcutVisibleInContextMenu(true);
  _SAVE_CURRENT_CHANGES->setToolTip(QString("<b>%1 (%2)</b><br/> Commit selected row changes") //
                                        .arg(_SAVE_CURRENT_CHANGES->text(), _SAVE_CURRENT_CHANGES->shortcut().toString()));//

  // **Sync cache/file system**
  _SYNC_NAME_FIELD_FROM_FILENAME = new (std::nothrow) QAction(QIcon(":/JsonEditor/NAME_FROM_JSON_FILENAME"), tr("Set Name from File Name"));
  _SYNC_NAME_FIELD_FROM_FILENAME->setToolTip(
      QString("<b>%1 (%2)</b><br/>Set the JSON \"Name\" field to this file's base name.")
          .arg(_SYNC_NAME_FIELD_FROM_FILENAME->text(), _SYNC_NAME_FIELD_FROM_FILENAME->shortcut().toString()));
  _SYNC_FILE_NAMES_WITH_NAME_CAST = new (std::nothrow) QAction(QIcon(":/JsonEditor/FILENAME_FROM_JSON_NAME"), tr("Sync File Names from Name && Cast"));
  _SYNC_FILE_NAMES_WITH_NAME_CAST->setToolTip(
      QString("<b>%1 (%2)</b><br/>Rename this JSON and all related files by composing their names from the \"Name\" and \"Cast\" fields.") //
          .arg(_SYNC_FILE_NAMES_WITH_NAME_CAST->text(), _SYNC_FILE_NAMES_WITH_NAME_CAST->shortcut().toString()));

  _RELOAD_JSON_FROM_FROM_DISK = new (std::nothrow) QAction(QIcon(":/JsonEditor/RELOAD_FROM_DISK"), tr("Reload from disk"), this);
  _RELOAD_JSON_FROM_FROM_DISK->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F5));
  _RELOAD_JSON_FROM_FROM_DISK->setShortcutVisibleInContextMenu(true);
  _RELOAD_JSON_FROM_FROM_DISK->setToolTip(QString("<b>%1 (%2)</b><br/> Force reload json file in current path from disk again") //
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->text(), _RELOAD_JSON_FROM_FROM_DISK->shortcut().toString()));

  _EXPORT_CAST_STUDIO_TO_DICTION = new (std::nothrow) QAction(QIcon(":/JsonEditor/AI_LEARN"), tr("Export to Dictionary"), this);
  _EXPORT_CAST_STUDIO_TO_DICTION->setToolTip(
      QString(
          "<b>%1 (%2)</b><br/> Export studio/cast from field `Cast` and `Studio` in json file. \n Improve its cast/studio hint capability.")
          .arg(_EXPORT_CAST_STUDIO_TO_DICTION->text(), _EXPORT_CAST_STUDIO_TO_DICTION->shortcut().toString()));

  // **Case Control**
  _CAPITALIZE_FIRST_LETTER_IN_SELECTION = new (std::nothrow) QAction(QIcon{":/JsonEditor/CASE_TITLE"}, tr("Capitalize First Letter"), this);
  _CAPITALIZE_FIRST_LETTER_IN_SELECTION->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_U));
  _CAPITALIZE_FIRST_LETTER_IN_SELECTION->setShortcutVisibleInContextMenu(true);
  _CAPITALIZE_FIRST_LETTER_IN_SELECTION->setToolTip(QString("<b>%1 (%2)</b><br/> Capitalize the first letter of each word in the selected text; leave the remaining letters unchanged.")
                                                        .arg(_CAPITALIZE_FIRST_LETTER_IN_SELECTION->text(), _CAPITALIZE_FIRST_LETTER_IN_SELECTION->shortcut().toString()));

  _LOWER_ALL_LETTERS_IN_SELECTION = new (std::nothrow) QAction(QIcon(":img/RENAME_LOWER_CASE"), tr("Lowercase All Letters"), this);
  _LOWER_ALL_LETTERS_IN_SELECTION->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L));
  _LOWER_ALL_LETTERS_IN_SELECTION->setShortcutVisibleInContextMenu(true);
  _LOWER_ALL_LETTERS_IN_SELECTION->setToolTip(QString("<b>%1 (%2)</b><br/> Convert the selected text to lowercase.") //
                                   .arg(_LOWER_ALL_LETTERS_IN_SELECTION->text(), _LOWER_ALL_LETTERS_IN_SELECTION->shortcut().toString()));

  // **Studio/Cast/Tags Field Operation**
  _INIT_STUDIO_CAST_FIELD = new (std::nothrow) QAction(QIcon(":/JsonEditor/CAST"), tr("Init Cast/Studio"), this);
  _INIT_STUDIO_CAST_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/>Init Cast/Studio Fields if empty.") //
                                    .arg(_INIT_STUDIO_CAST_FIELD->text(), _INIT_STUDIO_CAST_FIELD->shortcut().toString()));
  _COMPOSE_NAME_WITH_CAST = new (std::nothrow) QAction(QIcon(":/JsonEditor/AI_IDEA"), tr("Compose Name with Cast"), this);
  _COMPOSE_NAME_WITH_CAST->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_N));
  _COMPOSE_NAME_WITH_CAST->setToolTip(QString("<b>%1 (%2)</b><br/> Compose the \"Name\" field with \"Cast\" field as: Name - Cast1, Cast2, ...") //
                                     .arg(_COMPOSE_NAME_WITH_CAST->text(), _COMPOSE_NAME_WITH_CAST->shortcut().toString()));

  _EXTRACT_CAST_STUDIO = new (std::nothrow) QAction(QIcon(":/JsonEditor/AI_IDEA"), tr("Extract Cast && Studio"), this);
  _EXTRACT_CAST_STUDIO->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
  _EXTRACT_CAST_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/> Infer the Cast list and Studio name from the selected text and the current \"Name\" field.") //
                                       .arg(_EXTRACT_CAST_STUDIO->text(), _EXTRACT_CAST_STUDIO->shortcut().toString()));

  _SORT_DEDUP_CAST = new (std::nothrow) QAction(QIcon(":/JsonEditor/FORMAT_PAINTER"), tr("Sort && Dedupe Cast"), this);
  _SORT_DEDUP_CAST->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_I));
  _SORT_DEDUP_CAST->setShortcutVisibleInContextMenu(true);
  _SORT_DEDUP_CAST->setToolTip(QString("<b>%1 (%2)</b><br/> Sort the \"Cast\" field and remove duplicates.<br/>e.g., A,B,A -> A, B")
                             .arg(_SORT_DEDUP_CAST->text(), _SORT_DEDUP_CAST->shortcut().toString()));

  _SET_CONTENTS_FIXED = new (std::nothrow) QAction(QIcon(":/JsonEditor/ANCHOR_DROP"), tr("Mark Contents Fixed"), this);
  _SET_CONTENTS_FIXED->setToolTip(QString("<b>%1 (%2)</b><br/> Mark selection record(s) fixed and will no longer changed when request hint on studio/cast field") //
                                      .arg(_SET_CONTENTS_FIXED->text(), _SET_CONTENTS_FIXED->shortcut().toString()));
  _SET_CONTENTS_UNFIXED = new (std::nothrow) QAction(QIcon(":/JsonEditor/ANCHOR_WEIGH"), tr("Unmark Contents Fixed"), this);
  _SET_CONTENTS_UNFIXED->setToolTip(QString("<b>%1 (%2)</b><br/> Unmark selection record(s) fixed and will changed when request hint on studio/cast field") //
                                      .arg(_SET_CONTENTS_UNFIXED->text(), _SET_CONTENTS_UNFIXED->shortcut().toString()));


  _UPDATE_DURATION_FIELD = new (std::nothrow) QAction{QIcon{":img/VIDEO_DURATION"}, tr("Update duration"), this};
  _UPDATE_DURATION_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/> Read the duration information from video file and write into json file")
                                         .arg(_UPDATE_DURATION_FIELD->text(), _UPDATE_DURATION_FIELD->shortcut().toString()));
  _UPDATE_SIZE_FIELD = new (std::nothrow) QAction{QIcon{":img/FILE_SIZE"}, tr("Update size"), this};
  _UPDATE_SIZE_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/> Read the file size information from video file and write into json file") //
                                     .arg(_UPDATE_SIZE_FIELD->text(), _UPDATE_SIZE_FIELD->shortcut().toString()));
  _UPDATE_MD5_FIELD = new (std::nothrow) QAction{QIcon{":img/MD5_FILE_IDENTIFIER_PATH"}, tr("Update MD5"), this};
  _UPDATE_MD5_FIELD->setToolTip(QString("<b>%1 (%2)</b><br/> Read the hash information from video file and write into json file") //
                                     .arg(_UPDATE_MD5_FIELD->text(), _UPDATE_MD5_FIELD->shortcut().toString()));

  _UPDATE_JSON_KEY_VALUE_PAIR = new (std::nothrow) QAction{QIcon{":img/UPDATE_JSON"}, tr("Update K-V"), this};
  _UPDATE_JSON_KEY_VALUE_PAIR->setToolTip(QString("<b>%1 (%2)</b><br/> Update the key value of \"Name/VidName/ImgName/Size\"") //
                                    .arg(_UPDATE_JSON_KEY_VALUE_PAIR->text(), _UPDATE_JSON_KEY_VALUE_PAIR->shortcut().toString()));

  _STUDIO_FIELD_SET = new (std::nothrow) QAction(QIcon(":/JsonEditor/STUDIO"), tr("Set Studio"));
  _STUDIO_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input studio string and used to set Studio field") //
                                    .arg(_STUDIO_FIELD_SET->text(), _STUDIO_FIELD_SET->shortcut().toString()));
  _CLEAR_STUDIO = new (std::nothrow) QAction(QIcon(":/JsonEditor/STUDIO_CLEAR"), tr("Clear Studio"), this);
  _CLEAR_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Studio in selected rows.") //
                                .arg(_CLEAR_STUDIO->text(), _CLEAR_STUDIO->shortcut().toString()));
  m_studioOperationList.push_back(_STUDIO_FIELD_SET);
  m_studioOperationList.push_back(_CLEAR_STUDIO);

  _CAST_FIELD_SET = new (std::nothrow) QAction(QIcon(":img/CAST_LIST_FILE"), tr("Set Cast"), this);
  _CAST_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence used to set Cast field") //
                                  .arg(_CAST_FIELD_SET->text(), _CAST_FIELD_SET->shortcut().toString()));
  _CAST_FIELD_APPEND = new (std::nothrow) QAction(QIcon(":/JsonEditor/CAST_APPEND_INPUT"), tr("Add Cast"), this);
  _CAST_FIELD_APPEND->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence append to Cast field") //
                                     .arg(_CAST_FIELD_APPEND->text(), _CAST_FIELD_APPEND->shortcut().toString()));
  _CAST_FIELD_RMV = new (std::nothrow) QAction(QIcon(":/JsonEditor/CAST_REMOVE"), tr("Rmv Cast"), this);
  _CAST_FIELD_RMV->setToolTip(QString("<b>%1 (%2)</b><br/>Input a cast used to remove from Cast field") //
                                  .arg(_CAST_FIELD_RMV->text(), _CAST_FIELD_RMV->shortcut().toString()));
  _CLEAR_CAST = new (std::nothrow) QAction(QIcon(":/JsonEditor/CAST_CLEAR"), tr("Clear Cast"), this);
  _CLEAR_CAST->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Cast in selected rows.") //
                              .arg(_CLEAR_CAST->text(), _CLEAR_CAST->shortcut().toString()));
  m_castOperationList.push_back(_CAST_FIELD_SET);
  m_castOperationList.push_back(_CAST_FIELD_APPEND);
  m_castOperationList.push_back(_CAST_FIELD_RMV);
  m_castOperationList.push_back(_CLEAR_CAST);

  _TAGS_FIELD_SET = new (std::nothrow) QAction(QIcon(":/JsonEditor/TAGS_SET"), tr("Set Tags"), this);
  _TAGS_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence used to set Tags field") //
                                  .arg(_TAGS_FIELD_SET->text(), _TAGS_FIELD_SET->shortcut().toString()));
  _TAGS_FIELD_APPEND = new (std::nothrow) QAction(QIcon(":/JsonEditor/TAGS_APPEND"), tr("Add Tags"), this);
  _TAGS_FIELD_APPEND->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence append to Tags field") //
                                     .arg(_TAGS_FIELD_APPEND->text(), _TAGS_FIELD_APPEND->shortcut().toString()));
  _TAGS_FIELD_RMV = new (std::nothrow) QAction(QIcon(":/JsonEditor/TAGS_REMOVE"), tr("Rmv Tag"), this);
  _TAGS_FIELD_RMV->setToolTip(QString("<b>%1 (%2)</b><br/>Input a tag used to remove from Tags field") //
                                  .arg(_TAGS_FIELD_RMV->text(), _TAGS_FIELD_RMV->shortcut().toString()));
  _CLEAR_TAGS = new (std::nothrow) QAction(QIcon(":/JsonEditor/TAGS_CLEAR"), tr("Clear Tags"), this);
  _CLEAR_TAGS->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Tags in selected rows.") //
                              .arg(_CLEAR_TAGS->text(), _CLEAR_TAGS->shortcut().toString()));
  m_tagsOperationList.push_back(_TAGS_FIELD_SET);
  m_tagsOperationList.push_back(_TAGS_FIELD_APPEND);
  m_tagsOperationList.push_back(_TAGS_FIELD_RMV);
  m_tagsOperationList.push_back(_CLEAR_TAGS);

  _INFER_CAST_FROM_SELECTION = new (std::nothrow) QAction(QIcon(":/JsonEditor/CAST_APPEND_FROM_SENTENCE"), tr("Infer cast from selection"), this);
  _INFER_CAST_FROM_SELECTION->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_D));
  _INFER_CAST_FROM_SELECTION->setShortcutVisibleInContextMenu(true);
  _INFER_CAST_FROM_SELECTION->setToolTip(QString("<b>%1 (%2)</b><br/> Extract Cast from selected sentence") //
                                              .arg(_INFER_CAST_FROM_SELECTION->text(), _INFER_CAST_FROM_SELECTION->shortcut().toString()));
  _INFER_CAST_FROM_UPPERCASE_SELECTION = new (std::nothrow)
      QAction(QIcon(":/JsonEditor/CAST_APPEND_FROM_UPPERCASE_SENTENCE"), tr("Infer cast from upper selection"), this);
  _INFER_CAST_FROM_UPPERCASE_SELECTION->setToolTip(QString("<b>%1 (%2)</b><br/> Extract Cast from selected UPPERCASE sentence") //
                                          .arg(_INFER_CAST_FROM_UPPERCASE_SELECTION->text(), _INFER_CAST_FROM_UPPERCASE_SELECTION->shortcut().toString()));

  _CHECK_SAMPLEMD5_AND_VIDNAME_CONSISTENCY = new (std::nothrow) QAction(QIcon(":/JsonEditor/CHECK_SAMPLEMD5_AND_VIDNAME_CONSISTENCT"), tr("Check MD5 & VidName Consistency"), this);
  _CHECK_SAMPLEMD5_AND_VIDNAME_CONSISTENCY->setToolTip("List JSON files where SampleMD5 and VidName are inconsistent.\n"
                                                       "Valid states: both fields empty, or both fields non‑empty.");
}
