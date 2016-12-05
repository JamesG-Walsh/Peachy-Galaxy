#include <QDate>
#include <QDebug>
#include <QDirModel>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QPrintDialog>
#include <QPrinter>
#include <QString>
#include <QFile>
#include <database/QSortListIO.h>
#include <QSharedPointer>
#include <string.h>
#include <iostream>
#include <math.h>
using namespace std;

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CustomSort.h"
#include "editsort.h"
#include "ErrorEditDialog.h"
#include "selectData.h"

#include "database/CSVReader.h"
#include "database/QSortListIO.h"
#include "database/QFileIO.h"
#include "database/RecordsManager.h"
#include "datamodel/TreeModel.h"
#include "datamodel/GrantFundingTreeModel.h"
#include "datamodel/PresentationTreeModel.h"
#include "datamodel/PublicationTreeModel.h"
#include "datamodel/TeachingTreeModel.h"

#define FUNDORDER_SAVE  "fundsortorder.dat"
#define PRESORDER_SAVE  "pressortorder.dat"
#define PUBORDER_SAVE   "pubsortorder.dat"
#define TEACHORDER_SAVE "teachsortorder.dat"


std::vector<std::string> MainWindow::GRANTS_MANFIELDS = {"Member Name", "Funding Type", "Status", "Peer Reviewed?", "Role", "Title", "Start Date"};
std::vector<std::string> MainWindow::PRES_MANFIELDS = {"Member Name", "Date", "Type", "Role", "Title"};
std::vector<std::string> MainWindow::PUBS_MANFIELDS = {"Member Name", "Type", "Status Date", "Role", "Title"};
std::vector<std::string> MainWindow::TEACH_MANFIELDS = {"Member Name", "Start Date", "Program", "Division"};
std::vector<std::string> MainWindow::clickedNames;
std::vector<std::tuple <std::string, std::string, double>> MainWindow::chartLists;
std::vector<string> MainWindow::teachNames, MainWindow::pubNames, MainWindow::presNames, MainWindow::fundNames;
std::vector<string> MainWindow::teachNamesCustom, MainWindow::pubNamesCustom, MainWindow::presNamesCustom, MainWindow::fundNamesCustom;
bool MainWindow::teachFlag = false;
bool MainWindow::pubFlag = false;
bool MainWindow::presFlag = false;
bool MainWindow::fundFlag = false;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow),
    fundTree(NULL), presTree(NULL), pubTree(NULL), teachTree(NULL),
    funddb(NULL), presdb(NULL), pubdb(NULL), teachdb(NULL),
    teachdb2(NULL), pubdb2(NULL), presdb2(NULL), funddb2(NULL) {
    // draw GUI elements
    ui->setupUi(this);

    this->showMaximized();

    // set up the logo
    QPixmap logo(":/logo.png");
    ui->logo_label->setPixmap(logo);

    // set up application icon
    QIcon icon(":/icon32.ico");
    MainWindow::setWindowIcon(icon);
    MainWindow::setWindowTitle("Orion Galaxy");


    // set the date format
    QString dFormat("yyyy");
    ui->FromDate->setDisplayFormat(dFormat);
    ui->ToDate->setDisplayFormat(dFormat);

    // set start year to be 1950
    QDate startDate(1950, 1, 1);

    // set ranges for spinbox
    ui->FromDate->setDateRange(startDate, QDate::currentDate());
    ui->ToDate->setDateRange(startDate, QDate::currentDate());

    // set default dates (1950->current)
    ui->FromDate->setDate(startDate);
    ui->ToDate->setDate(QDate::currentDate());

    // set some member variables to the current date values
    yearStart = 1950;
    yearEnd = QDate::currentDate().year();

    //default print is disabled
    ui->teachPrintButton->setEnabled(false);
    ui->fundPrintButton->setEnabled(false);
    ui->pubPrintButton->setEnabled(false);
    ui->presPrintButton->setEnabled(false);

    ui->teachExportButton->setEnabled(false);
    ui->fundExportButton->setEnabled(false);
    ui->pubExportButton->setEnabled(false);
    ui->presExportButton->setEnabled(false);

    //Setup printer
    printer = new QPrinter();

    dateChanged = {false, false, false, false};

    //Look for any saved sessions
    QFileIO pubSave("pubfile.dat");
    QString readPubPath = pubSave.readPath();
    QFileIO teachSave("teachfile.dat");
    QString readTeachPath = teachSave.readPath();
    QFileIO presSave("presfile.dat");
    QString readPresPath = presSave.readPath();
    QFileIO fundSave("fundfile.dat");
    QString readFundPath = fundSave.readPath();
    QSortListIO saveSort("teachLastSort.dat");

    //Loads found saved sessions
    if(!readPubPath.isEmpty()){
        load_pub(readPubPath, true, true);
    }
    if(!readTeachPath.isEmpty()){
        load_teach(readTeachPath, true, true);
    }
    if(!readPresPath.isEmpty()){
        load_pres(readPresPath, true, true);
    }
    if(!readFundPath.isEmpty()){
        load_fund(readFundPath, true, true);
    }
}

MainWindow::~MainWindow() {
    delete ui;

    delete fundTree;
    delete presTree;
    delete pubTree;
    delete teachTree;

    delete funddb;
    delete presdb;
    delete pubdb;
    delete teachdb;
    delete teachdb2;
    delete funddb2;
    delete presdb2;
    delete pubdb2;
    delete printer;
}

void MainWindow::on_actionLoad_file_triggered() {
    QStringList filePaths = QFileDialog::getOpenFileNames(this,
                                                          "Select one or more files to load",
                                                          QDir::currentPath(),
                                                          tr("CSV (*.csv);; All files (*.*)"));
    if (filePaths.size() > 0)
    {
        const int NUM_TABS = 4;
        bool all_loaded[NUM_TABS] = {false, false, false, false};
        int sum = std::accumulate(all_loaded, all_loaded + NUM_TABS, 0);
        QStringList::Iterator it = filePaths.begin();
        while (sum != NUM_TABS && it != filePaths.end())
        {
            QString path = it[0];
            //note short-circuit eval
            if (!all_loaded[FUNDING] && load_fund(path, true)) {
                all_loaded[FUNDING] = true;
            } else if (!all_loaded[TEACH] && load_teach(path, true)) {
                all_loaded[TEACH] = true;
            } else if (!all_loaded[PUBLICATIONS] && load_pub(path, true)) {
                all_loaded[PUBLICATIONS] = true;
            } else if (!all_loaded[PRESENTATIONS] && load_pres(path, true)) {
                all_loaded[PRESENTATIONS] = true;
            }
            sum = std::accumulate(all_loaded, all_loaded + NUM_TABS, 0);
            ++it;
        }
    }
}


/*
 * When the load file button is clicked a file open dialog box
 * is opened.  If a file name is successfully returned, makeTree()
 * is called.
 */
QString MainWindow::load_file() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::currentPath(),
                                                    tr("CSV (*.csv);; All files (*.*)"));

    if (!filePath.isEmpty()) {
        return filePath;
    } else {
        return "";
    }
}

void MainWindow::refresh(int tabIndex)
{
    // if we've loaded in a file, update that data
    switch (tabIndex) {
    case FUNDING:
        if (!fundPath.isEmpty()) {
            makeTree(FUNDING);
            ui->fund_graph_stackedWidget->hide();
            ui->fundGraphTitle->clear();
        }
        break;

    case PRESENTATIONS:
        if (!presPath.isEmpty()) {
            makeTree(PRESENTATIONS);
            ui->pres_graph_stackedWidget->hide();
            ui->presGraphTitle->clear();
        }
        break;
    case PUBLICATIONS:
        if (!pubPath.isEmpty()) {
            makeTree(PUBLICATIONS);
            ui->pub_graph_stackedWidget->hide();
            ui->pubGraphTitle->clear();
        }
        break;

    case TEACH:
        if (!teachPath.isEmpty()) {
            makeTree(TEACH);
            ui->teach_graph_stackedWidget->hide();
            ui->teachGraphTitle->clear();
        }
        break;

    case TEACH_CUSTOM:
        if(!teachPath.isEmpty()){
            makeTree(TEACH_CUSTOM);
            ui->teach_graph_stackedWidget->hide();
            ui->teachGraphTitle->clear();
        }
        break;
    case FUNDING_CUSTOM:
        if (!fundPath.isEmpty()) {
            makeTree(FUNDING_CUSTOM);
            ui->fund_graph_stackedWidget->hide();
            ui->fundGraphTitle->clear();
        }
        break;

    case PRESENTATIONS_CUSTOM:
        if (!presPath.isEmpty()) {
            makeTree(PRESENTATIONS_CUSTOM);
            ui->pres_graph_stackedWidget->hide();
            ui->presGraphTitle->clear();
        }
        break;
    case PUBLICATIONS_CUSTOM:
        if (!pubPath.isEmpty()) {
            makeTree(PUBLICATIONS_CUSTOM);
            ui->pub_graph_stackedWidget->hide();
            ui->pubGraphTitle->clear();
        }
        break;
    }

}


