#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>

class ComboBox final : public QComboBox {
  Q_OBJECT

public:
  explicit ComboBox(QWidget *parent);
  ~ComboBox() = default;
  auto getCurrentValue() const -> QVariant;
  auto setCurrentValue(const QVariant &value) -> bool;
};

#endif // COMBOBOX_H
