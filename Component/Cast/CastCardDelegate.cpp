#include "CastCardDelegate.h"
#include "CastDbModel.h"
#include "ImageTool.h"
#include "StringTool.h"
#include "VideoTierTool.h"
#include "CastDBCardEditor.h"
#include "CastPsonFileHelper.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QAbstractProxyModel>
#include <QFileInfo>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

constexpr int CastCardDelegate::CARD_W;
constexpr int CastCardDelegate::CARD_H;

namespace {
constexpr int RADIUS = 8;

// 画一个圆角"胶囊"(标签 / +n 计数)
void DrawChip(QPainter* painter, const QRect& rect, const QString& text, const QFont& font,  //
              const QColor& fg, const QColor& bg, const QColor& border) {
  const qreal r{rect.height() / 2.0};
  painter->setPen(QPen{border, 1});
  painter->setBrush(bg);
  painter->drawRoundedRect(QRectF{rect}.adjusted(0.5, 0.5, -0.5, -0.5), r, r);
  painter->setFont(font);
  painter->setPen(fg);
  painter->drawText(rect, Qt::AlignCenter, text);
}
}  // namespace

QSize CastCardDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {
  return m_cardSize;
}

void CastCardDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if (painter == nullptr) {
    return;
  }
  const CastDbModel* pModel = dynamic_cast<const CastDbModel*>(index.model());
  if (pModel == nullptr || !index.isValid()) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }
  if (option.rect.width() <= 8 || option.rect.height() <= 8) {
    return;
  }

  const bool bSelected{(option.state & QStyle::State_Selected) != 0};
  const bool bHovered{(option.state & QStyle::State_MouseOver) != 0};

  // option.rect 由 view 的 gridSize(=cardSize) 决定, 一切尺寸都由它推算, 天然随 ctrl+滚轮缩放
  const int margin{qBound(2, option.rect.width() / 24, 10)};
  const QRect cardRect{option.rect};
  const QRect faceRect{cardRect.adjusted(margin, margin, -margin, -margin)};
  if (faceRect.width() <= 8 || faceRect.height() <= 8) {
    return;
  }
  const int innerW{faceRect.width()};
  const int innerH{faceRect.height()};

  painter->save();
  painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
  painter->setClipRect(option.rect);  // 严格裁到本 item 槽位, 防止画到相邻卡片上

  QPainterPath cardPath;
  cardPath.addRoundedRect(QRectF{faceRect}, RADIUS, RADIUS);
  painter->setClipPath(cardPath, Qt::ClipOperation::IntersectClip);

  // ---------- 1. 底图 ----------
  // GetPixmapFromCached 内部等比内接缩放(细长图 scaledToWidth, 扁平图 scaledToHeight), 且自带
  // QPixmapCache。千万不要在这里写 QImage{path}, 滚动时会疯狂读盘。
  painter->fillRect(faceRect, QColor{0x22, 0x22, 0x22});
  QString imgAbsPath{pModel->coverImageFullPathAtIndex(index, true)};  // 优先缩略图
  if (!QFileInfo{imgAbsPath}.isFile()) {
    imgAbsPath = pModel->coverImageFullPathAtIndex(index, false);  // 缩略图未生成时退回首图
  }
  const QPixmap cover{ImageTool::GetPixmapFromCached(imgAbsPath, innerW, innerH, true)};
  if (!cover.isNull()) {
    const QPoint topLeft{faceRect.center() - QPoint{cover.width() / 2, cover.height() / 2}};
    painter->drawPixmap(QRect{topLeft, cover.size()}, cover);
  } else {
    QFont placeholderFont{option.font};
    placeholderFont.setPixelSize(qBound(9, innerW / 15, 26));
    painter->setFont(placeholderFont);
    painter->setPen(QColor{0x88, 0x88, 0x88});
    painter->drawText(faceRect, Qt::AlignCenter, QStringLiteral("No Image"));
  }

  // ---------- 2. 字号/度量随卡片宽度等比缩放 ----------
  QFont nameFont{option.font};
  nameFont.setPixelSize(qBound(11, innerW / 11, 34));
  nameFont.setBold(true);
  QFont metaFont{option.font};
  metaFont.setPixelSize(qBound(9, innerW / 15, 26));
  QFont tagFont{option.font};
  tagFont.setPixelSize(qBound(8, innerW / 17, 22));

  const QFontMetrics nameFm{nameFont};
  const QFontMetrics metaFm{metaFont};
  const QFontMetrics tagFm{tagFont};

  const int pad{qBound(4, innerW / 20, 12)};
  const int tagH{qBound(13, tagFont.pixelSize() + 6, 30)};
  const int tagGap{qBound(3, innerW / 40, 8)};
  const int textLeft{faceRect.left() + pad};
  const int textWidth{innerW - 2 * pad};

  // ---------- 3. 底部渐变遮罩高度: 按三行文字实算, 不再写死 ----------
  const int footerH{qMin(innerH * 55 / 100, pad + nameFm.height() + 1 + metaFm.height() + 2 + tagH + pad)};
  const QRect footerRect{faceRect.left(), faceRect.bottom() - footerH + 1, innerW, footerH};
  {
    QLinearGradient grad{footerRect.topLeft(), footerRect.bottomLeft()};
    grad.setColorAt(0.0, QColor{0, 0, 0, 0});
    grad.setColorAt(1.0, QColor{0, 0, 0, 230});
    painter->fillRect(footerRect, grad);
  }

  // ---------- 4. 评分徽章(左上角 正方形/等腰直角三角形), 配色复用 VideoTierTool, 与场景视图评分条一致 ----------
  const int rate{pModel->rateAtIndex(index)};
  {
    const int badgeSide{qBound(22, innerW / 4, 96)};
    QColor badgeColor{VideoTierTool::GetVideoTierColorRgb(VideoTierTool::MovieRate2VideoTierE(rate))};
    badgeColor.setAlpha(235);
    painter->setPen(Qt::NoPen);
    painter->setBrush(badgeColor);
    // const QRect badgeRect{faceRect.left() + pad, faceRect.top() + pad, badgeSide, badgeSide};
    const QRect badgeRect{faceRect.left(), faceRect.top(), badgeSide, badgeSide};
    {
      // painter->drawRoundedRect(QRectF{badgeRect}, RADIUS, RADIUS);
    }
    {
      const qreal triSide = badgeSide * 1.7; // in theory 2
      const QPointF origin = badgeRect.topLeft();
      QPolygonF tri{{origin, QPointF(origin.x() + triSide, origin.y()), QPointF(origin.x(), origin.y() + triSide)}};
      painter->drawPolygon(tri);
    }

    QFont badgeFont{option.font};
    badgeFont.setPixelSize(badgeSide * 2 / 3);
    badgeFont.setBold(true);
    painter->setFont(badgeFont);
    painter->setPen(Qt::white);
    painter->drawText(badgeRect, Qt::AlignCenter, QString::number(rate));
  }

  // ---------- 5. 文案区: 姓名 / 身高+尺码 / 标签, 自下而上堆叠 ----------
  int cursorBottom{faceRect.bottom() - pad};
  const QRect tagRowRect{textLeft, cursorBottom - tagH + 1, textWidth, tagH};
  cursorBottom = tagRowRect.top() - 2;
  const QRect metaRect{textLeft, cursorBottom - metaFm.height() + 1, textWidth, metaFm.height()};
  cursorBottom = metaRect.top() - 1;
  const QRect nameRect{textLeft, cursorBottom - nameFm.height() + 1, textWidth, nameFm.height()};

  // 5.1 姓名
  painter->setFont(nameFont);
  painter->setPen(Qt::white);
  painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter,
                    nameFm.elidedText(pModel->fileName(index), Qt::ElideRight, textWidth));

  // 5.2 身高 / 尺码(schema 默认 -1 表示未知; 单位均为 cm)
  const int height{pModel->heightAtIndex(index)};
  const int size{pModel->sizeAtIndex(index)};
  QStringList metaParts;
  if (height >= 0) {
    metaParts << QString::asprintf("H %d", height);
  }
  if (size >= 0) {
    metaParts << QString::asprintf("S %d", size);
  }
  const QString meta{metaParts.isEmpty() ? QStringLiteral("H -    S -") : metaParts.join(QStringLiteral("   "))};
  painter->setFont(metaFont);
  painter->setPen(QColor{0xCC, 0xCC, 0xCC});
  painter->drawText(metaRect, Qt::AlignLeft | Qt::AlignVCenter, metaFm.elidedText(meta, Qt::ElideRight, textWidth));

  // 5.3 标签胶囊(\n 分隔), 放不下就截断并补 "+n"
  const QStringList tags{pModel->tagsAtIndex(index).split(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR, Qt::SkipEmptyParts)};
  {
    const QColor chipBg{255, 255, 255, 40};
    const QColor chipBorder{255, 255, 255, 100};
    const int chipPadX{qBound(5, innerW / 25, 12)};
    int x{tagRowRect.left()};
    for (int i = 0; i < tags.size(); ++i) {
      const QString tag{tags.at(i).trimmed()};
      if (tag.isEmpty()) {
        continue;
      }
      const QString plus{QString::asprintf("+%d", tags.size() - i)};
      const int chipW{tagFm.horizontalAdvance(tag) + 2 * chipPadX};
      const bool bHasMore{tags.size() - i > 1};
      // 不是最后一个时预留 "+n" 的位置, 避免最后一个挤掉计数
      const int reserve{bHasMore ? tagFm.horizontalAdvance(plus) + 2 * chipPadX + tagGap : 0};
      if (x + chipW + reserve > tagRowRect.right() + 1) {
        const int plusW{tagFm.horizontalAdvance(plus) + 2 * chipPadX};
        if (bHasMore && x + plusW <= tagRowRect.right() + 1) {
          DrawChip(painter, QRect{x, tagRowRect.top(), plusW, tagH}, plus, tagFont, Qt::white, chipBg, chipBorder);
        }
        break;
      }
      DrawChip(painter, QRect{x, tagRowRect.top(), chipW, tagH}, tag, tagFont, Qt::white, chipBg, chipBorder);
      x += chipW + tagGap;
    }
  }

  // ---------- 6. 边框高亮(在裁切路径内画, 天然不越界) ----------
  painter->setBrush(Qt::NoBrush);
  if (bSelected) {
    painter->setPen(QPen{QColor{0xFF, 0xC8, 0x00}, 2});
    painter->drawRoundedRect(QRectF{faceRect}.adjusted(1, 1, -1, -1), RADIUS, RADIUS);
  } else if (bHovered) {
    painter->setPen(QPen{QColor{255, 255, 255, 150}, 1});
    painter->drawRoundedRect(QRectF{faceRect}.adjusted(0.5, 0.5, -0.5, -0.5), RADIUS, RADIUS);
  } else {
    painter->setPen(QPen{QColor{0, 0, 0, 90}, 1});
    painter->drawRoundedRect(QRectF{faceRect}.adjusted(0.5, 0.5, -0.5, -0.5), RADIUS, RADIUS);
  }

  painter->restore();
}