int MainWindow::checkFile(int index, QString filePath, bool skip_prompt)
{
    CSVReader reader;
    std::vector<std::string> header;
    std::string searchstring, searchstring1;
    int sortHeaderIndex = 2;

    switch (index) {

    case TEACH:
        // read if first or if a new file is loaded
        if (teachPath.isEmpty() || (!teachPath.isEmpty() && filePath.compare(teachPath)) || teachFlag == false) {
            // create a new reader to read in the file
            reader = CSVReader(filePath.toStdString());
            header = reader.getHeaders();
            teachFlag = false;
            teachNames.clear();
            //Save session
            QFileIO teachSave("teachfile.dat");
            teachSave.savePath(filePath);
            // create a new manager for the data
            delete teachdb;
            teachdb = new RecordsManager(&header);
            // check for right file type by searching for unique header
            searchstring = "Program";
            searchstring1 = "Division";
            if (std::find(header.begin(), header.end(), searchstring) != header.end() || std::find(header.begin(), header.end(), searchstring1) != header.end()) {
                // load in data into the manager, with the date as the key
                sortHeaderIndex = teachdb->getHeaderIndex("Start Date");
                teachData = reader.getData();
                std::vector<std::vector<std::string>*> f_errs;
                unsigned int j;
                for (int i = 0; i < (int) teachData.size(); i++)
                {
                    for (j = 0; j < TEACH_MANFIELDS.size(); j++)
                    {
                        int index = teachdb->getHeaderIndex(TEACH_MANFIELDS[j]);
                        if (teachData[i][index].compare("") == 0)
                        {
                            f_errs.push_back(&teachData[i]);
                            break;
                        }
                    }

                    // if all mandatory fields are okay
                    if (j == TEACH_MANFIELDS.size())
                    {
                        // date interpretation
                        int yrIndex = teachdb->getHeaderIndex("Start Date");
                        int headerIndex = teachdb->getHeaderIndex("Member Name");
                        int year;
                        sscanf(teachData[i][yrIndex].c_str(), "%4d", &year);
                        teachData[i][yrIndex] = std::to_string(year);
                        if(std::find(teachNames.begin(), teachNames.end(), teachData[i][headerIndex]) == teachNames.end())
                            teachNames.push_back(teachData[i][headerIndex]);
                        teachdb->addRecord(reader.parseDateString(teachData[i][sortHeaderIndex]), &teachData[i]);
                    }
                }

                if (f_errs.size() > 0) {
                    if(handle_field_errors(f_errs, header, TEACH_MANFIELDS, filePath, skip_prompt)) {
                        for (unsigned int i = 0; i < f_errs.size(); i++) {
                            teachdb->addRecord(reader.parseDateString((*(f_errs[i]))[sortHeaderIndex]), f_errs[i]);
                        }
                    }
                }
            } else {
                return EXIT_FAILURE;
            }
        } else {
            return EXIT_SUCCESS;
        }


        ui->teachPrintButton->setEnabled(true);
        ui->teachExportButton->setEnabled(true);
        break;

    case PUBLICATIONS:
        // read if first or if a new file is loaded
        if (presPath.isEmpty() || (!presPath.isEmpty() && filePath.compare(presPath))) {
            // create a new reader to read in the file
            reader = CSVReader(filePath.toStdString());
            header = reader.getHeaders();
            pubFlag = false;
            pubNames.clear();
            //Save session
            QFileIO pubsave("pubfile.dat");
            pubsave.savePath(filePath);
            // create a new manager for the data
            delete pubdb;
            pubdb = new RecordsManager(&header);

            // check for right file type by searching for unique header
            searchstring = "Publication Status";
            if (std::find(header.begin(), header.end(), searchstring) != header.end()) {
                // load in data into the manager, with the date as the key
                sortHeaderIndex = pubdb->getHeaderIndex("Status Date");
                pubData = reader.getData();
                std::vector<std::vector<std::string>*> f_errs;
                unsigned int j;
                for (int i = 0; i < (int) pubData.size(); i++) {
                    for (j = 0; j < PUBS_MANFIELDS.size(); j++) {
                        int index = pubdb->getHeaderIndex(PUBS_MANFIELDS[j]);
                        if (pubData[i][index].compare("") == 0) {
                            f_errs.push_back(&pubData[i]);
                            break;
                        }
                    }

                    // if all mandatory fields are okay
                    if (j == PUBS_MANFIELDS.size()) {
                        // date interpretation
                        int yrIndex = pubdb->getHeaderIndex("Status Date");
                        int headerIndex = pubdb->getHeaderIndex("Member Name");
                        int year;
                        sscanf(pubData[i][yrIndex].c_str(), "%4d", &year);
                        pubData[i][yrIndex] = std::to_string(year);
                        if(std::find(pubNames.begin(), pubNames.end(), pubData[i][headerIndex]) == pubNames.end())
                            pubNames.push_back(pubData[i][headerIndex]);
                        pubdb->addRecord(reader.parseDateString(pubData[i][sortHeaderIndex]), &pubData[i]);
                    }
                }

                if (f_errs.size() > 0) {
                    if(handle_field_errors(f_errs, header, PUBS_MANFIELDS, filePath, skip_prompt)) {
                        for (unsigned int i = 0; i < f_errs.size(); i++) {
                            pubdb->addRecord(reader.parseDateString((*(f_errs[i]))[sortHeaderIndex]), f_errs[i]);
                        }
                    }
                }
            } else {
                return EXIT_FAILURE;
            }
        } else {
            return EXIT_SUCCESS;
        }
        ui->pubPrintButton->setEnabled(true);
        ui->pubExportButton->setEnabled(true);
        break;

    case PRESENTATIONS:
        // read if first or if a new file is loaded
        if (presPath.isEmpty() || (!presPath.isEmpty() && filePath.compare(presPath))) {
            // create a new reader to read in the file
            reader = CSVReader(filePath.toStdString());
            header = reader.getHeaders();
            presNames.clear();
            presFlag = false;
            //Save session
            QFileIO presSave("presfile.dat");
            presSave.savePath(filePath);
            // create a new manager for the data
            delete presdb;
            presdb = new RecordsManager(&header);

            // check for right file type by searching for unique header
            searchstring = "Activity Type";
            if (std::find(header.begin(), header.end(), searchstring) != header.end()) {
                // load in data into the manager, with the date as the key
                sortHeaderIndex = presdb->getHeaderIndex("Date");
                presData = reader.getData();
                std::vector<std::vector<std::string>*> f_errs;
                unsigned int j = 0;
                for (int i = 0; i < (int) presData.size(); i++) {
                    //check through mandatory fields for empty fields
                    for (j = 0; j < PRES_MANFIELDS.size(); j++) {
                        int index = presdb->getHeaderIndex(PRES_MANFIELDS[j]);
                        if (presData[i][index].compare("") == 0) {
                            f_errs.push_back(&presData[i]);
                            break;
                        }
                    }

                    // if all mandatory fields are okay
                    if (j == PRES_MANFIELDS.size()) {
                        // date interpretation
                        int yrIndex = presdb->getHeaderIndex("Date");
                        int headerIndex = presdb->getHeaderIndex("Member Name");
                        int year;
                        sscanf(presData[i][yrIndex].c_str(), "%4d", &year);
                        presData[i][yrIndex] = std::to_string(year);
                        if(std::find(presNames.begin(), presNames.end(), presData[i][headerIndex]) == presNames.end())
                            presNames.push_back(presData[i][headerIndex]);
                        presdb->addRecord(reader.parseDateString(presData[i][sortHeaderIndex]), &presData[i]);
                    }
                }

                if (f_errs.size() > 0) {
                    if(handle_field_errors(f_errs, header, PRES_MANFIELDS, filePath, skip_prompt)) {
                        for (unsigned int i = 0; i < f_errs.size(); i++) {
                            presdb->addRecord(reader.parseDateString((*(f_errs[i]))[sortHeaderIndex]), f_errs[i]);
                        }
                    }
                }
            } else {
                return EXIT_FAILURE;
            }
        } else {
            return EXIT_SUCCESS;
        }
        ui->presPrintButton->setEnabled(true);
        ui->presExportButton->setEnabled(true);
        break;

    case FUNDING:
        // read if first or if a new file is loaded
        if (fundPath.isEmpty() || (!fundPath.isEmpty() && filePath.compare(fundPath))) {
            // create a new reader to read in the file
            reader = CSVReader(filePath.toStdString());
            header = reader.getHeaders();
            fundNames.clear();
            fundFlag = false;
            //Save session
            QFileIO fundSave("fundfile.dat");
            fundSave.savePath(filePath);
            // create a new manager for the data
            delete funddb;
            funddb = new RecordsManager(&header);

            // check for right file type by searching for unique header
            searchstring = "Funding Type";

            if (std::find(header.begin(), header.end(), searchstring) != header.end()) {
                // load in data into the manager, with the date as the key
                sortHeaderIndex = funddb->getHeaderIndex("Start Date");
                fundData = reader.getData();
                std::vector<std::vector<std::string>*> f_errs;
                unsigned int j;
                for (int i = 0; i < (int) fundData.size(); i++) {
                    for (j = 0; j < GRANTS_MANFIELDS.size(); j++) {
                        int index = funddb->getHeaderIndex(GRANTS_MANFIELDS[j]);
                        if (fundData[i][index].compare("") == 0) {
                            f_errs.push_back(&fundData[i]);
                            break;
                        }
                    }

                    // if all mandatory fields are okay
                    if (j == GRANTS_MANFIELDS.size()) {
                        // date interpretation
                        int yrIndex = funddb->getHeaderIndex("Start Date");
                        int headerIndex = funddb->getHeaderIndex("Member Name");
                        int year;
                        sscanf(fundData[i][yrIndex].c_str(), "%4d", &year);
                        fundData[i][yrIndex] = std::to_string(year);

                        // boolean interpretation
                        int prIndex = funddb->getHeaderIndex("Peer Reviewed?");
                        if (fundData[i][prIndex].compare("True") == 0) {
                            fundData[i][prIndex] = "Peer Reviewed";
                        } else {
                            fundData[i][prIndex] = "Not Peer Reviewed";
                        }
                        if(std::find(fundNames.begin(), fundNames.end(), fundData[i][headerIndex]) == fundNames.end())
                            fundNames.push_back(fundData[i][headerIndex]);
                        funddb->addRecord(reader.parseDateString(fundData[i][sortHeaderIndex]), &fundData[i]);
                    }
                }
                if (f_errs.size() > 0) {
                    if(handle_field_errors(f_errs, header, GRANTS_MANFIELDS, filePath, skip_prompt)) {
                        for (unsigned int i = 0; i < f_errs.size(); i++) {
                            funddb->addRecord(reader.parseDateString((*(f_errs[i]))[sortHeaderIndex]), f_errs[i]);
                        }
                    }
                }
            } else {
                return EXIT_FAILURE;
            }
        } else {
            return EXIT_SUCCESS;
        }
        ui->fundPrintButton->setEnabled(true);
        ui->fundExportButton->setEnabled(true);
        break;
    }
    return EXIT_SUCCESS;
}

void MainWindow::createDefaultSortOrder(int tabIndex)
{
    QStringList defaultOrder;
    defaultOrder << "Default";

    switch (tabIndex) {
    case FUNDING:
        // specify default sort order
        defaultOrder << "Member Name" << "Funding Type" << "Peer Reviewed?" << "Status" << "Role" << "Title";

        // add default list to member variable
        allFundOrders << defaultOrder;

    {
        // save the default for the user
        QSortListIO saveFundSort(FUNDORDER_SAVE);
        saveFundSort.saveList(allFundOrders);
    }

        break;
    case PRESENTATIONS:
        // specify default sort order
        defaultOrder << "Member Name" << "Type" << "Role" << "Title";

        // add default list to member variable
        allPresOrders << defaultOrder;

    {
        // save the default for the user
        QSortListIO savePresSort(PRESORDER_SAVE);
        savePresSort.saveList(allPresOrders);
    }

        break;
    case PUBLICATIONS:
        // specify default sort order
        defaultOrder << "Member Name" << "Type" << "Role" << "Title";

        // add default list to member variable
        allPubOrders << defaultOrder;

    {
        // save the default for the user
        QSortListIO savePubSort(PUBORDER_SAVE);
        savePubSort.saveList(allPubOrders);
    }

        break;
    case TEACH:
        // specify default sort order
        defaultOrder << "Member Name" << "Program";

        // add default list to member variable
        allTeachOrders << defaultOrder;

    {
        // save the default for the user
        QSortListIO saveTeachSort(FUNDORDER_SAVE);
        saveTeachSort.saveList(allFundOrders);
    }

        break;
    }
}

/*
 * err: vector of pointers to record entries.
 * headers: vector of strings containing the db headers
 * mandatory: vector of strings containing the mandatory fields in the db
 *
 * Returns true when the passed err vector contains edited fields.
 * Returns false when no fields have been edited.
 */
bool MainWindow::handle_field_errors(std::vector<std::vector<std::string>*>& err,
                                     std::vector<std::string>& headers,
                                     std::vector<std::string>& mandatory,
                                     QString &type,
                                     bool skip_prompt) {

    //Since CSVReader alldata contains completely empty records
    //remove them first.
    if(skip_prompt){
        return false;
    }
    else{
        std::vector<std::vector<std::string>*>::iterator it;
        for (it = err.begin(); it != err.end(); it++) {
            bool allEmpty = true;
            for (int col = 0; col < (int) (*it)->size(); col++) {
                if ((*it)->at(col).compare("") != 0) {
                    allEmpty = false;
                }
            }
            if (allEmpty) {
                it = err.erase(it);
                it--;
            }
        }
        //Return false; there are no errors to correct
        if (err.size() == 0) {
            return false;
        }
        QMessageBox prompt;
        QString mainText = "File contains ";
        mainText.append(QString::number(err.size()));
        mainText.append(" records with missing mandatory fields.");
        prompt.setText(mainText);
        prompt.setInformativeText("Do you want to edit these entries or discard?");
        prompt.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        prompt.setDefaultButton(QMessageBox::Yes);
        prompt.setButtonText(QMessageBox::Yes, "Edit");
        prompt.setButtonText(QMessageBox::No, "Discard");
        prompt.setWindowIcon(QIcon(":/icon32.ico"));
        int ret = prompt.exec();
        switch (ret) {
        case QMessageBox::Yes: {
            ErrorEditDialog diag(this, err, headers, mandatory, type);

            if(diag.exec()) {
                return true;
            }
            return false;
        }

        case QMessageBox::No:
        default:
            return false;
        }
    }
}

/*
 * Builds a TreeModel for the ui's embedded treeview.
 */
