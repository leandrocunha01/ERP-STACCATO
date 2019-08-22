#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "application.h"
#include "pagamentosdia.h"
#include "reaisdelegate.h"
#include "ui_widgetfinanceirofluxocaixa.h"
#include "widgetfinanceirofluxocaixa.h"

WidgetFinanceiroFluxoCaixa::WidgetFinanceiroFluxoCaixa(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetFinanceiroFluxoCaixa) { ui->setupUi(this); }

WidgetFinanceiroFluxoCaixa::~WidgetFinanceiroFluxoCaixa() { delete ui; }

void WidgetFinanceiroFluxoCaixa::setConnections() {
  connect(ui->dateEdit, &QDateEdit::dateChanged, this, &WidgetFinanceiroFluxoCaixa::montaFiltro);
  connect(ui->groupBoxCaixa1, &QGroupBox::toggled, this, &WidgetFinanceiroFluxoCaixa::on_groupBoxCaixa1_toggled);
  connect(ui->groupBoxCaixa2, &QGroupBox::toggled, this, &WidgetFinanceiroFluxoCaixa::on_groupBoxCaixa2_toggled);
  connect(ui->groupBoxMes, &QGroupBox::toggled, this, &WidgetFinanceiroFluxoCaixa::montaFiltro);
  connect(ui->itemBoxCaixa1, &ItemBox::textChanged, this, &WidgetFinanceiroFluxoCaixa::montaFiltro);
  connect(ui->itemBoxCaixa2, &ItemBox::textChanged, this, &WidgetFinanceiroFluxoCaixa::montaFiltro);
  connect(ui->tableCaixa, &TableView::activated, this, &WidgetFinanceiroFluxoCaixa::on_tableCaixa_activated);
  connect(ui->tableCaixa2, &TableView::activated, this, &WidgetFinanceiroFluxoCaixa::on_tableCaixa2_activated);
}

void WidgetFinanceiroFluxoCaixa::updateTables() {
  if (not isSet) {
    ui->dateEdit->setDate(QDate::currentDate());

    ui->itemBoxCaixa1->setSearchDialog(SearchDialog::conta(this));
    ui->itemBoxCaixa2->setSearchDialog(SearchDialog::conta(this));

    // REFAC: 0dont hardcode magic numbers
    const int contaSantander = 3;
    const int contaCaixa = 8;

    ui->itemBoxCaixa1->setId(contaSantander);
    ui->itemBoxCaixa2->setId(contaCaixa);

    ui->groupBoxCaixa1->setChecked(true);
    ui->groupBoxCaixa2->setChecked(true);

    setConnections();

    isSet = true;
  }

  if (not modelIsSet) { modelIsSet = true; }

  montaFiltro();
}

void WidgetFinanceiroFluxoCaixa::resetTables() { modelIsSet = false; }

