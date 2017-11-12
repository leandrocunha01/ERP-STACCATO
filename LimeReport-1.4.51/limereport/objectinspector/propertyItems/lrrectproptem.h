/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#ifndef LRRECTPROPTEM_H
#define LRRECTPROPTEM_H

#include <QMetaProperty>
#include <QRectF>

#include "lrobjectpropitem.h"

namespace LimeReport {

class RectPropItem : public ObjectPropItem {
  Q_OBJECT
public:
  RectPropItem() : ObjectPropItem() {}
  RectPropItem(QObject *object, ObjectsList *objects, const QString &name, const QString &displayName, const QVariant &value, ObjectPropItem *parent, bool readonly = true);
  QString displayValue() const;
};

class RectMMPropItem : public ObjectPropItem {
  Q_OBJECT
public:
  RectMMPropItem() : ObjectPropItem() {}
  RectMMPropItem(QObject *object, ObjectsList *objects, const QString &name, const QString &displayName, const QVariant &value, ObjectPropItem *parent, bool readonly = true);
  QString displayValue() const;
public slots:
  void itemPosChanged(QObject * /*object*/, QPointF newPos, QPointF oldPos);
  void itemGeometryChanged(QObject *object, QRectF newGeometry, QRectF oldGeometry);

private:
  void setValue(const QString &propertyName, qreal propertyValue);
};

class RectMMValuePropItem : public ObjectPropItem {
  Q_OBJECT
public:
  RectMMValuePropItem() : ObjectPropItem() {}
  RectMMValuePropItem(QObject *object, ObjectsList *objects, const QString &name, const QString &displayName, const QVariant &value, ObjectPropItem *parent, bool readonly);
  QString displayValue() const;
  QWidget *createProperyEditor(QWidget *) const;
  void setPropertyEditorData(QWidget *, const QModelIndex &) const;
  void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &);
};

} // namespace LimeReport
#endif // LRRECTPROPTEM_H
