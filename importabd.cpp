#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

#include "importabd.h"
#include "ui_importabd.h"
#include "cadastrocliente.h"

ImportaBD::ImportaBD(QWidget *parent) : QDialog(parent), ui(new Ui::ImportaBD) {
  ui->setupUi(this);

  progressDialog = new QProgressDialog(this);
  progressDialog->setCancelButton(0);
  progressDialog->setLabelText("Importando...");
  progressDialog->setWindowTitle("ERP Staccato");
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->setCancelButtonText("Cancelar");
  connect(&futureWatcher, &QFutureWatcherBase::finished, progressDialog, &QProgressDialog::cancel);
  connect(&futureWatcher, &QFutureWatcherBase::finished, this, &ImportaBD::mostraResultado);
  connect(progressDialog, &QProgressDialog::canceled, &importaExport, &ImportaExport::cancel);
  connect(&importaExport, &ImportaExport::progressRangeChanged, this, &ImportaBD::updateProgressRange);
  connect(&importaExport, &ImportaExport::progressValueChanged, this, &ImportaBD::updateProgressValue);
  connect(&importaExport, &ImportaExport::progressTextChanged, this, &ImportaBD::updateProgressText);
  connect(progressDialog, &QProgressDialog::canceled, &portinari, &ImportaPortinari::cancel);
  connect(&portinari, &ImportaPortinari::progressRangeChanged, this, &ImportaBD::updateProgressRange);
  connect(&portinari, &ImportaPortinari::progressValueChanged, this, &ImportaBD::updateProgressValue);
  connect(&portinari, &ImportaPortinari::progressTextChanged, this, &ImportaBD::updateProgressText);
  show();
}

ImportaBD::~ImportaBD() {
  delete ui;
}

void ImportaBD::mostraResultado() {
  QMessageBox::information(this, "Aviso", futureWatcher.result(), QMessageBox::Ok);
}

void ImportaBD::updateProgressRange(int max) {
  progressDialog->setMaximum(max);
}

void ImportaBD::updateProgressValue(int val) {
  progressDialog->setValue(val);
}

void ImportaBD::updateProgressText(QString str) {
  progressDialog->setLabelText(str);
}

void ImportaBD::on_pushButtonPortinari_clicked() {
  QString file = QFileDialog::getOpenFileName(this, "Importar tabela da Portinari", QDir::currentPath(), tr("Excel (*.xls)"));
  if (file.isEmpty()) {
    return;
  }

  QFuture<QString> future = QtConcurrent::run(&this->portinari, &ImportaPortinari::importar, file);
  futureWatcher.setFuture(future);
  progressDialog->exec();
}

void ImportaBD::on_pushButtonApavisa_clicked() {
  QString file = QFileDialog::getOpenFileName(this, "Importar tabela da Apavisa", QDir::currentPath(), tr("Excel (*.xls)"));
  if (file.isEmpty()) {
    return;
  }

  QFuture<QString> future = QtConcurrent::run(&this->apavisa, &ImportaApavisa::importar, file);
  futureWatcher.setFuture(future);
  progressDialog->exec();
}

void ImportaBD::on_pushButtonExport_clicked() {
  QString file = QFileDialog::getOpenFileName(this, "Importar tabela genérica", QDir::currentPath(), tr("Excel (*.xlsx)"));
  if (file.isEmpty()) {
    return;
  }
  int validade = QInputDialog::getInt(this, "Validade", "Insira a validade em dias: ");

  QFuture<QString> future = QtConcurrent::run(&importaExport, &ImportaExport::importar, file, validade);
  futureWatcher.setFuture(future);
  progressDialog->exec();
}
