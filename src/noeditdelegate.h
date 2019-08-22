#pragma once

#include <QStyledItemDelegate>

class NoEditDelegate final : public QStyledItemDelegate {

public:
  explicit NoEditDelegate(QObject *parent = nullptr);
  auto createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &) const -> QWidget * final;
};
