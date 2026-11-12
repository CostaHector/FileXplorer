#include "AccountCardDelegate.h"
#include "AccountCardEditor.h"
#include "AccountInfo.h"
#include "AccountFileHelper.h"
#include <QPainter>
#include <QPushButton>

constexpr int DELEGATE_ICON_SIZE = 64;

QSize AccountCardDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const {
  const QFontMetrics fm(option.font);
  constexpr int kLines = 7; // 7个键值对需要显示
  const int h = CARD_MARGIN * 2 + PADDING * 2 + kLines * fm.height() + (kLines - 1) * LINE_SPACING;
  return QSize(240, h);
}

void AccountCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  painter->save();

  const bool selected = option.state & QStyle::State_Selected;
  const QPalette& pal = option.palette;

  // 卡片背景 + 边框
  const QRect cardRect = option.rect.adjusted(CARD_MARGIN, CARD_MARGIN, -CARD_MARGIN, -CARD_MARGIN);
  painter->fillRect(cardRect, selected ? pal.highlight() : pal.base());
  painter->setPen(pal.mid().color());
  painter->drawRect(cardRect);

  { // Icon 右上角
    const QIcon& typeIcon = AccountFileHelper::IconForType(index.data(AccountInfo::Role::TYPE_STR_ROLE).toString().toLower());
    if (!typeIcon.isNull()) {
      const QRect iconRect(cardRect.right() - DELEGATE_ICON_SIZE - PADDING, cardRect.top() + PADDING, DELEGATE_ICON_SIZE, DELEGATE_ICON_SIZE);
      typeIcon.paint(painter, iconRect, Qt::AlignCenter, selected ? QIcon::Selected : QIcon::Normal, QIcon::Off);
    }
  }

  // Others 的换行在常态下替换为可见标记
  QString othersText = index.data(AccountInfo::Role::OTHERS_STR_ROLE).toString();
  othersText.replace('\n', QStringLiteral(" \u21B5 "));   // \n替换为可见字符"↵"

  const bool isModified{index.data(AccountInfo::Role::IS_MODIFIED_ROLE).toBool()};
  const bool isDeleted{index.data(AccountInfo::Role::IS_DELETED_ROLE).toBool()};

  const QRect contentRect{cardRect.adjusted(PADDING, PADDING, -PADDING, -PADDING)};
  const QColor labelColor{selected ? pal.highlightedText().color() : pal.mid().color()};
  const QColor valueColor{isDeleted
                              ? pal.color(QPalette::Disabled, QPalette::Text)
                              : (selected ? pal.highlightedText().color() : pal.text().color())};

  const std::array<std::pair<QString, QString>, 7> rows
      {
       std::pair<QString, QString>{"Type",     index.data(AccountInfo::Role::TYPE_STR_ROLE).toString()},
       std::pair<QString, QString>{"Name",     index.data(AccountInfo::Role::NAME_STR_ROLE).toString()},
       std::pair<QString, QString>{"Account",  index.data(AccountInfo::Role::ACCOUNT_STR_ROLE).toString()},
       std::pair<QString, QString>{"Password", index.data(AccountInfo::Role::PWD_STR_ROLE).toString()},
       std::pair<QString, QString>{"Others",   othersText},
       std::pair<QString, QString>{"Modified", isModified ? "Yes": "No"},
       std::pair<QString, QString>{"Deleted",  isDeleted ? "Yes": "No"},
       };

  const QFontMetrics fm(option.font);
  int labelWidth = 0;
  for (const auto& r : rows) {
    labelWidth = std::max(labelWidth, fm.horizontalAdvance(r.first + ":"));
  }
  labelWidth += 8;

  int y = contentRect.top();
  for (const auto& r : rows) {
    const QRect lineRect(contentRect.left(), y, contentRect.width(), fm.height());

    painter->setPen(labelColor);
    painter->drawText(QRect(lineRect.left(), lineRect.top(), labelWidth, lineRect.height()), Qt::AlignLeft | Qt::AlignVCenter, r.first + ":");

    painter->setPen(valueColor);
    const int valueWidth = lineRect.width() - labelWidth;
    const QString value = fm.elidedText(r.second, Qt::ElideRight, valueWidth);
    painter->drawText(QRect(lineRect.left() + labelWidth, lineRect.top(), valueWidth, lineRect.height()), Qt::AlignLeft | Qt::AlignVCenter, value);

    y += fm.height() + LINE_SPACING;
  }


  // isDeleted=true, 绘制一个X作为删除标记：两条对角线组成 X，覆盖整张卡片
  if (isDeleted) {
    painter->save();
    QPen deletePen(QColor(220, 40, 40, 200));   // 红色，轻微透明，不完全遮住文字
    deletePen.setWidth(2);
    painter->setPen(deletePen);
    painter->drawLine(cardRect.topLeft(), cardRect.bottomRight());
    painter->drawLine(cardRect.bottomLeft(), cardRect.topRight());
    painter->restore();
  }

  // 修改标记：左上角 "!"，强调“仅在内存中，尚未落盘”
  if (isModified) {
    static QIcon MODIFIED_SVG{":img/MODIFIED"};
    if (!MODIFIED_SVG.isNull()) {
      painter->save();
      const QRect iconRect(cardRect.left() + PADDING, cardRect.top() + PADDING, DELEGATE_ICON_SIZE, DELEGATE_ICON_SIZE);
      MODIFIED_SVG.paint(painter, iconRect, Qt::AlignCenter, selected ? QIcon::Selected : QIcon::Normal, QIcon::Off);
      painter->restore();
    }
  }

  painter->restore();
}