void MainWindow::makeTree(int tabIndex) {
    switch (tabIndex) {
    case TEACH:
        // set up some member variables to point to the current data in use
        currentdb = teachdb;
        delete teachTree;
        teachTree = new TeachingTreeModel(currentdb);
        currentTree = teachTree;

        // get some UI elements so we can put the tree and graphs into them
        currentView = ui->teachTreeView;

        // analyze the data into a tree
        currentTree->setupModel(yearStart, yearEnd, teachSortOrder, getFilterStartChar(TEACH), getFilterEndChar(TEACH));

        ui->teach_pie_button->toggle();

        break;
    case TEACH_CUSTOM:
        currentdb = teachdb2;
        delete teachTree;
        teachTree = new TeachingTreeModel(currentdb);
        currentTree = teachTree;
        currentView = ui->teachTreeView;
        currentTree->setupModel(yearStart, yearEnd, teachSortOrder, getFilterStartChar(TEACH), getFilterEndChar(TEACH));

        ui->teach_pie_button->toggle();

        break;
    case PUBLICATIONS:
        // set up some member variables to point to the current data in use
        currentdb = pubdb;
        delete pubTree;
        pubTree = new PublicationTreeModel(currentdb);
        currentTree = pubTree;

        // get some UI elements so we can put the tree and graphs into them
        currentView = ui->pubTreeView;

        // analyze the data into a tree
        currentTree->setupModel(yearStart, yearEnd, pubSortOrder, getFilterStartChar(PUBLICATIONS), getFilterEndChar(PUBLICATIONS));

        ui->pub_pie_button->toggle();

        break;
    case PUBLICATIONS_CUSTOM:
        currentdb = pubdb2;
        delete pubTree;
        pubTree = new PublicationTreeModel(currentdb);
        currentTree = pubTree;

        // get some UI elements so we can put the tree and graphs into them
        currentView = ui->pubTreeView;

        // analyze the data into a tree
        currentTree->setupModel(yearStart, yearEnd, pubSortOrder, getFilterStartChar(PUBLICATIONS), getFilterEndChar(PUBLICATIONS));

        ui->pub_pie_button->toggle();

        break;
    case PRESENTATIONS:
        // set up some member variables to point to the current data in use
        currentdb = presdb;
        delete presTree;

        presTree = new PresentationTreeModel(currentdb);
        currentTree = presTree;

        // get some UI elements so we can put the tree and graphs into them
        currentView = ui->presTreeView;

        // analyze the data into a tree
        currentTree->setupModel(yearStart, yearEnd, presSortOrder, getFilterStartChar(PRESENTATIONS), getFilterEndChar(PRESENTATIONS));

        ui->pres_pie_button->toggle();

        break;
    case PRESENTATIONS_CUSTOM:
        currentdb = presdb2;
        delete presTree;

        presTree = new PresentationTreeModel(currentdb);
        currentTree = presTree;

        // get some UI elements so we can put the tree and graphs into them
        currentView = ui->presTreeView;

        // analyze the data into a tree
        currentTree->setupModel(yearStart, yearEnd, presSortOrder, getFilterStartChar(PRESENTATIONS), getFilterEndChar(PRESENTATIONS));

        ui->pres_pie_button->toggle();

        break;

    case FUNDING:
        // set up some member variables to point to the current data in use
        currentdb = funddb;
        delete fundTree;
        fundTree = new GrantFundingTreeModel(currentdb);
        currentTree = fundTree;

        // get some UI elements so we can put the tree and graphs into them
        currentView = ui->fundTreeView;

        // analyze the data into a tree
        currentTree->setupModel(yearStart, yearEnd, fundSortOrder, getFilterStartChar(FUNDING), getFilterEndChar(FUNDING));

        ui->fund_pie_button->toggle();

        break;

    case FUNDING_CUSTOM:
        currentdb = funddb2;
        delete fundTree;
        fundTree = new GrantFundingTreeModel(currentdb);
        currentTree = fundTree;

        // get some UI elements so we can put the tree and graphs into them
        currentView = ui->fundTreeView;

        // analyze the data into a tree
        currentTree->setupModel(yearStart, yearEnd, fundSortOrder, getFilterStartChar(FUNDING), getFilterEndChar(FUNDING));

        ui->fund_pie_button->toggle();

        break;
    }

    // set up the tree in our UI
    currentView->setModel(currentTree);

    // set resize property to stretch
    currentView->header()->resizeSections(QHeaderView::Stretch);
}

void MainWindow::setupPieChart(PieChartWidget* pieChart, QListWidget *pieListWidget, std::vector<std::pair <std::string, double>> pieChartList) {
    // draws the pie graph by sending piechartwidget a vector of name, presentation count
    int pieSize = (int) pieChartList.size();
    QVector<QColor> colorList(pieSize);
    pieListWidget->clear();
    for (int i = 0; i < pieSize; i++) {
        colorList[i] = (QColor(qrand() % 256, qrand() % 256, qrand() % 256));
        pieListWidget->addItem(QString::fromStdString(pieChartList[i].first));

        // set legend colors
        QPixmap pixmap(100, 100);
        pixmap.fill(QColor(colorList[i]));
        QIcon tempIcon(pixmap);
        pieListWidget->item(i)->setIcon(tempIcon);
    }

    pieChart->setData(pieChartList, colorList); //passes vector list to piechartwidget
}


void MainWindow::setupBarChart(QCustomPlot *barChart, std::vector<std::pair <std::string, double>> barChartList) {
    // create empty bar chart objects:
    QCPBars *yLabels = new QCPBars(barChart->yAxis, barChart->xAxis);
    barChart->addPlottable(yLabels);

    // set names and colors:
    QPen pen;
    pen.setWidthF(1.2);
    yLabels->setName("Type");
    pen.setColor(QColor(255, 131, 0));
    yLabels->setPen(pen);
    yLabels->setBrush(QColor(255, 131, 0, 50));

    //get label list
    int barSize = (int) barChartList.size();
    double maxCount = 0;
    double scaledCount;
    QVector<double> ticks;
    QVector<QString> ylabels;
    QVector<double> count;

    //add label list to y axis labels
    for (int i = 0; i < barSize; i++){
        ticks << (i+1);
        ylabels << QString::fromStdString(barChartList[i].first);
        if (barChartList[i].second>1000000){
            scaledCount = barChartList[i].second/1000000;
        } else if (barChartList[i].second>1000){
            scaledCount = barChartList[i].second/1000;
        } else{
            scaledCount = barChartList[i].second;
        }
        count <<scaledCount;

        if (maxCount < barChartList[i].second)
            maxCount = barChartList[i].second;
    }

    //setup Y Axis
    barChart->yAxis->setAutoTicks(false);
    barChart->yAxis->setAutoTickLabels(false);
    barChart->yAxis->setTickVector(ticks);
    barChart->yAxis->setTickVectorLabels(ylabels);
    barChart->yAxis->setTickLabelPadding(1);
    barChart->yAxis->setSubTickCount(0);
    barChart->yAxis->setTickLength(0, 1);
    barChart->yAxis->grid()->setVisible(true);
    barChart->yAxis->setRange(0, barSize+1);

    if(maxCount>1000000){
        maxCount = maxCount/1000000;
        barChart->xAxis->setLabel("Total (in Millions)");
    }else if (maxCount>1000){
        maxCount = maxCount/1000;
        barChart->xAxis->setLabel("Total (in Thousands)");
    }else{
        barChart->xAxis->setLabel("Total");
    }

    // setup X Axis
    barChart->xAxis->setAutoTicks(true);
    barChart->xAxis->setRange(0,maxCount+(maxCount*.05));
    barChart->xAxis->setAutoTickLabels(true);
    barChart->xAxis->setAutoTickStep(true);
    barChart->xAxis->grid()->setSubGridVisible(true);

    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(QColor(0, 0, 0, 25));
    barChart->xAxis->grid()->setPen(gridPen);
    gridPen.setStyle(Qt::DotLine);
    barChart->xAxis->grid()->setSubGridPen(gridPen);

    yLabels->setData(ticks, count);
}

void MainWindow::setupLineChart(QCustomPlot *lineChart, std::vector<std::pair <std::string, double>> lineChartList) {

    double maxCount = 0;
    double minYear = std::stod(lineChartList[0].first);
    double maxYear = std::stod(lineChartList[((int) lineChartList.size()) -1].first);
    double numYears = maxYear - minYear + 1;

    QVector<double> x(numYears, 0), y(numYears, 0);
    for(int i = 0; i < numYears; i++) x[i] = minYear + i;
    for(int i = 0; i < (int) lineChartList.size(); i++)
    {
        x[std::stoi(lineChartList[i].first) - minYear] = std::stod(lineChartList[i].first);
        y[std::stoi(lineChartList[i].first) - minYear] = lineChartList[i].second;
        if (maxCount < lineChartList[i].second) maxCount = lineChartList[i].second;
    }

    lineChart->addGraph();
    lineChart->graph(0)->setData(x, y);
    lineChart->graph(0)->setPen((QColor(qrand() % 256, qrand() % 256, qrand() % 256)));

    lineChart->xAxis->setLabel("Year");
    lineChart->xAxis->setRange(minYear, maxYear);
    lineChart->xAxis->setAutoTickStep(false);
    lineChart->xAxis->setSubTickCount(0);
    lineChart->xAxis->setTickStep(1);
    lineChart->yAxis->setRange(0, maxCount);
}

void MainWindow::setupScatterPlot(QCustomPlot *scatterPlot, std::vector<std::pair <std::string, double>> scatterPlotList) {

    double minMoney = std::numeric_limits<double>::max();
    double maxMoney = 0;
    double minFundings = std::numeric_limits<double>::max();
    double maxFundings = 0;

    int listSize = (int) scatterPlotList.size();
    QVector<double> x(listSize), y(listSize);

    for(int i = 0; i < (int) scatterPlotList.size(); i++)
    {
        double first = std::stod(scatterPlotList[i].first);
        double second = log(scatterPlotList[i].second);

        x[i] = first;
        y[i] = second;

        // Update axis limits
        if (minMoney > second) minMoney = second;
        if (maxMoney < second) maxMoney = second;
        if (maxFundings < first) maxFundings =  first;
        if (minFundings > first) minFundings =  first;
    }

    qSort(x);
    qSort(y);

    scatterPlot->addGraph();
    scatterPlot->graph(0)->setData(x, y);
    scatterPlot->graph(0)->setPen(QPen((QColor(0, 0, 255)),4));
    scatterPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    scatterPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross));

    scatterPlot->xAxis->setLabel("Number of times funded");
    scatterPlot->yAxis->setLabel("log of the Amount of funding in CAD");
    scatterPlot->xAxis->setRange(minFundings-0.5*(x[1]-x[0]), maxFundings+-0.5*(x[1]-x[0]));
    scatterPlot->yAxis->setRange(minMoney-0.5*(y[1]-y[0]), maxMoney+0.5*(y[1]-y[0]));
    scatterPlot->xAxis->setAutoTickStep(false);
    scatterPlot->xAxis->setSubTickCount(0);
    scatterPlot->xAxis->setTickStep(1);
}

void MainWindow::setupHistogramChart(QCustomPlot *histogramChart, std::vector<std::pair <std::string, double>> histogramChartList){

    QVector<double> ticks,count;

    int histogramSize = (int) histogramChartList.size();
    double scaledCount;
    double maxCount = 0;
    QVector<QString> xlabels;
    //add label list to y axis labels
          for (int i = 0; i < histogramSize; i++){
              ticks << (i+1);
              xlabels << QString::fromStdString(histogramChartList[i].first);
              //qDebug() <<  QString::fromStdString(histogramChartList[i].first);
              if (histogramChartList[i].second>1000000){
                  scaledCount = histogramChartList[i].second/1000000;
              } else if (histogramChartList[i].second>1000){
                  scaledCount = histogramChartList[i].second/1000;
              } else{
                  scaledCount = histogramChartList[i].second;
              }
              count <<scaledCount;

              if (maxCount < histogramChartList[i].second)
                  maxCount = histogramChartList[i].second;
          }

    QCPBars *bars1 = new QCPBars(histogramChart->xAxis, histogramChart->yAxis);
    histogramChart->addPlottable(bars1);


    bars1->addData(ticks,count);

    if(maxCount>1000000){
        maxCount = maxCount/1000000;
        histogramChart->xAxis->setLabel("Total (in Millions)");
    }else if (maxCount>1000){
        maxCount = maxCount/1000;
        histogramChart->xAxis->setLabel("Total (in Thousands)");
    }else{
        histogramChart->xAxis->setLabel("Total");
    }


    //setup Y Axis
    histogramChart->xAxis->setAutoTicks(false);
    histogramChart->xAxis->setAutoTickLabels(false);
    histogramChart->xAxis->setTickVector(ticks);
    histogramChart->xAxis->setTickVectorLabels(xlabels);
    histogramChart->xAxis->setTickLabelPadding(1);
    histogramChart->xAxis->setSubTickCount(0);
    histogramChart->xAxis->setTickLength(0, 1);
    histogramChart->xAxis->grid()->setVisible(true);
    histogramChart->xAxis->setRange(0, histogramSize+3);
    histogramChart->xAxis->setTickLabelRotation(30);


    histogramChart->yAxis->setAutoTicks(true);
    histogramChart->yAxis->setRange(0,maxCount+(maxCount*.05));
    histogramChart->yAxis->setAutoTickLabels(true);
    histogramChart->yAxis->setAutoTickStep(true);
   // histogramChart->yAxis->grid()->setSubGridVisible(true);


}

