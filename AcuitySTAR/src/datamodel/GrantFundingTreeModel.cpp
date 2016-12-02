#include "database/RecordsManager.h"
#include "datamodel/TreeItem.h"
#include "datamodel/GrantFundingTreeModel.h"

#include <QDebug>

GrantFundingTreeModel::GrantFundingTreeModel(RecordsManager* db, QObject *parent) :
TreeModel(db, parent) {
}

GrantFundingTreeModel::~GrantFundingTreeModel() {
}

void GrantFundingTreeModel::setupModel(int yearStart, int yearEnd, std::vector<std::string> sortFields, char filterStart, char filterEnd)
{
    //qDebug() << "entered setupModel";
    rootItem = new TreeItem(dataObj->createHeadersListForGrants(sortFields[0]));
    //qDebug() << "middle setupModel";
    setupModelData(dataObj->createQStringForGrants(yearStart, yearEnd, sortFields, filterStart, filterEnd).split(QString("\n")), rootItem);
    //qDebug() << "leaving setupModel";
}
