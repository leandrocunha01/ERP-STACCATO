#include "precoestoque.h"
#include "ui_precoestoque.h"

#include "application.h"
#include "noeditdelegate.h"
#include "reaisdelegate.h"
#include "sortfilterproxymodel.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

PrecoEstoque::PrecoEstoque(QWidget *parent) : QDialog(parent), ui(new Ui::PrecoEstoque) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setupTables();
  setConnections();
}

PrecoEstoque::~PrecoEstoque() { delete ui; }

void PrecoEstoque::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &PrecoEstoque::on_lineEditBusca_textChanged, connectionType);
  connect(ui->pushButtonCancelar, &QPushButton::clicked, this, &PrecoEstoque::on_pushButtonCancelar_clicked, connectionType);
  connect(ui->pushButtonSalvar, &QPushButton::clicked, this, &PrecoEstoque::on_pushButtonSalvar_clicked, connectionType);
}

void PrecoEstoque::setupTables() {
  modelProduto.setTable("produto");

  modelProduto.setHeaderData("fornecedor", "Fornecedor");
  modelProduto.setHeaderData("descricao", "Descrição");
  modelProduto.setHeaderData("estoqueRestante", "Estoque Disp.");
  modelProduto.setHeaderData("un", "Un.");
  modelProduto.setHeaderData("un2", "Un.2");
  modelProduto.setHeaderData("colecao", "Coleção");
  modelProduto.setHeaderData("tipo", "Tipo");
  modelProduto.setHeaderData("formComercial", "Form. Com.");
  modelProduto.setHeaderData("codComercial", "Cód. Com.");
  modelProduto.setHeaderData("precoVenda", "R$");
  modelProduto.setHeaderData("ui", "UI");

  modelProduto.setFilter("estoque = TRUE AND estoqueRestante > 0");

  modelProduto.select();

  modelProduto.proxyModel = new SortFilterProxyModel(&modelProduto, this);

  ui->table->setModel(&modelProduto);

  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("idEstoque");
  ui->table->hideColumn("idFornecedor");
  ui->table->hideColumn("quantCaixa");
  ui->table->hideColumn("minimo");
  ui->table->hideColumn("multiplo");
  ui->table->hideColumn("m2cx");
  ui->table->hideColumn("pccx");
  ui->table->hideColumn("kgcx");
  ui->table->hideColumn("codBarras");
  ui->table->hideColumn("ncm");
  ui->table->hideColumn("cfop");
  ui->table->hideColumn("icms");
  ui->table->hideColumn("cst");
  ui->table->hideColumn("qtdPallet");
  ui->table->hideColumn("custo");
  ui->table->hideColumn("ipi");
  ui->table->hideColumn("st");
  ui->table->hideColumn("sticms");
  ui->table->hideColumn("mva");
  ui->table->hideColumn("oldPrecoVenda");
  ui->table->hideColumn("markup");
  ui->table->hideColumn("comissao");
  ui->table->hideColumn("observacoes");
  ui->table->hideColumn("origem");
  ui->table->hideColumn("temLote");
  ui->table->hideColumn("validade");
  ui->table->hideColumn("descontinuado");
  ui->table->hideColumn("atualizarTabelaPreco");
  ui->table->hideColumn("representacao");
  ui->table->hideColumn("estoque");
  ui->table->hideColumn("promocao");
  ui->table->hideColumn("idProdutoRelacionado");
  ui->table->hideColumn("desativado");

  for (int column = 0, columnCount = modelProduto.columnCount(); column < columnCount; ++column) {
    if (modelProduto.record().fieldName(column).endsWith("Upd")) { ui->table->setColumnHidden(column, true); }
  }

  ui->table->setItemDelegate(new NoEditDelegate(this));

  ui->table->setItemDelegateForColumn("precoVenda", new ReaisDelegate(this));
}

void PrecoEstoque::on_pushButtonSalvar_clicked() {
  modelProduto.submitAll();

  qApp->enqueueInformation("Dados atualizados!", this);
  close();
}

void PrecoEstoque::on_pushButtonCancelar_clicked() { close(); }

void PrecoEstoque::on_lineEditBusca_textChanged(const QString &) {
  const QString text = qApp->sanitizeSQL(ui->lineEditBusca->text());

  if (text.isEmpty()) { return modelProduto.setFilter("estoque = TRUE AND estoqueRestante > 0"); }

  QStringList strings = text.split(" ", Qt::SkipEmptyParts);

  for (auto &string : strings) { string.contains("-") ? string.prepend("\"").append("\"") : string.prepend("+").append("*"); }

  modelProduto.setFilter("MATCH(fornecedor, descricao, codComercial, colecao) AGAINST('" + strings.join(" ") + "' IN BOOLEAN MODE) AND estoque = TRUE AND estoqueRestante > 0");
}
