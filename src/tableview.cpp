#include <QAbstractProxyModel>
#include <QHeaderView>
#include <QMenu>
#include <QSqlQueryModel>
#include <QSqlRecord>

#include "tableview.h"

TableView::TableView(QWidget *parent) : QTableView(parent) {
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, &QWidget::customContextMenuRequested, this, &TableView::showContextMenu);

  verticalHeader()->setResizeContentsPrecision(0);
  horizontalHeader()->setResizeContentsPrecision(0);

  verticalHeader()->setDefaultSectionSize(20);

  setMouseTracking(true);
}

void TableView::showContextMenu(const QPoint &pos) {
  QMenu contextMenu;

  QAction action("Autodimensionar", this);
  action.setCheckable(true);
  action.setChecked(autoResize);
  connect(&action, &QAction::triggered, this, &TableView::toggleAutoResize);
  contextMenu.addAction(&action);

  contextMenu.exec(mapToGlobal(pos));
}

void TableView::toggleAutoResize() { autoResize = not autoResize; }

void TableView::hideColumn(const QString &column) {
  if (const auto *model = qobject_cast<QAbstractProxyModel *>(QTableView::model())) {
    if (const auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::hideColumn(sourceModel->record().indexOf(column));
      return;
    }
  }

  if (const auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::hideColumn(model->record().indexOf(column));
    return;
  }
}

void TableView::showColumn(const QString &column) {
  if (const auto *model = qobject_cast<QAbstractProxyModel *>(QTableView::model())) {
    if (const auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::showColumn(sourceModel->record().indexOf(column));
      return;
    }
  }

  if (const auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::showColumn(model->record().indexOf(column));
    return;
  }
}

void TableView::setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate) {
  if (const auto *model = qobject_cast<QAbstractProxyModel *>(QTableView::model())) {
    if (const auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::setItemDelegateForColumn(sourceModel->record().indexOf(column), delegate);
      return;
    }
  }

  if (const auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::setItemDelegateForColumn(model->record().indexOf(column), delegate);
    return;
  }
}

void TableView::openPersistentEditor(const int row, const QString &column) {
  if (const auto *model = qobject_cast<QAbstractProxyModel *>(QTableView::model())) {
    if (const auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::openPersistentEditor(model->index(row, sourceModel->record().indexOf(column)));
      return;
    }
  }

  if (const auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::openPersistentEditor(model->index(row, model->record().indexOf(column)));
    return;
  }
}

void TableView::openPersistentEditor(const int row, const int column) { QTableView::openPersistentEditor(QTableView::model()->index(row, column)); }

void TableView::setItemDelegateForColumn(const int column, QAbstractItemDelegate *delegate) { QTableView::setItemDelegateForColumn(column, delegate); }

void TableView::sortByColumn(const QString &column, Qt::SortOrder order) {
  if (const auto *model = qobject_cast<QAbstractProxyModel *>(QTableView::model())) {
    if (const auto *sourceModel = qobject_cast<QSqlQueryModel *>(model->sourceModel())) {
      QTableView::sortByColumn(sourceModel->record().indexOf(column), order);
      return;
    }
  }

  if (const auto *model = qobject_cast<QSqlQueryModel *>(QTableView::model())) {
    QTableView::sortByColumn(model->record().indexOf(column), order);
    return;
  }
}

void TableView::setModel(QAbstractItemModel *model) {
  QTableView::setModel(model);

  hideColumn("created");
  hideColumn("lastUpdated");
}

void TableView::enterEvent(QEvent *event) {
  if (autoResize) { resizeColumnsToContents(); }
  QTableView::enterEvent(event);
}

// TODO: 4program copy - http://stackoverflow.com/questions/3135737/copying-part-of-qtableview