void MainWindow::on_teach_new_sort_clicked() {
    if (teachdb != NULL) {
        CustomSort* sortdialog = new CustomSort();
        sortdialog->setFields(TEACH_MANFIELDS);
        int ret = sortdialog->exec();
        if (ret)
        {
            QStringList newSortOrder = sortdialog->getSortFields();
            allTeachOrders << newSortOrder;
            ui->teach_sort->addItem(newSortOrder.at(0));
            qDebug() <<  QString::fromStdString("****new sort order***");
            qDebug() << newSortOrder;
            qDebug() <<  QString::fromStdString("****new sort order***");


            // save the sort fields to file
            QSortListIO saveTeachSort(TEACHORDER_SAVE);
            saveTeachSort.saveList(allTeachOrders);
        }
        delete sortdialog;
    } else {
        QMessageBox::critical(this, "Missing File", "Please load a file first.");
    }
}

void MainWindow::on_teach_edit_sort_clicked(){
    if (teachdb != NULL) {

        EditSort* sortdialog = new EditSort();
        sortdialog->setFields(teachSortOrder);

        int ret = sortdialog->exec();
        if (ret)
        {
            QStringList editSortOrder;
            editSortOrder << ui->teach_sort->currentText();
            editSortOrder << sortdialog->getSortFields();

           int currentIndex =  ui->teach_sort->currentIndex();
           allTeachOrders.replace(currentIndex,editSortOrder);

            // save the sort fields to file
           QSortListIO saveTeachSort(TEACHORDER_SAVE);
           saveTeachSort.saveList(allTeachOrders);
           refresh(TEACH);
        }
        delete sortdialog;

    } else {
        QMessageBox::critical(this, "Missing File", "Please load a file first.");
    }
}


void MainWindow::on_pub_new_sort_clicked() {
    if (pubdb != NULL) {
        CustomSort* sortdialog = new CustomSort();
        sortdialog->setFields(PUBS_MANFIELDS);
        int ret = sortdialog->exec();
        if (ret) {
            QStringList newSortOrder = sortdialog->getSortFields();
            allPubOrders << newSortOrder;
            ui->pub_sort->addItem(newSortOrder.at(0));

            // save the sort fields to file
            QSortListIO savePubSort(PUBORDER_SAVE);
            savePubSort.saveList(allPubOrders);
        }
        delete sortdialog;
    } else {
        QMessageBox::critical(this, "Missing File", "Please load a file first.");
    }
}

void MainWindow::on_pub_edit_sort_clicked(){
    if (pubdb != NULL) {
        EditSort* sortdialog = new EditSort();
        sortdialog->setFields(pubSortOrder);

        int ret = sortdialog->exec();
        if (ret)
        {
            QStringList editSortOrder;
            editSortOrder << ui->pub_sort->currentText();
            editSortOrder << sortdialog->getSortFields();

           int currentIndex =  ui->pub_sort->currentIndex();
           allPubOrders.replace(currentIndex,editSortOrder);

            // save the sort fields to file
           QSortListIO savePubSort(PUBORDER_SAVE);
           savePubSort.saveList(allPubOrders);
           refresh(PUBLICATIONS);
        }
        delete sortdialog;

    } else {
        QMessageBox::critical(this, "Missing File", "Please load a file first.");
    }

}



void MainWindow::on_pres_new_sort_clicked() {
    if (presdb != NULL) {
        CustomSort* sortdialog = new CustomSort();
        sortdialog->setFields(PRES_MANFIELDS);
        int ret = sortdialog->exec();
        if (ret) {
            QStringList newSortOrder = sortdialog->getSortFields();
            allPresOrders << newSortOrder;
            ui->pres_sort->addItem(newSortOrder.at(0));

            // save the sort fields to file
            QSortListIO savePresSort(PRESORDER_SAVE);
            savePresSort.saveList(allPresOrders);
        }
        delete sortdialog;
    } else {
        QMessageBox::critical(this, "Missing File", "Please load a file first.");
    }
}

void MainWindow::on_pres_edit_sort_clicked(){
    if (presdb != NULL) {

        EditSort* sortdialog = new EditSort();
        sortdialog->setFields(presSortOrder);

        int ret = sortdialog->exec();
        if (ret)
        {
            QStringList editSortOrder;
            editSortOrder << ui->pres_sort->currentText();
            editSortOrder << sortdialog->getSortFields();

           int currentIndex =  ui->pres_sort->currentIndex();
           allPresOrders.replace(currentIndex,editSortOrder);

            // save the sort fields to file
           QSortListIO savePresSort(PRESORDER_SAVE);
           savePresSort.saveList(allPresOrders);
           refresh(PRESENTATIONS);
        }
        delete sortdialog;

    } else {
        QMessageBox::critical(this, "Missing File", "Please load a file first.");
    }

}

void MainWindow::on_fund_new_sort_clicked() {
    if (funddb != NULL) {
        CustomSort* sortdialog = new CustomSort();
        sortdialog->setFields(GRANTS_MANFIELDS);
        int ret = sortdialog->exec();
        if (ret) {
            QStringList newSortOrder = sortdialog->getSortFields();
            allFundOrders << newSortOrder;
            ui->fund_sort->addItem(newSortOrder.at(0));

            // save the sort fields to file
            QSortListIO saveFundSort(FUNDORDER_SAVE);
            saveFundSort.saveList(allFundOrders);
        }
        delete sortdialog;
    } else {
        QMessageBox::critical(this, "Missing File", "Please load a file first.");
    }
}


void MainWindow::on_fund_edit_sort_clicked(){
    if (presdb != NULL) {

        EditSort* sortdialog = new EditSort();
        sortdialog->setFields(fundSortOrder);

        int ret = sortdialog->exec();
        if (ret)
        {
            QStringList editSortOrder;
            editSortOrder << ui->fund_sort->currentText();
            editSortOrder << sortdialog->getSortFields();

           int currentIndex =  ui->fund_sort->currentIndex();
           allFundOrders.replace(currentIndex,editSortOrder);

            // save the sort fields to file
           QSortListIO savePresSort(FUNDORDER_SAVE);
           savePresSort.saveList(allFundOrders);
           refresh(FUNDING);
        }
        delete sortdialog;

    } else {
        QMessageBox::critical(this, "Missing File", "Please load a file first.");
    }
}


void MainWindow::on_teach_sort_currentIndexChanged(int index) {
    if(index != -1) {

        QStringList sortOrder = allTeachOrders[index];
        teachSortOrder.clear();
        for (int i = 1; i < sortOrder.size(); i++) {
            teachSortOrder.emplace_back(sortOrder[i].toStdString());
        }
        ui->teach_filter->setText(QString::fromStdString(teachSortOrder[0]));
        if(teachFlag == true)
            refresh(TEACH_CUSTOM);
        else
            refresh(TEACH);
    }
}

void MainWindow::on_pub_sort_currentIndexChanged(int index) {
    if(index != -1) {
        QStringList sortOrder = allPubOrders[index];
        pubSortOrder.clear();
        for (int i = 1; i < sortOrder.size(); i++) {
            pubSortOrder.emplace_back(sortOrder[i].toStdString());
        }
        ui->pub_filter->setText(QString::fromStdString(pubSortOrder[0]));
        if(pubFlag == true)
            refresh(PUBLICATIONS_CUSTOM);
        else
            refresh(PUBLICATIONS);
    }
}

void MainWindow::on_pres_sort_currentIndexChanged(int index) {
    if(index != -1) {
        QStringList sortOrder = allPresOrders[index];
        presSortOrder.clear();
        for (int i = 1; i < sortOrder.size(); i++) {
            presSortOrder.emplace_back(sortOrder[i].toStdString());
        }
        ui->pres_filter->setText(QString::fromStdString(presSortOrder[0]));
        if(presFlag == true)
            refresh(PRESENTATIONS_CUSTOM);
        else
            refresh(PRESENTATIONS);
    }
}

void MainWindow::on_fund_sort_currentIndexChanged(int index) {
    if(index != -1) {
        QStringList sortOrder = allFundOrders[index];
        fundSortOrder.clear();
        for (int i = 1; i < sortOrder.size(); i++) {
            fundSortOrder.emplace_back(sortOrder[i].toStdString());
        }
        ui->fund_filter->setText(QString::fromStdString(fundSortOrder[0]));
        if(fundFlag == true)
            refresh(FUNDING_CUSTOM);
        else
            refresh(FUNDING);
    }
}

void MainWindow::on_teach_delete_sort_clicked() {
    if (ui->teach_sort->currentIndex()!=0) {
        QMessageBox prompt;
        prompt.setText("Are you sure you want to delete " + ui->teach_sort->currentText() + "?");
        prompt.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        if (prompt.exec()==QMessageBox::Yes) {
            allTeachOrders.removeAt(ui->teach_sort->currentIndex());
            ui->teach_sort->removeItem(ui->teach_sort->currentIndex());

            QSortListIO saveTeachSort(TEACHORDER_SAVE);
            saveTeachSort.saveList(allTeachOrders);
        }
    } else {
        QMessageBox::critical(this, "", "Cannot delete Default");
    }
}

void MainWindow::on_pub_delete_sort_clicked() {
    if (ui->pub_sort->currentIndex()!=0) {
        QMessageBox prompt;
        prompt.setText("Are you sure you want to delete " + ui->pub_sort->currentText() + "?");
        prompt.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        if (prompt.exec()==QMessageBox::Yes) {
            allPubOrders.removeAt(ui->pub_sort->currentIndex());
            ui->pub_sort->removeItem(ui->pub_sort->currentIndex());

            QSortListIO savePubSort(PUBORDER_SAVE);
            savePubSort.saveList(allPubOrders);
        }
    } else {
        QMessageBox::critical(this, "", "Cannot delete Default");
    }
}

void MainWindow::on_pres_delete_sort_clicked() {
    if (ui->pres_sort->currentIndex()!=0) {
        QMessageBox prompt;
        prompt.setText("Are you sure you want to delete " + ui->pres_sort->currentText() + "?");
        prompt.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        if (prompt.exec()==QMessageBox::Yes) {
            allPresOrders.removeAt(ui->pres_sort->currentIndex());
            ui->pres_sort->removeItem(ui->pres_sort->currentIndex());

            QSortListIO savePresSort(PRESORDER_SAVE);
            savePresSort.saveList(allPresOrders);
        }
    } else {
        QMessageBox::critical(this, "", "Cannot delete Default");
    }
}

void MainWindow::on_fund_delete_sort_clicked() {
    if (ui->fund_sort->currentIndex()!=0) {
        QMessageBox prompt;
        prompt.setText("Are you sure you want to delete " + ui->fund_sort->currentText() + "?");
        prompt.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        if (prompt.exec()==QMessageBox::Yes) {
            allFundOrders.removeAt(ui->fund_sort->currentIndex());
            ui->fund_sort->removeItem(ui->fund_sort->currentIndex());

            QSortListIO saveFundSort(FUNDORDER_SAVE);
            saveFundSort.saveList(allFundOrders);
        }
    } else {
        QMessageBox::critical(this, "", "Cannot delete Default");
    }
}

void MainWindow::on_teach_line_button_toggled() { ui->teach_graph_stackedWidget->setCurrentIndex(2);}
void MainWindow::on_teach_bar_button_toggled() { ui->teach_graph_stackedWidget->setCurrentIndex(1);}
void MainWindow::on_teach_pie_button_toggled() { ui->teach_graph_stackedWidget->setCurrentIndex(0);}

