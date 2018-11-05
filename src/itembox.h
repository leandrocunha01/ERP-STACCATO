#ifndef ITEMBOX_H
#define ITEMBOX_H

#include <QLineEdit>
#include <QPushButton>

#include "registerdialog.h"
#include "searchdialog.h"

class ItemBox final : public QLineEdit {
  Q_OBJECT

public:
  explicit ItemBox(QWidget *parent);
  ~ItemBox() = default;
  auto changeItem(const QVariant &newValue) -> void;
  auto clear() -> void;
  auto getValue() const -> QVariant;
  auto setFilter(const QString &filter) -> void;
  auto setFornecedorRep(const QString &fornecedor) -> void;
  auto setReadOnlyItemBox(const bool isReadOnly) -> void;
  auto setRegisterDialog(RegisterDialog *dialog) -> void;
  auto setRepresentacao(const bool isRepresentacao) -> void;
  auto setSearchDialog(SearchDialog *dialog) -> void;
  auto setValue(const QVariant &newValue) -> void;

signals:
  void valueChanged(const QVariant &changedValue);

private:
  Q_PROPERTY(QVariant value READ getValue WRITE setValue STORED false)
  // attributes
  bool readOnlyItemBox = false;
  QPushButton *searchButton;
  QPushButton *plusButton;
  QVariant value;
  RegisterDialog *registerDialog = nullptr;
  SearchDialog *searchDialog = nullptr;
  // methods
  auto edit() -> void;
  auto mouseDoubleClickEvent(QMouseEvent *event) -> void final;
  auto resetCursor() -> void;
  auto resizeEvent(QResizeEvent *event) -> void final;
  auto search() -> void;
  auto setIcons() -> void;
};

#endif // ITEMBOX_H
