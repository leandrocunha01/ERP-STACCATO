#ifndef SEARCHDIALOGPROXY_H
#define SEARCHDIALOGPROXY_H

#include <QIdentityProxyModel>

#include "sqlrelationaltablemodel.h"

class SearchDialogProxy : public QIdentityProxyModel {

public:
  SearchDialogProxy(SqlRelationalTableModel *model, QObject *parent = 0);
  ~SearchDialogProxy() = default;
  QVariant data(const QModelIndex &proxyIndex, int role) const override;

private:
  const int estoque;
  const int promocao;
  const int descontinuado;
  const int validade;
};

#endif // SEARCHDIALOGPROXY_H
