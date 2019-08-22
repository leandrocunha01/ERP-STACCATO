#pragma once

#include <QSortFilterProxyModel>

class SortFilterProxyModel : public QSortFilterProxyModel {
  Q_OBJECT

public:
  explicit SortFilterProxyModel(QAbstractItemModel *model, QObject *parent = nullptr);
  auto data(const int row, const int column, int role = Qt::DisplayRole) const -> QVariant;

private:
  using QSortFilterProxyModel::data;
};