void MainWindow::on_pub_line_button_toggled() { ui->pub_graph_stackedWidget->setCurrentIndex(2);}
void MainWindow::on_pub_bar_button_toggled() { ui->pub_graph_stackedWidget->setCurrentIndex(1);}
void MainWindow::on_pub_pie_button_toggled() { ui->pub_graph_stackedWidget->setCurrentIndex(0);}

void MainWindow::on_pres_line_button_toggled() { ui->pres_graph_stackedWidget->setCurrentIndex(2);}
void MainWindow::on_pres_bar_button_toggled() { ui->pres_graph_stackedWidget->setCurrentIndex(1);}
void MainWindow::on_pres_pie_button_toggled() { ui->pres_graph_stackedWidget->setCurrentIndex(0);}

void MainWindow::on_fund_histogram_button_toggled(){ui->fund_graph_stackedWidget->setCurrentIndex(3);}
void MainWindow::on_fund_line_button_toggled() { ui->fund_graph_stackedWidget->setCurrentIndex(2);}
void MainWindow::on_fund_bar_button_toggled() { ui->fund_graph_stackedWidget->setCurrentIndex(1);}
void MainWindow::on_fund_pie_button_toggled() { ui->fund_graph_stackedWidget->setCurrentIndex(0);}

void MainWindow::on_teach_load_file_clicked() {
    QString path = load_file();
    if (!path.isEmpty()) {
        load_teach(path, true);
    }
}


bool MainWindow::load_teach(QString path, bool multi_file, bool skip_prompt)
{
    if (!checkFile(TEACH, path, skip_prompt))
    {
        // enable gui elements
        ui->teach_sort->setEnabled(true);
        ui->teach_delete_sort->setEnabled(true);
        ui->teach_new_sort->setEnabled(true);
        ui->teach_edit_sort->setEnabled(true);
        ui->teach_filter_from->setEnabled(true);
        ui->teach_filter_to->setEnabled(true);
        ui->teach_pie_button->setEnabled(true);
        ui->teach_bar_button->setEnabled(true);
        ui->teach_line_button->setEnabled(true);
        ui->teach_to_label->setEnabled(true);
        ui->teach_sort_label->setEnabled(true);
        ui->teach_filter->setEnabled(true);
        ui->teach_filter_label->setEnabled(true);
        ui->teachCustomList->setEnabled(true);

        // load save order
        QSortListIO teachSaveOrder(TEACHORDER_SAVE);
        allTeachOrders = teachSaveOrder.readList();
        ui->teach_sort->clear();
        for (int i = 0; i < allTeachOrders.size(); i++) {
            ui->teach_sort->addItem(allTeachOrders.at(i).at(0));
        }

        // create default sort order if none are loaded
        if (ui->teach_sort->currentIndex() < 0)
        {
            createDefaultSortOrder(TEACH);
            ui->teach_sort->addItem(allTeachOrders[0][0]);
        }

        // create the tree
        teachPath = path;
        makeTree(TEACH);
        ui->teach_file_label->setText(teachPath);

        return true;
    } else
    {
        if (!multi_file) {
            QMessageBox::critical(this, "Invalid File", "Not a valid teaching file.");
            on_teach_load_file_clicked();
        }
    }
    return false;
}

void MainWindow::on_pub_load_file_clicked() {
    QString path = load_file();
    if (!path.isEmpty()) {
        load_pub(path);
    }
}

bool MainWindow::load_pub(QString path, bool multi_file, bool skip_prompt) {
    if (!checkFile(PUBLICATIONS, path, skip_prompt)) {
        // enable gui elements
        ui->pub_sort->setEnabled(true);
        ui->pub_delete_sort->setEnabled(true);
        ui->pub_new_sort->setEnabled(true);
        ui->pub_edit_sort->setEnabled(true);
        ui->pub_filter_from->setEnabled(true);
        ui->pub_filter_to->setEnabled(true);
        ui->pub_pie_button->setEnabled(true);
        ui->pub_bar_button->setEnabled(true);
        ui->pub_line_button->setEnabled(true);
        ui->pub_to_label->setEnabled(true);
        ui->pub_sort_label->setEnabled(true);
        ui->pub_filter->setEnabled(true);
        ui->pub_filter_label->setEnabled(true);
        ui->pubCustomList->setEnabled(true);


        // load save order
        QSortListIO pubSaveOrder(PUBORDER_SAVE);
        allPubOrders = pubSaveOrder.readList();
        ui->pub_sort->clear();
        for (int i = 0; i < allPubOrders.size(); i++) {
            ui->pub_sort->addItem(allPubOrders.at(i).at(0));
        }

        // create default sort order if none are loaded
        if (ui->pub_sort->currentIndex() < 0) {
            createDefaultSortOrder(PUBLICATIONS);
            ui->pub_sort->addItem(allPubOrders[0][0]);
        }

        // create the tree
        pubPath = path;
        makeTree(PUBLICATIONS);
        ui->pub_file_label->setText(pubPath);




        return true;
    } else {
        if (!multi_file) {
            QMessageBox::critical(this, "Invalid File", "Not a valid publications file.");
            on_pub_load_file_clicked();
        }
    }
    return false;
}

void MainWindow::on_pres_load_file_clicked() {
    QString path = load_file();
    if (!path.isEmpty()) {
        load_pres(path);
    }
}

bool MainWindow::load_pres(QString path, bool multi_file, bool skip_prompt) {
    if (!checkFile(PRESENTATIONS, path, skip_prompt)) {
        // enable gui elements
        ui->pres_sort->setEnabled(true);
        ui->pres_delete_sort->setEnabled(true);
        ui->pres_new_sort->setEnabled(true);
        ui->pres_edit_sort->setEnabled(true);
        ui->pres_filter_from->setEnabled(true);
        ui->pres_filter_to->setEnabled(true);
        ui->pres_pie_button->setEnabled(true);
        ui->pres_bar_button->setEnabled(true);
        ui->pres_line_button->setEnabled(true);
        ui->pres_to_label->setEnabled(true);
        ui->pres_sort_label->setEnabled(true);
        ui->pres_filter->setEnabled(true);
        ui->pres_filter_label->setEnabled(true);
        ui->presCustomList->setEnabled(true);


        // load save order
        QSortListIO presSaveOrder(PRESORDER_SAVE);
        allPresOrders = presSaveOrder.readList();
        ui->pres_sort->clear();
        for (int i = 0; i < allPresOrders.size(); i++) {
            ui->pres_sort->addItem(allPresOrders.at(i).at(0));
        }

        // create default sort order if none are loaded
        if (ui->pres_sort->currentIndex() < 0) {
            createDefaultSortOrder(PRESENTATIONS);
            ui->pres_sort->addItem(allPresOrders[0][0]);
        }

        // create the tree
        presPath = path;
        makeTree(PRESENTATIONS);
        ui->pres_file_label->setText(presPath);

        return true;
    } else {
        if (!multi_file) {
            QMessageBox::critical(this, "Invalid File", "Not a valid presentations file.");
            on_pres_load_file_clicked();
        }
    }
    return false;
}

void MainWindow::on_fund_load_file_clicked() {
    QString path = load_file();
    if (!path.isEmpty()) {
        load_fund(path);
    }
}

bool MainWindow::load_fund(QString path, bool multi_file, bool skip_prompt) {
    if (!checkFile(FUNDING, path, skip_prompt)) {
        // enable gui elements
        ui->fund_sort->setEnabled(true);
        ui->fund_delete_sort->setEnabled(true);
        ui->fund_new_sort->setEnabled(true);
        ui->fund_edit_sort->setEnabled(true);
        ui->fund_filter_from->setEnabled(true);
        ui->fund_filter_to->setEnabled(true);
        ui->fund_pie_button->setEnabled(true);
        ui->fund_bar_button->setEnabled(true);
        ui->fund_line_button->setEnabled(true);
        ui->fund_to_label->setEnabled(true);
        ui->fund_sort_label->setEnabled(true);
        ui->fund_filter->setEnabled(true);
        ui->fund_filter_label->setEnabled(true);
        ui->fundCustomList->setEnabled(true);


        // load save order
        QSortListIO fundSaveOrder(FUNDORDER_SAVE);
        allFundOrders = fundSaveOrder.readList();
        ui->fund_sort->clear();
        for (int i = 0; i < allFundOrders.size(); i++) {
            ui->fund_sort->addItem(allFundOrders.at(i).at(0));
        }

        // create default sort order if none are loaded
        if (ui->fund_sort->currentIndex() < 0) {
            createDefaultSortOrder(FUNDING);
            ui->fund_sort->addItem(allFundOrders[0][0]);
        }

        // create the tree
        fundPath = path;
        makeTree(FUNDING);
        ui->fund_file_label->setText(fundPath);

        return true;
    } else {
        if (!multi_file) {
            QMessageBox::critical(this, "Invalid File", "Not a valid grants and funding file.");
            on_fund_load_file_clicked();
        }
    }
    return false;
}

void MainWindow::on_FromDate_dateChanged(const QDate &date) {
    // set the member variable to the new date
    yearStart = date.year();

    // update end date spinbox to not fall below that year
    ui->ToDate->setMinimumDate(date);

    dateChanged = {true, true, true, true};

    // refresh the GUI
    refresh(ui->categoryTab->currentIndex());
}

void MainWindow::on_ToDate_dateChanged(const QDate &date) {
    // set the member variable to the new date
    yearEnd = date.year();

    // update end date spinbox to not fall below that year
    ui->ToDate->setMinimumDate(ui->FromDate->date());

    dateChanged = {true, true, true, true};

    // refresh the GUI
    refresh(ui->categoryTab->currentIndex());
}

void MainWindow::on_categoryTab_currentChanged() {
    if (dateChanged[ui->categoryTab->currentIndex()] == true) {
        refresh(ui->categoryTab->currentIndex());
        dateChanged[ui->categoryTab->currentIndex()] = false;
    }
}

std::vector<std::pair <std::string, double>> MainWindow::fundTreeView_total_scatter(const QModelIndex &index)
{
    std::vector<std::pair <std::string, double>> chartList;
    QString clickedName = index.data(Qt::DisplayRole).toString();

    for (int i = 0; i < index.model()->rowCount()-1; i++)
    {
        std::vector<std::string> parentsList;
        QString name;
        QModelIndex current = index;
        current = current.sibling(i,0);
        //qDebug() << current.sibling(i,2).data(Qt::DisplayRole).toString().remove(0,1).remove(',').toDouble();
        chartList.emplace_back(current.sibling(i,1).data(Qt::DisplayRole).toString().toStdString(), current.sibling(i,2).data(Qt::DisplayRole).toString().remove(0,1).remove(',').toDouble());
    }
    return chartList;

}

std::vector<std::pair <std::string, double>> MainWindow::teachTreeView_total(const QModelIndex &index)
{
    std::vector<std::pair <std::string, double>> chartList;
    QString clickedName = index.data(Qt::DisplayRole).toString();

    for (int i = 0; i < index.model()->rowCount()-1; i++)
    {
        std::vector<std::string> parentsList;
        QString name;
        QModelIndex current = index;
        current = current.sibling(i,0);

        std::string entryName = current.data(Qt::DisplayRole).toString().toStdString();

        //qDebug() << current.sibling(i,1).data(Qt::DisplayRole).toString();
        //qDebug() << current.data(Qt::DisplayRole).toString();

        while (true) {
            name = current.data(Qt::DisplayRole).toString();
            if(name!="") {
                auto it = parentsList.begin();
                it = parentsList.insert(it, name.toStdString());
            } else {
                break;
            }
            current = current.parent();
        }
        if (parentsList.size() != teachSortOrder.size()) {
            teachClickedName = clickedName;
            std::vector<std::string> sortOrder(teachSortOrder.begin(), teachSortOrder.begin()+parentsList.size()+1);
            std::vector<std::pair <std::string, int>> list =
                    teachdb->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(TEACH), getFilterEndChar(TEACH));
            double tot = 0;
            for (int i = 0; i < (int) list.size(); i++) {
                tot += static_cast<double>(list[i].second);
            }
            chartList.emplace_back(entryName, tot);
        } else {
            ui->teach_graph_stackedWidget->hide();
            ui->teachGraphTitle->clear();
            teachClickedName.clear();
        }
    }
    return chartList;
}

