#include "QuickWhereClauseDialogMovie.h"
#include "PublicMacro.h"
#include "TableFields.h"
#include "PublicVariable.h"
#include "QuickWhereClauseHelper.h"
#include "CastAkasManager.h"

void QuickWhereClauseDialogMovie::CreatePrivateWidget() {
  using namespace MOVIE_TABLE;
  m_Name = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Name), this}; // shared
  CHECK_NULLPTR_RETURN_VOID(m_Name);
  m_Name->setToolTip( //
      "Henry Cavill&Chris Evans\n"
      "A: search 1 person\n"              //
      "A & B: search 2 person BOTH\n"     //
      "A1 | A2: search 2 person EITHER\n" //
      "(A1|A2)&B: search 2 person BOTH(One person also known as A2)");

  m_Size = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Size), this};
  CHECK_NULLPTR_RETURN_VOID(m_Size);
  m_Size->setToolTip(">1000000000&<1500000000");

  m_Duration = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Duration), this};
  CHECK_NULLPTR_RETURN_VOID(m_Duration);
  m_Duration->setToolTip(">6000 unit: ms"); // 6s

  m_Studio = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Studio), this};
  CHECK_NULLPTR_RETURN_VOID(m_Studio);
  m_Studio->setToolTip(R"(20th Century Studios|Marvel Studios)");

  m_Cast = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Cast), this};
  CHECK_NULLPTR_RETURN_VOID(m_Cast);
  m_Cast->setToolTip(R"(Henry Cavill&Chris Evans)");

  m_Tags = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Tags), this}; // shared
  CHECK_NULLPTR_RETURN_VOID(m_Tags);
  m_Tags->setToolTip(R"(Superhero|Documentary)");

  AppendColumnFilterLineEdit(m_Name);
  AppendColumnFilterLineEdit(m_Size);
  AppendColumnFilterLineEdit(m_Duration);
  AppendColumnFilterLineEdit(m_Studio);
  AppendColumnFilterLineEdit(m_Cast);
  AppendColumnFilterLineEdit(m_Tags);

  m_Name->setFocus();
  setWindowTitle(QString{"SELECT * FROM `%1` WHERE ?"}.arg(DB_TABLE::MOVIES));
}
