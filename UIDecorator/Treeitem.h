/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>

typedef QPair<QString,QVariant> TreeItemData;

class TreeItem
{
public:
    explicit TreeItem(const QVector<TreeItemData> &data , TreeItem *parent = 0);
    ~TreeItem();

    TreeItem *getParent();
    void setParent(TreeItem *parent);
    TreeItem *getItem(int getNumber);
    const QList<TreeItem*>& getItems() const;

    int getNumber() const;

    void addItem(TreeItem* item);
    int itemsCount() const;
    int columnCount() const;

    bool insertItem(int position, int count, int columns);
    bool insertColumns(int position, int columns);

    bool removeItem(int position, int count);
    bool removeColumns(int position, int columns);


    bool setData(int column, const QVariant &value);
    bool setData(int column, const TreeItemData &value);
    QVariant getData(int column) const;

    void setData(const QVector<TreeItemData>& data);
    const QVector<TreeItemData>& getData() const;

private:
    QList<TreeItem*> mItems;
    QVector< TreeItemData > mData;
    TreeItem *mParent;
};


#endif // TREEITEM_H