void WidgetFinanceiroFluxoCaixa::montaFiltro() {
  const QString filtroData = ui->groupBoxMes->isChecked() ? "`Data` IS NOT NULL AND DATE_FORMAT(`Data`, '%Y-%m') = '" + ui->dateEdit->date().toString("yyyy-MM") + "'" : "`Data` IS NOT NULL";

  const QString filtroConta = ui->groupBoxCaixa1->isChecked() and ui->itemBoxCaixa1->getId().isValid() ? "idConta = " + ui->itemBoxCaixa1->getId().toString() : "";

  // TODO: see if the outer select can be removed
  if (filtroConta.isEmpty()) {
    modelCaixa.setQuery("SELECT * FROM (SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS Acumulado FROM view_fluxo_resumo v JOIN (SELECT @running_total := 0) r WHERE `Data` IS "
                        "NOT NULL ORDER BY Data, idConta) x WHERE " +
                        filtroData);
  } else {
    modelCaixa.setQuery("SELECT * FROM (SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS Acumulado FROM view_fluxo_resumo2 v JOIN (SELECT @running_total := 0) r WHERE " +
                        filtroConta + " AND `Data` IS NOT NULL ORDER BY Data) x WHERE " + filtroData);
  }

  ui->tableCaixa->setModel(&modelCaixa);
  ui->tableCaixa->setItemDelegateForColumn("SAIDA", new ReaisDelegate(this));
  ui->tableCaixa->setItemDelegateForColumn("ENTRADA", new ReaisDelegate(this));
  ui->tableCaixa->setItemDelegateForColumn("R$", new ReaisDelegate(this));
  ui->tableCaixa->setItemDelegateForColumn("Acumulado", new ReaisDelegate(this));

  ui->tableCaixa->showColumn("SAIDA");

  ui->tableCaixa->hideColumn("Conta");
  ui->tableCaixa->hideColumn("idConta");
  ui->tableCaixa->hideColumn("Data Pag");

  // calcular saldo

  QSqlQuery query;

  if (not query.exec(modelCaixa.query().executedQuery() + " ORDER BY DATA DESC LIMIT 1")) { return qApp->enqueueError("Erro buscando saldo: " + query.lastError().text(), this); }

  if (query.first()) { ui->doubleSpinBoxSaldo1->setValue(query.value("Acumulado").toDouble()); }

  // ----------------------------------------------------------------------------------------------------------

  const QString filtroConta2 = ui->groupBoxCaixa2->isChecked() and ui->itemBoxCaixa2->getId().isValid() ? "idConta = " + ui->itemBoxCaixa2->getId().toString() : "";

  if (filtroConta2.isEmpty()) {
    modelCaixa2.setQuery("SELECT * FROM (SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS Acumulado FROM view_fluxo_resumo v JOIN (SELECT @running_total := 0) r WHERE `Data` IS "
                         "NOT NULL ORDER BY Data, idConta) x WHERE " +
                         filtroData);
  } else {
    modelCaixa2.setQuery("SELECT * FROM (SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS Acumulado FROM view_fluxo_resumo2 v JOIN (SELECT @running_total := 0) r WHERE " +
                         filtroConta2 + " AND `Data` IS NOT NULL ORDER BY Data) x WHERE " + filtroData);
  }

  ui->tableCaixa2->setModel(&modelCaixa2);
  ui->tableCaixa2->setItemDelegateForColumn("SAIDA", new ReaisDelegate(this));
  ui->tableCaixa2->setItemDelegateForColumn("ENTRADA", new ReaisDelegate(this));
  ui->tableCaixa2->setItemDelegateForColumn("R$", new ReaisDelegate(this));
  ui->tableCaixa2->setItemDelegateForColumn("Acumulado", new ReaisDelegate(this));

  ui->tableCaixa2->showColumn("SAIDA");

  ui->tableCaixa2->hideColumn("Conta");
  ui->tableCaixa2->hideColumn("idConta");
  ui->tableCaixa2->hideColumn("Data Pag");

  // calcular saldo

  if (not query.exec(modelCaixa2.query().executedQuery() + " ORDER BY DATA DESC LIMIT 1")) { return qApp->enqueueError("Erro buscando saldo: " + query.lastError().text(), this); }

  if (query.first()) { ui->doubleSpinBoxSaldo2->setValue(query.value("Acumulado").toDouble()); }

  // ----------------------------------------------------------------------------------------------------------

  modelFuturo.setQuery("SELECT v.*, @running_total := @running_total + COALESCE(v.`R$`, 0) AS Acumulado FROM view_fluxo_resumo3 v JOIN (SELECT @running_total := 0) r");

  if (modelFuturo.lastError().isValid()) { return qApp->enqueueError("Erro buscando dados futuros: " + modelFuturo.lastError().text(), this); }

  ui->tableFuturo->setModel(&modelFuturo);
  ui->tableFuturo->setItemDelegateForColumn("SAIDA", new ReaisDelegate(this));
  ui->tableFuturo->setItemDelegateForColumn("ENTRADA", new ReaisDelegate(this));
  ui->tableFuturo->setItemDelegateForColumn("R$", new ReaisDelegate(this));
  ui->tableFuturo->setItemDelegateForColumn("Acumulado", new ReaisDelegate(this));

  // ----------------------------------------------------------------------------------------------------------
}

void WidgetFinanceiroFluxoCaixa::on_tableCaixa2_activated(const QModelIndex &index) {
  const QDate date = modelCaixa2.data(index.row(), "Data").toDate();
  const QString idConta = modelCaixa2.data(index.row(), "idConta").toString();

  auto *dia = new PagamentosDia(this);
  dia->setFilter(date, idConta);
  dia->show();
}

void WidgetFinanceiroFluxoCaixa::on_tableCaixa_activated(const QModelIndex &index) {
  const QDate date = modelCaixa.data(index.row(), "Data").toDate();
  const QString idConta = modelCaixa.data(index.row(), "idConta").toString();

  auto *dia = new PagamentosDia(this);
  dia->setFilter(date, idConta);
  dia->show();
}

void WidgetFinanceiroFluxoCaixa::on_groupBoxCaixa1_toggled(const bool checked) {
  if (not checked) {
    ui->itemBoxCaixa1->clear();
    return;
  }

  montaFiltro();
}

void WidgetFinanceiroFluxoCaixa::on_groupBoxCaixa2_toggled(const bool checked) {
  if (not checked) {
    ui->itemBoxCaixa2->clear();
    return;
  }

  montaFiltro();
}

// TODO: 0nao agrupar contas no view_fluxo_resumo (apenas quando filtrado)
// TODO: 0fazer delegate para reduzir tamanho da fonte
// TODO: separar a tabela 'Futuro' em duas telas, uma 'vencidos' e a outra mantem igual a atual
