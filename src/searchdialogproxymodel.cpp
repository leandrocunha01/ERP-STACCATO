#include "searchdialogproxymodel.h"

#include "application.h"
#include "user.h"

#include <QBrush>
#include <QDate>
#include <QDebug>
#include <QSqlRecord>

SearchDialogProxyModel::SearchDialogProxyModel(QSqlQueryModel *model, QObject *parent)
    : SortFilterProxyModel(model, parent), descontinuadoColumn(model->record().indexOf("descontinuado")), estoqueColumn(model->record().indexOf("estoque")),
      promocaoColumn(model->record().indexOf("promocao")), validadeColumn(model->record().indexOf("validadeProdutos")) {}

SearchDialogProxyModel::SearchDialogProxyModel(SqlTreeModel *model, QObject *parent)
    : SortFilterProxyModel(model, parent), estoqueColumn(model->fieldIndex("estoque")), promocaoColumn(model->fieldIndex("promocao")) {}

QVariant SearchDialogProxyModel::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole or role == Qt::ForegroundRole) {
    if (not proxyIndex.model()->hasChildren(proxyIndex) and proxyIndex.parent().isValid()) {
      const QString tema = User::getSetting("User/tema").toString();

      if (role == Qt::BackgroundRole) { return (tema == "escuro") ? QBrush(Qt::darkGray) : QBrush(Qt::gray); }
      if (role == Qt::ForegroundRole) { return (tema == "escuro") ? QBrush(Qt::white) : QBrush(Qt::black); }
    }

    if (descontinuadoColumn != -1) {
      const bool descontinuado = proxyIndex.siblingAtColumn(descontinuadoColumn).data().toBool();

      if (descontinuado) {
        if (role == Qt::BackgroundRole) { return QBrush(Qt::red); }
        if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
      }
    }

    if (estoqueColumn != -1 and promocaoColumn != -1) {
      const bool estoque = proxyIndex.siblingAtColumn(estoqueColumn).data().toBool();
      const int promocao = proxyIndex.siblingAtColumn(promocaoColumn).data().toInt();

      if (estoque and promocao == 0) {
        if (role == Qt::BackgroundRole) { return QBrush(Qt::yellow); }
        if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
      }

      if (estoque and promocao == 2) {
        if (role == Qt::BackgroundRole) { return QBrush(Qt::blue); } // STACCATO OFF
        if (role == Qt::ForegroundRole) { return QBrush(Qt::white); }
      }

      if (not estoque and promocao == 1) {
        if (role == Qt::BackgroundRole) { return QBrush(Qt::green); } // promocao
        if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
      }
    }

    if (proxyIndex.column() == validadeColumn) {
      const QDate validade = proxyIndex.siblingAtColumn(validadeColumn).data().toDate();
      const bool expirado = validade < qApp->serverDate();

      if (validade.isValid() and expirado) {
        if (role == Qt::BackgroundRole) { return QBrush(Qt::red); }
        if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
      }
    }

    if (role == Qt::ForegroundRole) {
      const QString tema = User::getSetting("User/tema").toString();

      return (tema == "escuro") ? QBrush(Qt::white) : QBrush(Qt::black);
    }
  }

  return QSortFilterProxyModel::data(proxyIndex, role);
}

bool SearchDialogProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
  if (source_left.parent().isValid()) { return false; } // disable sorting for childs

  return QSortFilterProxyModel::lessThan(source_left, source_right);
}