void MainWindow::on_teachTreeView_clicked(const QModelIndex &index) {
    ui->teach_line_button->setEnabled(false);
    QString clickedName = index.data(Qt::DisplayRole).toString();
    if (clickedName==teachClickedName || index.column()!=0) { return;}
    std::vector<std::pair <std::string, double>> chartListTotal;
    bool total = false;
    if (clickedName == "Total")
    {
        total = true;
        chartListTotal = teachTreeView_total(index);
    }
    std::vector<std::string> parentsList;
    QModelIndex current = index;
    QString name;
    while (true) {
        name = current.data(Qt::DisplayRole).toString();
        if(name!="") {
            auto it = parentsList.begin();
            it = parentsList.insert(it, name.toStdString());
        } else {
            break;
        }
        current = current.parent();
    }

    if (parentsList.size()!=teachSortOrder.size()) {
        teachClickedName = clickedName;
        std::vector<std::string> sortOrder(teachSortOrder.begin(), teachSortOrder.begin()+parentsList.size()+1);
        std::vector<std::pair <std::string, int>> list;
        if(teachFlag == true){
            std::vector<std::pair <std::string, int>> list =
                    teachdb2->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(TEACH), getFilterEndChar(TEACH));
            std::vector<std::pair <std::string, double>> chartList;

            for (int i = 0; i < (int) list.size(); i++) {
                chartList.emplace_back(list[i].first, static_cast<double>(list[i].second));

            }
            if(total) chartList = chartListTotal;
            if (!chartList.empty()) {
                ui->teachBarChart->clearPlottables();
                setupBarChart(ui->teachBarChart, chartList);
                ui->teachBarChart->replot();

                setupPieChart(ui->teachPieChart, ui->teachPieList, chartList);

                ui->teachLineChart->clearPlottables();
                if((sortOrder[sortOrder.size()-1]).compare("Start Date") == 0 && !total)
                {
                    ui->teach_line_button->setEnabled(true);
                    setupLineChart(ui->teachLineChart,chartList);
                }
                ui->teachLineChart->yAxis->setLabel("Number of courses taught");
                ui->teachLineChart->replot();


                // setupBarChart(ui->teachLineChart,chartList);

                if (parentsList.size()>1) {
                    ui->teachGraphTitle->setText("Total " + clickedName + " Teaching by " +
                                                 QString::fromStdString(teachSortOrder[parentsList.size()]) + " for " + QString::fromStdString(parentsList[0]));
                } else {
                    ui->teachGraphTitle->setText("Total Teaching by " + QString::fromStdString(parentsList[0]));
                }
                ui->teach_graph_stackedWidget->show();
            }
        }
        else{
            std::vector<std::pair <std::string, int>> list =
                    teachdb->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(TEACH), getFilterEndChar(TEACH));

            std::vector<std::pair <std::string, double>> chartList;

            for (int i = 0; i < (int) list.size(); i++) {
                chartList.emplace_back(list[i].first, static_cast<double>(list[i].second));

            }
            if(total) chartList = chartListTotal;
            if (!chartList.empty()) {
                ui->teachBarChart->clearPlottables();
                setupBarChart(ui->teachBarChart, chartList);
                ui->teachBarChart->replot();

                setupPieChart(ui->teachPieChart, ui->teachPieList, chartList);

                ui->teachLineChart->clearPlottables();
                if((sortOrder[sortOrder.size()-1]).compare("Start Date") == 0 && !total)
                {
                    ui->teach_line_button->setEnabled(true);
                    setupLineChart(ui->teachLineChart,chartList);
                }
                ui->teachLineChart->yAxis->setLabel("Number of courses taught");
                ui->teachLineChart->replot();


                // setupBarChart(ui->teachLineChart,chartList);

                if (parentsList.size()>1) {
                    ui->teachGraphTitle->setText("Total " + clickedName + " Teaching by " +
                                                 QString::fromStdString(teachSortOrder[parentsList.size()]) + " for " + QString::fromStdString(parentsList[0]));
                } else {
                    ui->teachGraphTitle->setText("Total Teaching by " + QString::fromStdString(parentsList[0]));
                }
                ui->teach_graph_stackedWidget->show();
            }
        }

    } else {
        ui->teach_graph_stackedWidget->hide();
        ui->teachGraphTitle->clear();
        teachClickedName.clear();
    }
}

std::vector<std::pair <std::string, double>> MainWindow::pubTreeView_total(const QModelIndex &index)
{
    std::vector<std::pair <std::string, double>> chartList;
    QString clickedName = index.data(Qt::DisplayRole).toString();

    for (int i = 0; i < index.model()->rowCount()-1; i++)
    {
        std::vector<std::string> parentsList;
        QString name;
        QModelIndex current = index;
        current = current.sibling(i,0);

        chartList.emplace_back(current.sibling(i,1).data(Qt::DisplayRole).toString().toStdString(), current.sibling(i,2).data(Qt::DisplayRole).toDouble());
    }
    return chartList;
}

void MainWindow::on_pubTreeView_clicked(const QModelIndex &index) {
    ui->pub_line_button->setEnabled(false);
    QString clickedName = index.data(Qt::DisplayRole).toString();
    if (clickedName==pubClickedName || index.column()!=0) { return;}
    std::vector<std::pair <std::string, double>> chartListTotal;
    bool total = false;
    if (clickedName == "Total")
    {
        total = true;
        chartListTotal = pubTreeView_total(index);
    }

    std::vector<std::string> parentsList;
    QModelIndex current = index;
    QString name;
    while (true) {
        name = current.data(Qt::DisplayRole).toString();
        if(name!="") {
            auto it = parentsList.begin();
            it = parentsList.insert(it, name.toStdString());
        } else {
            break;
        }
        current = current.parent();
    }

    if (parentsList.size()!=pubSortOrder.size()) {
        pubClickedName = clickedName;
        std::vector<std::string> sortOrder(pubSortOrder.begin(), pubSortOrder.begin()+parentsList.size()+1);
        if(pubFlag == true){
            std::vector<std::pair <std::string, int>> list =
                    pubdb2->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(TEACH), getFilterEndChar(TEACH));
            std::vector<std::pair <std::string, double>> chartList;

            for (int i = 0; i < (int) list.size(); i++) {
                chartList.emplace_back(list[i].first, static_cast<double>(list[i].second));
            }
            if(total) chartList = chartListTotal;
            if (!chartList.empty()) {
                ui->pubBarChart->clearPlottables();
                setupBarChart(ui->pubBarChart, chartList);
                ui->pubBarChart->replot();

                setupPieChart(ui->pubPieChart, ui->pubPieList, chartList);

                ui->pubLineChart->clearPlottables();
                if((sortOrder[sortOrder.size()-1]).compare("Start Date") == 0  && !total)
                {
                    ui->pub_line_button->setEnabled(true);
                    setupLineChart(ui->pubLineChart,chartList);
                }
                ui->pubLineChart->yAxis->setLabel("Number of publications");
                ui->pubLineChart->replot();

                if (parentsList.size()>1) {
                    ui->pubGraphTitle->setText("Total " + clickedName + " Publications by " +
                                               QString::fromStdString(pubSortOrder[parentsList.size()]) + " for " + QString::fromStdString(parentsList[0]));
                } else {
                    ui->pubGraphTitle->setText("Total Publications by " + QString::fromStdString(parentsList[0]));
                }
                ui->pub_graph_stackedWidget->show();
            }

        }
        else{
            std::vector<std::pair <std::string, int>> list =
                    pubdb->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(PUBLICATIONS), getFilterEndChar(PUBLICATIONS));
            std::vector<std::pair <std::string, double>> chartList;
            for (int i = 0; i < (int) list.size(); i++) {
                chartList.emplace_back(list[i].first, static_cast<double>(list[i].second));
            }
            if(total) chartList = chartListTotal;
            if (!chartList.empty()) {
                ui->pubBarChart->clearPlottables();
                setupBarChart(ui->pubBarChart, chartList);
                ui->pubBarChart->replot();

                setupPieChart(ui->pubPieChart, ui->pubPieList, chartList);

                ui->pubLineChart->clearPlottables();
                if((sortOrder[sortOrder.size()-1]).compare("Start Date") == 0  && !total)
                {
                    ui->pub_line_button->setEnabled(true);
                    setupLineChart(ui->pubLineChart,chartList);
                }
                ui->pubLineChart->yAxis->setLabel("Number of publications");
                ui->pubLineChart->replot();

                if (parentsList.size()>1) {
                    ui->pubGraphTitle->setText("Total " + clickedName + " Publications by " +
                                               QString::fromStdString(pubSortOrder[parentsList.size()]) + " for " + QString::fromStdString(parentsList[0]));
                } else {
                    ui->pubGraphTitle->setText("Total Publications by " + QString::fromStdString(parentsList[0]));
                }
                ui->pub_graph_stackedWidget->show();
            }
        }
    } else {
        ui->pub_graph_stackedWidget->hide();
        ui->pubGraphTitle->clear();
        pubClickedName.clear();
    }
}

std::vector<std::pair <std::string, double>> MainWindow::presTreeView_total(const QModelIndex &index)
{
    std::vector<std::pair <std::string, double>> chartList;
    QString clickedName = index.data(Qt::DisplayRole).toString();

    for (int i = 0; i < index.model()->rowCount()-1; i++)
    {
        std::vector<std::string> parentsList;
        QString name;
        QModelIndex current = index;
        current = current.sibling(i,0);
        std::string entryName = current.data(Qt::DisplayRole).toString().toStdString();
        while (true) {
            name = current.data(Qt::DisplayRole).toString();
            if(name!="") {
                auto it = parentsList.begin();
                it = parentsList.insert(it, name.toStdString());
            } else {
                break;
            }
            current = current.parent();
        }
        if (parentsList.size() != presSortOrder.size()) {
            presClickedName = clickedName;
            std::vector<std::string> sortOrder(presSortOrder.begin(), presSortOrder.begin()+parentsList.size()+1);
            std::vector<std::pair <std::string, int>> list =
                    presdb->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(TEACH), getFilterEndChar(TEACH));
            double tot = 0;
            for (int i = 0; i < (int) list.size(); i++) {
                tot += static_cast<double>(list[i].second);
            }
            chartList.emplace_back(entryName, tot);
        } else {
            ui->pres_graph_stackedWidget->hide();
            ui->presGraphTitle->clear();
            presClickedName.clear();
        }
    }
    return chartList;
}

