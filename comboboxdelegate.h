#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class ComboBoxDelegate : public QStyledItemDelegate {
  public:
    ComboBoxDelegate(QObject *parent = 0);
    ~ComboBoxDelegate();

    // QAbstractItemDelegate interface
  public:
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    virtual void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // COMBOBOXDELEGATE_H
