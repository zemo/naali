// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryTreeView.cpp
 *  @brief Inventory tree view UI widget.
 */

#include "StableHeaders.h"
#include "InventoryTreeView.h"
#include "InventoryItemModel.h"
#include "AbstractInventoryDataModel.h"
//#include "OpenSimInventoryDataModel.h"

#include <QWidget>
#include <QDragEnterEvent>
#include <QApplication>
#include <QUrl>

namespace Inventory
{

InventoryTreeView::InventoryTreeView(QWidget *parent) : QTreeView(parent)
{
    setEditTriggers(QAbstractItemView::EditKeyPressed);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setAnimated(true);
    setAllColumnsShowFocus(true);
}

// virtual
InventoryTreeView::~InventoryTreeView()
{
}

void InventoryTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
            event->acceptProposedAction();
    }
    else if(event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
}

void InventoryTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
            event->acceptProposedAction();
    }
    else if(event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
}

void InventoryTreeView::dropEvent(QDropEvent *event)
{
    const QMimeData *data = event->mimeData();
    if (data->hasUrls())
    {
        InventoryItemModel *itemModel = dynamic_cast<InventoryItemModel *>(model());
        if (!itemModel)
        {
            event->ignore();
            return;
        }

        AbstractInventoryDataModel *m = itemModel->GetInventory();
        if (!m)
        {
            event->ignore();
            return;
        }

        QStringList filenames, itemnames;
        QList<QUrl> urlList = data->urls();

        QListIterator<QUrl> it(urlList);
        while(it.hasNext())
            filenames << it.next().path().remove(0, 1);

        if (!filenames.isEmpty())
            m->UploadFiles(filenames, itemnames, 0);
    }
    else
        QTreeView::dropEvent(event);
}

}
