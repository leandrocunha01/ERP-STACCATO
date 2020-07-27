#pragma once

#include <QGraphicsTextItem>

class EstoqueItem : public QObject, public QGraphicsSimpleTextItem {
  Q_OBJECT

public:
  explicit EstoqueItem(const QString &text, const int idVendaProduto2, QGraphicsItem *parent = nullptr);
  const int idVendaProduto2;

signals:
  void startDragSignal();

protected:
  auto startDrag(QPointF pos) -> void;
  virtual auto mousePressEvent(QGraphicsSceneMouseEvent *event) -> void override;

private:
};
