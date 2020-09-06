#pragma once

#include "sortfilterproxymodel.h"

#include <QSqlQueryModel>

class NFeProxyModel final : public SortFilterProxyModel {

public:
  explicit NFeProxyModel(QSqlQueryModel *model, QObject *parent);
  ~NFeProxyModel() = default;

private:
  // attributes
  const int statusColumn = -1;
  const int dataColumn = -1;
  // methods
  auto data(const QModelIndex &proxyIndex, const int role) const -> QVariant final;
};
