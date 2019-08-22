#include <QBrush>

#include "followupproxymodel.h"
#include "usersession.h"

FollowUpProxyModel::FollowUpProxyModel(SqlRelationalTableModel *model, QObject *parent) : QIdentityProxyModel(parent), semaforoColumn(model->fieldIndex("semaforo", true)) { setSourceModel(model); }

QVariant FollowUpProxyModel::data(const QModelIndex &proxyIndex, int role) const {
  if (semaforoColumn != -1) {
    const FieldColors semaforo = static_cast<FieldColors>(QIdentityProxyModel::data(index(proxyIndex.row(), semaforoColumn), Qt::DisplayRole).toInt());

    if (semaforo == FieldColors::Quente) {
      if (role == Qt::BackgroundRole) { return QBrush(QColor(255, 66, 66)); }
      if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
    }

    if (semaforo == FieldColors::Morno) {
      if (role == Qt::BackgroundRole) { return QBrush(QColor(255, 170, 0)); }
      if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
    }

    if (semaforo == FieldColors::Frio) {
      if (role == Qt::BackgroundRole) { return QBrush(QColor(70, 113, 255)); }
      if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
    }
  }

  if (role == Qt::ForegroundRole) {
    const auto tema = UserSession::getSetting("User/tema");

    if (not tema) { return QBrush(Qt::black); }

    return tema->toString() == "claro" ? QBrush(Qt::black) : QBrush(Qt::white);
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