void MainWindow::on_presTreeView_clicked(const QModelIndex &index) {
    ui->pres_line_button->setEnabled(false);
    QString clickedName = index.data(Qt::DisplayRole).toString();
    if (clickedName==presClickedName || index.column()!=0) { return;}
    std::vector<std::pair <std::string, double>> chartListTotal;
    bool total = false;
    if (clickedName == "Total")
    {
        total = true;
        chartListTotal = presTreeView_total(index);
    }

    std::vector<std::string> parentsList;
    QModelIndex current = index;
    QString name;
    while (true) {
        name = current.data(Qt::DisplayRole).toString();
        if(name!="") {
            auto it = parentsList.begin();
            it = parentsList.insert(it, name.toStdString());
        } else {
            break;
        }
        current = current.parent();
    }

    if (parentsList.size()!=presSortOrder.size()) {
        presClickedName = clickedName;
        std::vector<std::string> sortOrder(presSortOrder.begin(), presSortOrder.begin()+parentsList.size()+1);
        if(presFlag == true){
            std::vector<std::pair <std::string, int>> list =
                    presdb2->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(PRESENTATIONS), getFilterEndChar(PRESENTATIONS));
            std::vector<std::pair <std::string, double>> chartList;
            for (int i = 0; i < (int) list.size(); i++) {
                chartList.emplace_back(list[i].first, static_cast<double>(list[i].second));
            }
            if(total) chartList = chartListTotal;
            if (!chartList.empty()) {
                ui->presBarChart->clearPlottables();
                setupBarChart(ui->presBarChart, chartList);
                ui->presBarChart->replot();

                setupPieChart(ui->presPieChart, ui->presPieList, chartList);

                ui->presLineChart->clearPlottables();
                if((sortOrder[sortOrder.size()-1]).compare("Start Date") == 0  && !total)
                {
                    ui->pres_line_button->setEnabled(true);
                    setupLineChart(ui->presLineChart,chartList);
                }
                ui->presLineChart->yAxis->setLabel("Number of presentations");
                ui->presLineChart->replot();



                if (parentsList.size()>1) {
                    ui->presGraphTitle->setText("Total " + clickedName + " Presentations by " +
                                                QString::fromStdString(presSortOrder[parentsList.size()]) + " for " + QString::fromStdString(parentsList[0]));
                } else {
                    ui->presGraphTitle->setText("Total Presentations by " + QString::fromStdString(parentsList[0]));
                }
                ui->pres_graph_stackedWidget->show();
            }
        }
        else{
            std::vector<std::pair <std::string, int>> list =
                    presdb->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(PRESENTATIONS), getFilterEndChar(PRESENTATIONS));
            std::vector<std::pair <std::string, double>> chartList;
            for (int i = 0; i < (int) list.size(); i++) {
                chartList.emplace_back(list[i].first, static_cast<double>(list[i].second));
            }
            if(total) chartList = chartListTotal;
            if (!chartList.empty()) {
                ui->presBarChart->clearPlottables();
                setupBarChart(ui->presBarChart, chartList);
                ui->presBarChart->replot();

                setupPieChart(ui->presPieChart, ui->presPieList, chartList);

                ui->presLineChart->clearPlottables();
                if((sortOrder[sortOrder.size()-1]).compare("Start Date") == 0  && !total)
                {
                    ui->pres_line_button->setEnabled(true);
                    setupLineChart(ui->presLineChart,chartList);
                }
                ui->presLineChart->yAxis->setLabel("Number of presentations");
                ui->presLineChart->replot();



                if (parentsList.size()>1) {
                    ui->presGraphTitle->setText("Total " + clickedName + " Presentations by " +
                                                QString::fromStdString(presSortOrder[parentsList.size()]) + " for " + QString::fromStdString(parentsList[0]));
                } else {
                    ui->presGraphTitle->setText("Total Presentations by " + QString::fromStdString(parentsList[0]));
                }
                ui->pres_graph_stackedWidget->show();
            }
        }
    } else {
        ui->pres_graph_stackedWidget->hide();
        ui->presGraphTitle->clear();
        presClickedName.clear();
    }
}

std::vector<std::pair <std::string, double>> MainWindow::fundTreeView_total(const QModelIndex &index)
{
    std::vector<std::pair <std::string, double>> chartList;
    QString clickedName = index.data(Qt::DisplayRole).toString();

    for (int i = 0; i < index.model()->rowCount()-1; i++)
    {
        std::vector<std::string> parentsList;
        QString name;
        QModelIndex current = index;
        current = current.sibling(i,0);
        std::string entryName = current.data(Qt::DisplayRole).toString().toStdString();
        while (true) {
            name = current.data(Qt::DisplayRole).toString();
            if(name!="") {
                auto it = parentsList.begin();
                it = parentsList.insert(it, name.toStdString());
            } else {
                break;
            }
            current = current.parent();
        }
        if (parentsList.size() != fundSortOrder.size()) {
            fundClickedName = clickedName;
            std::vector<std::string> sortOrder(fundSortOrder.begin(), fundSortOrder.begin()+parentsList.size()+1);
            std::vector<std::pair <std::string, int>> list =
                    funddb->getCountTuple(yearStart, yearEnd, sortOrder, parentsList, getFilterStartChar(TEACH), getFilterEndChar(TEACH));
            double tot = 0;
            for (int i = 0; i < (int) list.size(); i++) {
                tot += static_cast<double>(list[i].second);
            }
            chartList.emplace_back(entryName, tot);
        } else {
            ui->fund_graph_stackedWidget->hide();
            ui->fundGraphTitle->clear();
            fundClickedName.clear();
        }
    }
    return chartList;

}

void MainWindow::on_fundTreeView_clicked(const QModelIndex &index) {
    ui->fund_line_button->setEnabled(false);
    ui->fund_histogram_button->setEnabled(false);
    QString clickedName = index.data(Qt::DisplayRole).toString();
    if (clickedName==fundClickedName || index.column()!=0) { return;}
    std::vector<std::pair <std::string, double>> chartListTotal;
    std::vector<std::pair <std::string, double>> chartListScatter;
    bool total = false;
    if (clickedName == "Total")
    {
        total = true;
        chartListTotal = fundTreeView_total(index);
        chartListScatter = fundTreeView_total_scatter(index);
    }

    std::vector<std::string> parentsList;
    QModelIndex current = index;
    QString name;
    while (true) {
        name = current.data(Qt::DisplayRole).toString();
        if(name!="") {
            auto it = parentsList.begin();
            it = parentsList.insert(it, name.toStdString());
        } else {
            break;
        }
        current = current.parent();
    }

    if (parentsList.size()!=fundSortOrder.size()) {
        if (clickedName != fundClickedName  || clickedName == "Total") {
            fundClickedName = clickedName;
            std::vector<std::string> sortOrder(fundSortOrder.begin(), fundSortOrder.begin()+parentsList.size()+1);
            if(fundFlag == true){
                std::vector<std::pair <std::string, double>> chartList =
                        funddb2->getTotalsTuple(yearStart, yearEnd, sortOrder, parentsList, "Total Amount", getFilterStartChar(FUNDING), getFilterEndChar(FUNDING));
                if(total) chartList = chartListTotal;
                if (!chartList.empty()) {
                    ui->fundBarChart->clearPlottables();
                    setupBarChart(ui->fundBarChart, chartList);
                    ui->fundBarChart->replot();

                    setupPieChart(ui->fundPieChart, ui->fundPieList, chartList);

                    ui->fundLineChart->clearPlottables();
                    if((sortOrder[sortOrder.size()-1]).compare("Start Date") == 0  && !total)
                    {
                        ui->fund_line_button->setEnabled(true);
                        setupLineChart(ui->fundLineChart,chartList);
                    }
                    ui->fundLineChart->yAxis->setLabel("Amount of funding in CAD");
                    ui->fundLineChart->replot();

                    if(total)
                    {
                        ui->fund_histogram_button->setEnabled(true);
                        ui->fundHistogramChart->clearPlottables();
                        setupScatterPlot(ui->fundHistogramChart,chartListScatter);
                        ui->fundHistogramChart->replot();
                    }

                    if (parentsList.size()>1) {
                        ui->fundGraphTitle->setText("Total " + clickedName + " Grants & Funding by " +
                                                    QString::fromStdString(fundSortOrder[parentsList.size()]) + " for " + QString::fromStdString(parentsList[0]));
                    } else {
                        ui->fundGraphTitle->setText("Total Grants & Funding by " + QString::fromStdString(parentsList[0]));
                    }
                    ui->fund_graph_stackedWidget->show();
                }
            }
            else{
                std::vector<std::pair <std::string, double>> chartList =
                        funddb->getTotalsTuple(yearStart, yearEnd, sortOrder, parentsList, "Total Amount", getFilterStartChar(FUNDING), getFilterEndChar(FUNDING));
                if(total) chartList = chartListTotal;
                if (!chartList.empty()) {
                    ui->fundBarChart->clearPlottables();
                    setupBarChart(ui->fundBarChart, chartList);
                    ui->fundBarChart->replot();

                    setupPieChart(ui->fundPieChart, ui->fundPieList, chartList);

                    ui->fundLineChart->clearPlottables();
                    if((sortOrder[sortOrder.size()-1]).compare("Start Date") == 0  && !total)
                    {
                        ui->fund_line_button->setEnabled(true);
                        setupLineChart(ui->fundLineChart,chartList);
                    }
                    ui->fundLineChart->yAxis->setLabel("Amount of funding in CAD");
                    ui->fundLineChart->replot();

                    if(total)
                    {
                        ui->fund_histogram_button->setEnabled(true);
                        ui->fundHistogramChart->clearPlottables();
                        setupScatterPlot(ui->fundHistogramChart,chartListScatter);
                        ui->fundHistogramChart->replot();
                    }

                    if (parentsList.size()>1) {
                        ui->fundGraphTitle->setText("Total " + clickedName + " Grants & Funding by " +
                                                    QString::fromStdString(fundSortOrder[parentsList.size()]) + " for " + QString::fromStdString(parentsList[0]));
                    } else {
                        ui->fundGraphTitle->setText("Total Grants & Funding by " + QString::fromStdString(parentsList[0]));
                    }
                    ui->fund_graph_stackedWidget->show();
                }
            }
        } else {
            ui->fund_graph_stackedWidget->hide();
            ui->fundGraphTitle->clear();
            fundClickedName.clear();
        }
    }
}

void MainWindow::on_teachPrintButton_clicked()
{
    QPrintDialog printDialog(printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        QCPPainter painter;
        painter.begin(printer);
        ui->teachChartFrame->render(&painter);
    }
}

void MainWindow::on_fundPrintButton_clicked()
{
    QPrintDialog printDialog(printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        QCPPainter painter;
        painter.begin(printer);
        ui->fundChartFrame->render(&painter);
    }
}

void MainWindow::on_presPrintButton_clicked()
{
    QPrintDialog printDialog(printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        QCPPainter painter;
        painter.begin(printer);
        ui->presChartFrame->render(&painter);
    }
}

void MainWindow::on_pubPrintButton_clicked()
{
    QPrintDialog printDialog(printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        QCPPainter painter;
        painter.begin(printer);
        ui->pubChartFrame->render(&painter);
    }
}

void MainWindow::on_teachExportButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export File"),
                                                    QDir::homePath(),
                                                    tr("PDF (*.pdf)"));
    if (fileName.contains("") != 0) {
        QPdfWriter writer(fileName);
        writer.setPageOrientation(QPageLayout::Landscape);
        QPainter painter;
        painter.begin(&writer);
        painter.scale(10.0, 10.0);
        ui->teachChartFrame->render(&painter);
        /* Another option for bar chart since it is QCustom plot
        if (ui->teach_bar_button->isChecked()) {
            ui->teachBarChart->savePdf(fileName);
        } */
    }

}

void MainWindow::on_fundExportButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export File"),
                                                    QDir::homePath(),
                                                    tr("PDF (*.pdf)"));
    if (fileName.contains("") != 0) {
        QPdfWriter writer(fileName);
        writer.setPageOrientation(QPageLayout::Landscape);
        QPainter painter;
        painter.begin(&writer);
        painter.scale(10.0, 10.0);
        ui->fundChartFrame->render(&painter);
    }
}

void MainWindow::on_presExportButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export File"),
                                                    QDir::homePath(),
                                                    tr("PDF (*.pdf)"));
    if (fileName.contains("") != 0) {
        QPdfWriter writer(fileName);
        writer.setPageOrientation(QPageLayout::Landscape);
        QPainter painter;
        painter.begin(&writer);
        painter.scale(10.0, 10.0);
        ui->presChartFrame->render(&painter);
    }
}

void MainWindow::on_pubExportButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export File"),
                                                    QDir::homePath(),
                                                    tr("PDF (*.pdf)"));
    if (fileName.contains("") != 0) {
        QPdfWriter writer(fileName);
        writer.setPageOrientation(QPageLayout::Landscape);
        QPainter painter;
        painter.begin(&writer);
        painter.scale(10.0, 10.0);
        ui->pubChartFrame->render(&painter);
    }
}


char MainWindow::getFilterStartChar(int type) {
    char charInField;

    // get char from the text field
    switch (type) {
    case FUNDING:
        charInField = ui->fund_filter_from->text().toStdString()[0];
        break;
    case PRESENTATIONS:
        charInField = ui->pres_filter_from->text().toStdString()[0];
        break;
    case PUBLICATIONS:
        charInField = ui->pub_filter_from->text().toStdString()[0];
        break;
    case TEACH:
        charInField = ui->teach_filter_from->text().toStdString()[0];
        break;
    case TEACH_CUSTOM:
        charInField = ui->teach_filter_from->text().toStdString()[0];
        break;
    }

    // convert the char to uppercase
    char uppercase = charInField & ~0x20;

    // if alphabetical, return that letter
    if ('A' <= uppercase && uppercase <= 'Z') {
        return uppercase;
    } else {
        // otherwise, default is '*'
        return '*';
    }
}

