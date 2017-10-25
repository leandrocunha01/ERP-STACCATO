#include "acbr.h"
#include "ui_xml_viewer.h"
#include "xml.h"
#include "xml_viewer.h"

XML_Viewer::XML_Viewer(QWidget *parent) : QDialog(parent), ui(new Ui::XML_Viewer) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  ui->treeView->setModel(&model);
  ui->treeView->setUniformRowHeights(true);
  ui->treeView->setAnimated(true);
  ui->treeView->setEditTriggers(QTreeView::NoEditTriggers);
}

XML_Viewer::~XML_Viewer() { delete ui; }

void XML_Viewer::exibirXML(const QByteArray &content) {
  if (content.isEmpty()) return;

  fileContent = content;

  XML xml(content);
  xml.montarArvore(model);

  ui->treeView->expandAll();

  show();
}

void XML_Viewer::on_pushButtonDanfe_clicked() {
  QString resposta;
  if (not ACBr::gerarDanfe(fileContent, resposta)) return;
}
