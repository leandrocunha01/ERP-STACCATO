#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class TableView final : public QTableView {
  Q_OBJECT

public:
  explicit TableView(QWidget *parent = nullptr);
  ~TableView() final = default;
  void hideColumn(const QString &column);
  void openPersistentEditor(const int row, const int column);
  void openPersistentEditor(const int row, const QString &column);
  void setItemDelegateForColumn(const int column, QAbstractItemDelegate *delegate);
  void setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate);
  void setModel(QAbstractItemModel *model) final;
  void showColumn(const QString &column);

public slots:
  void sortByColumn(const QString &column, Qt::SortOrder order = Qt::AscendingOrder);
};

#endif // TABLEVIEW_H