QWidget *CastCardDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
  CastDBCardEditor* editor = new CastDBCardEditor(parent);
  CastCardDelegate* self = const_cast<CastCardDelegate*>(this);

  connect(editor, &CastDBCardEditor::stageCurrentChanges, self, [self, editor]() {
    emit self->commitData(editor);
    emit self->closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
  });

  connect(editor, &CastDBCardEditor::revertCurrentChanges, self, [self, editor]() {
    emit self->closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
  });

  return editor;
}

void CastCardDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  auto* e = qobject_cast<CastDBCardEditor*>(editor);
  if (!e) return;

  e->mActorNameEdit->setText(index.siblingAtColumn(  CastDbModelField::FIELD_E::Name).data(Qt::DisplayRole).toString());
  e->mRateEdit->setValue(index.siblingAtColumn(      CastDbModelField::FIELD_E::Rate).data(Qt::DisplayRole).toInt());
  e->mAliasEdit->setText(index.siblingAtColumn(      CastDbModelField::FIELD_E::ALIAS).data(Qt::DisplayRole).toString());
  e->mTagsEdit->setText(index.siblingAtColumn(       CastDbModelField::FIELD_E::Tags).data(Qt::DisplayRole).toString());
  e->mOriBelongToEdit->setText(index.siblingAtColumn(CastDbModelField::FIELD_E::Ori).data(Qt::DisplayRole).toString());
  e->mHeightEdit->setValue(index.siblingAtColumn(    CastDbModelField::FIELD_E::Height).data(Qt::DisplayRole).toInt());
  e->mSizeEdit->setValue(index.siblingAtColumn(      CastDbModelField::FIELD_E::Size).data(Qt::DisplayRole).toInt());
  e->mDetailEdit->setPlainText(index.siblingAtColumn(CastDbModelField::FIELD_E::Detail).data(Qt::DisplayRole).toString());
}

void CastCardDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  auto* e = qobject_cast<CastDBCardEditor*>(editor);
  if (!e) return;

  QModelIndex srcIndex = index;
  QAbstractItemModel *targetModel = model;
  if (auto *proxy = qobject_cast<QAbstractProxyModel *>(model)) {
    // 如果用了代理， 那就获取原模型和再原模型中的index
    srcIndex = proxy->mapToSource(index);
    targetModel = proxy->sourceModel();
  }
  if (!srcIndex.isValid() || !targetModel) {
    return;
  }

  // 3. 兜底：源模型若内部发生行重排，持久索引会跟随
  const QPersistentModelIndex pIndex(srcIndex);

  const QString newName    = e->mActorNameEdit->text();
  const int newRateValue   = e->mRateEdit->value();
  const QString newAlias   = e->mAliasEdit->text();
  const QString newTags    = e->mTagsEdit->text();
  const QString newOri     = e->mOriBelongToEdit->text();
  const int newHeightValue = e->mHeightEdit->value();
  const int newSizeValue   = e->mSizeEdit->value();
  const QString newDetail  = e->mDetailEdit->toPlainText();

  srcIndex = pIndex;
  targetModel->setData(srcIndex.siblingAtColumn(CastDbModelField::FIELD_E::Name), newName);
  srcIndex = pIndex;
  targetModel->setData(srcIndex.siblingAtColumn(CastDbModelField::FIELD_E::Rate), newRateValue);
  srcIndex = pIndex;
  targetModel->setData(srcIndex.siblingAtColumn(CastDbModelField::FIELD_E::ALIAS), newAlias);
  srcIndex = pIndex;
  targetModel->setData(srcIndex.siblingAtColumn(CastDbModelField::FIELD_E::Tags), newTags);
  srcIndex = pIndex;
  targetModel->setData(srcIndex.siblingAtColumn(CastDbModelField::FIELD_E::Ori), newOri);
  srcIndex = pIndex;
  targetModel->setData(srcIndex.siblingAtColumn(CastDbModelField::FIELD_E::Height), newHeightValue);
  srcIndex = pIndex;
  targetModel->setData(srcIndex.siblingAtColumn(CastDbModelField::FIELD_E::Size), newSizeValue);
  srcIndex = pIndex;
  targetModel->setData(srcIndex.siblingAtColumn(CastDbModelField::FIELD_E::Detail), newDetail);
}

void CastCardDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const {
  QRect r = option.rect.adjusted(CARD_MARGIN, CARD_MARGIN, -CARD_MARGIN, -CARD_MARGIN);;
  r.setHeight(qMax(r.height(), editor->sizeHint().height()));
  editor->setGeometry(r);
}