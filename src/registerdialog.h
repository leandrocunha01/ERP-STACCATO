#pragma once

#include <QDataWidgetMapper>
#include <QDialog>
#include <QLineEdit>

#include "sqlrelationaltablemodel.h"

class RegisterDialog : public QDialog {
  Q_OBJECT

public:
  explicit RegisterDialog(const QString &table, const QString &primaryKey, QWidget *parent = nullptr);
  ~RegisterDialog() override = default;

  auto marcarDirty() -> void;
  auto show() -> void;
  virtual auto viewRegisterById(const QVariant &id) -> bool;

signals:
  void registerUpdated(const QVariant &idCliente);

protected:
  // attributes
  bool isDirty = false;
  enum class Tipo { Cadastrar, Atualizar } tipo = Tipo::Cadastrar;
  int currentRow = -1;
  QDataWidgetMapper mapper;
  QString primaryId;
  QString primaryKey;
  QStringList textKeys;
  SqlRelationalTableModel model;
  // methods
  auto addMapping(QWidget *widget, const QString &key, const QByteArray &propertyName = QByteArray()) -> void;
  auto closeEvent(QCloseEvent *event) -> void final;
  auto columnsToUpper(SqlRelationalTableModel &someModel, const int row) -> bool;
  auto confirmationMessage() -> bool;
  auto data(const QString &key) -> QVariant;
  auto data(const int row, const QString &key) -> QVariant;
  auto getTextKeys() const -> QStringList;
  auto keyPressEvent(QKeyEvent *event) -> void final;
  auto remove() -> void;
  auto requiredStyle() -> QString;
  auto setData(const QString &key, const QVariant &value) -> bool;
  auto setForeignKey(SqlRelationalTableModel &secondaryModel) -> bool;
  auto setTextKeys(const QStringList &value) -> void;
  auto validaCNPJ(const QString &text) -> bool;
  auto validaCPF(const QString &text) -> bool;
  auto verifyFields(const QList<QLineEdit *> &list) -> bool;
  virtual auto cadastrar() -> bool = 0;
  virtual auto clearFields() -> void = 0;
  virtual auto newRegister() -> bool;
  virtual auto registerMode() -> void = 0;
  virtual auto save(const bool silent = false) -> bool final;
  virtual auto savingProcedures() -> bool = 0;
  virtual auto setupMapper() -> void = 0;
  virtual auto successMessage() -> void = 0;
  virtual auto updateMode() -> void = 0;
  virtual auto verifyFields() -> bool = 0;
  virtual auto verifyRequiredField(QLineEdit *line, const bool silent = false) -> bool;
  virtual auto viewRegister() -> bool;
};