QWidget *AccountCardDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  auto* editor = new AccountCardEditor(parent);
  auto* self = const_cast<AccountCardDelegate*>(this);

  connect(editor->saveButton(), &QPushButton::clicked, self, [self, editor]() {
    emit self->commitData(editor);
    emit self->closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
  });

  connect(editor->cancelButton(), &QPushButton::clicked, self, [self, editor]() {
    emit self->closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
  });

  return editor;
}

void AccountCardDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  auto* e = qobject_cast<AccountCardEditor*>(editor);
  if (!e) return;
  e->typeEdit()->setText(       index.data(AccountInfo::Role::TYPE_STR_ROLE).toString());
  e->nameEdit()->setText(       index.data(AccountInfo::Role::NAME_STR_ROLE).toString());
  e->accountEdit()->setText(    index.data(AccountInfo::Role::ACCOUNT_STR_ROLE).toString());
  e->pwdEdit()->setText(        index.data(AccountInfo::Role::PWD_STR_ROLE).toString());
  e->othersEdit()->setPlainText(index.data(AccountInfo::Role::OTHERS_STR_ROLE).toString());
}

void AccountCardDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  auto* e = qobject_cast<AccountCardEditor*>(editor);
  if (!e) return;
  model->setData(index, e->typeEdit()->text(),          AccountInfo::Role::TYPE_STR_ROLE);
  model->setData(index, e->nameEdit()->text(),          AccountInfo::Role::NAME_STR_ROLE);
  model->setData(index, e->accountEdit()->text(),       AccountInfo::Role::ACCOUNT_STR_ROLE);
  model->setData(index, e->pwdEdit()->text(),           AccountInfo::Role::PWD_STR_ROLE);
  model->setData(index, e->othersEdit()->toPlainText(), AccountInfo::Role::OTHERS_STR_ROLE);
}

void AccountCardDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const {
  QRect r = option.rect.adjusted(CARD_MARGIN, CARD_MARGIN, -CARD_MARGIN, -CARD_MARGIN);
  r.setHeight(qMax(r.height(), editor->sizeHint().height()));
  editor->setGeometry(r);
}
