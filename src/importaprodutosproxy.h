#ifndef IMPORTAPRODUTOSPROXY_H
#define IMPORTAPRODUTOSPROXY_H

#include <QIdentityProxyModel>

#include "sqltablemodel.h"

class ImportaProdutosProxy : public QIdentityProxyModel {

public:
  ImportaProdutosProxy(SqlTableModel *model, QObject *parent = 0);
  ~ImportaProdutosProxy() = default;
  QVariant data(const QModelIndex &proxyIndex, const int role) const override;

private:
  const int descontinuado;
  enum class Status { Novo = 1, Atualizado, ForaPadrao, Errado };
};

#endif // IMPORTAPRODUTOSPROXY_H
