#pragma once

#include "registeraddressdialog.h"
#include "searchdialog.h"

namespace Ui {
class CadastroProfissional;
}

class CadastroProfissional final : public RegisterAddressDialog {
  Q_OBJECT

public:
  explicit CadastroProfissional(QWidget *parent = nullptr);
  ~CadastroProfissional();

private:
  // attributes
  QString tipoPFPJ;
  SearchDialog *sdProfissional;
  Ui::CadastroProfissional *ui;
  // methods
  auto cadastrar() -> bool final;
  auto cadastrarEndereco(const Tipo tipoEndereco = Tipo::Cadastrar) -> bool;
  auto clearEndereco() -> void;
  auto clearFields() -> void final;
  auto novoEndereco() -> void;
  auto on_checkBoxMostrarInativos_clicked(const bool checked) -> void;
  auto on_lineEditCEP_textChanged(const QString &cep) -> void;
  auto on_lineEditCNPJBancario_textEdited(const QString &text) -> void;
  auto on_lineEditCNPJ_editingFinished() -> void;
  auto on_lineEditCNPJ_textEdited(const QString &text) -> void;
  auto on_lineEditCPFBancario_textEdited(const QString &text) -> void;
  auto on_lineEditCPF_editingFinished() -> void;
  auto on_lineEditCPF_textEdited(const QString &text) -> void;
  auto on_lineEditContatoCPF_textEdited(const QString &text) -> void;
  auto on_lineEditProfissional_editingFinished() -> void;
  auto on_pushButtonAdicionarEnd_clicked() -> void;
  auto on_pushButtonAtualizarEnd_clicked() -> void;
  auto on_pushButtonAtualizar_clicked() -> void;
  auto on_pushButtonBuscar_clicked() -> void;
  auto on_pushButtonCadastrar_clicked() -> void;
  auto on_pushButtonNovoCad_clicked() -> void;
  auto on_pushButtonRemoverEnd_clicked() -> void;
  auto on_pushButtonRemover_clicked() -> void;
  auto on_radioButtonPF_toggled(const bool checked) -> void;
  auto on_tableEndereco_clicked(const QModelIndex &index) -> void;
  auto registerMode() -> void final;
  auto savingProcedures() -> bool final;
  auto setupMapper() -> void final;
  auto setupTables() -> void;
  auto setupUi() -> void;
  auto successMessage() -> void final;
  auto updateMode() -> void final;
  auto verifyFields() -> bool final;
  auto viewRegister() -> bool final;
};