char MainWindow::getFilterEndChar(int type) {
    char charInField;

    // get char from the text field
    switch (type) {
    case FUNDING:
        charInField = ui->fund_filter_to->text().toStdString()[0];
        break;
    case PRESENTATIONS:
        charInField = ui->pres_filter_to->text().toStdString()[0];
        break;
    case PUBLICATIONS:
        charInField = ui->pub_filter_to->text().toStdString()[0];
        break;
    case TEACH:
        charInField = ui->teach_filter_to->text().toStdString()[0];
        break;
    case TEACH_CUSTOM:
        charInField = ui->teach_filter_to->text().toStdString()[0];
        break;
    case FUNDING_CUSTOM:
        charInField = ui->fund_filter_to->text().toStdString()[0];
        break;
    case PRESENTATIONS_CUSTOM:
        charInField = ui->pres_filter_to->text().toStdString()[0];
        break;
    case PUBLICATIONS_CUSTOM:
        charInField = ui->pub_filter_to->text().toStdString()[0];
        break;
    }

    // convert the char to uppercase
    char uppercase = charInField & ~0x20;

    // if alphabetical, return that letter
    if ('A' <= uppercase && uppercase <= 'Z') {
        return uppercase;
    } else if (charInField == '*') {
        // otherwise, is it '*'?
        return charInField;
    } else {
        // otherwise, default is 'Z'
        return 'Z';
    }
}

void MainWindow::on_teach_filter_from_textChanged() {
    if(teachFlag == true)
        refresh(TEACH_CUSTOM);
    else
        refresh(TEACH);
}
void MainWindow::on_teach_filter_to_textChanged() {
    if(teachFlag == true)
        refresh(TEACH_CUSTOM);
    else
        refresh(TEACH);
}
void MainWindow::on_pub_filter_from_textChanged() {
    if(pubFlag == true)
        refresh(PUBLICATIONS_CUSTOM);
    else
        refresh(PUBLICATIONS);
}
void MainWindow::on_pub_filter_to_textChanged() {
    if(pubFlag == true)
        refresh(PUBLICATIONS_CUSTOM);
    else
        refresh(PUBLICATIONS);
}
void MainWindow::on_pres_filter_from_textChanged() {
    if(presFlag == true)
        refresh(PRESENTATIONS_CUSTOM);
    else
        refresh(PRESENTATIONS);
}
void MainWindow::on_pres_filter_to_textChanged() {
    if(presFlag == true)
        refresh(PRESENTATIONS_CUSTOM);
    else
        refresh(PRESENTATIONS);
}
void MainWindow::on_fund_filter_from_textChanged() {
    if(fundFlag == true)
        refresh(FUNDING_CUSTOM);
    else
        refresh(FUNDING);
}
void MainWindow::on_fund_filter_to_textChanged() {
    if(fundFlag == true)
        refresh(FUNDING_CUSTOM);
    else
        refresh(FUNDING);
}



void MainWindow::on_teachCustomList_clicked()
{
    selectData* sortdialog = new selectData();
    sortdialog->setWindowTitle("New Custom List");
    sortdialog->setFields(teachNames);
    sortdialog->setCustomFields(teachNamesCustom);
    std::vector<string> tempCustomList;
    int ret = sortdialog->exec();
    if (ret){
        teachNamesCustom = sortdialog->getCustomSortFields();
        teachNames = sortdialog->getSortFields();

        if(teachNamesCustom.size() == 0){
            tempCustomList = teachNames;
        }
        else{
            tempCustomList = teachNamesCustom;
        }
        teachFlag = true;
        // create a new reader to read in the file
        CSVReader reader;
        std::vector<std::string> header;
        int sortHeaderIndex = 2;
        reader = CSVReader(teachPath.toStdString());
        header = reader.getHeaders();

        // create a new manager for the data
        delete teachdb2;
        teachdb2 = new RecordsManager(&header);
        // check for right file type by searching for unique header
        sortHeaderIndex = teachdb2->getHeaderIndex("Start Date");
        int headerIndex = teachdb->getHeaderIndex("Member Name");
        teachData = reader.getData();
        unsigned int j;
        for (int i = 0; i < (int) teachData.size(); i++)
        {
            for (j = 0; j < TEACH_MANFIELDS.size(); j++)
            {
                int index = teachdb2->getHeaderIndex(TEACH_MANFIELDS[j]);
                if (teachData[i][index].compare("") == 0)
                {
                    break;
                }
            }

            // if all mandatory fields are okay
            if (j == TEACH_MANFIELDS.size())
            {
                // date interpretation
                int yrIndex = teachdb2->getHeaderIndex("Start Date");
                int year;
                sscanf(teachData[i][yrIndex].c_str(), "%4d", &year);
                teachData[i][yrIndex] = std::to_string(year);
                if(std::find(tempCustomList.begin(), tempCustomList.end(), teachData[i][headerIndex])!=tempCustomList.end())
                    teachdb2->addRecord(reader.parseDateString(teachData[i][sortHeaderIndex]), &teachData[i]);
            }
        }
        ui->teach_graph_stackedWidget->hide();
        ui->teachGraphTitle->clear();
        teachClickedName.clear();
        makeTree(TEACH_CUSTOM);

    }
    delete sortdialog;
}

void MainWindow::on_pubCustomList_clicked()
{
    selectData* sortdialog = new selectData();
    sortdialog->setWindowTitle("New Custom List");
    sortdialog->setFields(pubNames);
    sortdialog->setCustomFields(pubNamesCustom);
    std::vector<string> tempCustomList;
    int ret = sortdialog->exec();
    if (ret){
        pubNamesCustom = sortdialog->getCustomSortFields();
        pubNames = sortdialog->getSortFields();

        if(pubNamesCustom.size() == 0){
            tempCustomList = pubNames;
        }
        else{
            tempCustomList = pubNamesCustom;
        }
        pubFlag = true;
        // create a new reader to read in the file
        CSVReader reader;
        std::vector<std::string> header;
        int sortHeaderIndex = 2;
        reader = CSVReader(pubPath.toStdString());
        header = reader.getHeaders();

        // create a new manager for the data
        delete pubdb2;
        pubdb2 = new RecordsManager(&header);
        // check for right file type by searching for unique header
        sortHeaderIndex = pubdb2->getHeaderIndex("Status Date");
        pubData = reader.getData();
        unsigned int j;
        for (int i = 0; i < (int) pubData.size(); i++)
        {
            for (j = 0; j < PUBS_MANFIELDS.size(); j++)
            {
                int index = pubdb2->getHeaderIndex(PUBS_MANFIELDS[j]);
                if (pubData[i][index].compare("") == 0)
                {
                    break;
                }
            }

            // if all mandatory fields are okay
            if (j == PUBS_MANFIELDS.size())
            {
                // date interpretation
                int yrIndex = pubdb2->getHeaderIndex("Status Date");
                int headerIndex = pubdb2->getHeaderIndex("Member Name");
                int year;
                sscanf(pubData[i][yrIndex].c_str(), "%4d", &year);
                pubData[i][yrIndex] = std::to_string(year);
                if(std::find(tempCustomList.begin(), tempCustomList.end(), pubData[i][headerIndex])!=tempCustomList.end())
                    pubdb2->addRecord(reader.parseDateString(pubData[i][sortHeaderIndex]), &pubData[i]);
            }
        }
        ui->pub_graph_stackedWidget->hide();
        ui->pubGraphTitle->clear();
        pubClickedName.clear();
        makeTree(PUBLICATIONS_CUSTOM);

    }
    delete sortdialog;
}
void MainWindow::on_presCustomList_clicked(){
    selectData* sortdialog = new selectData();
    sortdialog->setWindowTitle("New Custom List");
    sortdialog->setFields(presNames);
    sortdialog->setCustomFields(presNamesCustom);
    std::vector<string> tempCustomList;
    int ret = sortdialog->exec();
    if (ret){
        presNamesCustom = sortdialog->getCustomSortFields();
        presNames = sortdialog->getSortFields();

        if(presNamesCustom.size() == 0){
            tempCustomList = presNames;
        }
        else{
            tempCustomList = presNamesCustom;
        }
        presFlag = true;
        // create a new reader to read in the file
        CSVReader reader;
        std::vector<std::string> header;
        int sortHeaderIndex = 2;
        reader = CSVReader(presPath.toStdString());
        header = reader.getHeaders();

        // create a new manager for the data
        delete presdb2;
        presdb2 = new RecordsManager(&header);

        // load in data into the manager, with the date as the key
        sortHeaderIndex = presdb2->getHeaderIndex("Date");
        presData = reader.getData();
        unsigned int j = 0;
        for (int i = 0; i < (int) presData.size(); i++) {
            //check through mandatory fields for empty fields
            for (j = 0; j < PRES_MANFIELDS.size(); j++) {
                int index = presdb2->getHeaderIndex(PRES_MANFIELDS[j]);
                if (presData[i][index].compare("") == 0) {
                    break;
                }
            }

            // if all mandatory fields are okay
            if (j == PRES_MANFIELDS.size()) {
                // date interpretation
                int yrIndex = presdb2->getHeaderIndex("Date");
                int headerIndex = presdb2->getHeaderIndex("Member Name");
                int year;
                sscanf(presData[i][yrIndex].c_str(), "%4d", &year);
                presData[i][yrIndex] = std::to_string(year);
                if(std::find(tempCustomList.begin(), tempCustomList.end(), presData[i][headerIndex])!=tempCustomList.end())
                    presdb2->addRecord(reader.parseDateString(presData[i][sortHeaderIndex]), &presData[i]);
            }
        }
        ui->pres_graph_stackedWidget->hide();
        ui->presGraphTitle->clear();
        presClickedName.clear();
        makeTree(PRESENTATIONS_CUSTOM);
    }
    delete sortdialog;
}

void MainWindow::on_fundCustomList_clicked(){
    selectData* sortdialog = new selectData();
    sortdialog->setWindowTitle("New Custom List");
    sortdialog->setFields(fundNames);
    sortdialog->setCustomFields(fundNamesCustom);
    std::vector<string> tempCustomList;
    int ret = sortdialog->exec();
    if (ret){
        fundNamesCustom = sortdialog->getCustomSortFields();
        fundNames = sortdialog->getSortFields();

        if(fundNamesCustom.size() == 0){
            tempCustomList = fundNames;
        }
        else{
            tempCustomList = fundNamesCustom;
        }
        fundFlag = true;
        // create a new reader to read in the file
        CSVReader reader;
        std::vector<std::string> header;
        int sortHeaderIndex = 2;
        reader = CSVReader(fundPath.toStdString());
        header = reader.getHeaders();

        // create a new manager for the data
        delete funddb2;
        funddb2 = new RecordsManager(&header);

        // load in data into the manager, with the date as the key
        sortHeaderIndex = funddb2->getHeaderIndex("Start Date");
        fundData = reader.getData();
        unsigned int j;
        for (int i = 0; i < (int) fundData.size(); i++) {
            for (j = 0; j < GRANTS_MANFIELDS.size(); j++) {
                int index = funddb2->getHeaderIndex(GRANTS_MANFIELDS[j]);
                if (fundData[i][index].compare("") == 0) {
                    break;
                }
            }

            // if all mandatory fields are okay
            if (j == GRANTS_MANFIELDS.size()) {
                // date interpretation
                int yrIndex = funddb2->getHeaderIndex("Start Date");
                int headerIndex = funddb2->getHeaderIndex("Member Name");
                int year;
                sscanf(fundData[i][yrIndex].c_str(), "%4d", &year);
                fundData[i][yrIndex] = std::to_string(year);

                // boolean interpretation
                int prIndex = funddb2->getHeaderIndex("Peer Reviewed?");
                if (fundData[i][prIndex].compare("True") == 0) {
                    fundData[i][prIndex] = "Peer Reviewed";
                } else {
                    fundData[i][prIndex] = "Not Peer Reviewed";
                }
                if(std::find(tempCustomList.begin(), tempCustomList.end(), fundData[i][headerIndex])!=tempCustomList.end())
                    funddb2->addRecord(reader.parseDateString(fundData[i][sortHeaderIndex]), &fundData[i]);
            }
        }
        ui->fund_graph_stackedWidget->hide();
        ui->fundGraphTitle->clear();
        fundClickedName.clear();
        makeTree(FUNDING_CUSTOM);
    }
    delete sortdialog;
}
