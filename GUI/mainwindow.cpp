#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "processmodel.h"

#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Process Viewer");
    resize(900, 600);

    auto *model = new ProcessModel(this);
    auto *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);

    auto *view = new QTableView(this);
    view->setModel(proxy);
    view->setSortingEnabled(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->horizontalHeader()->setStretchLastSection(true);
    setCentralWidget(view);            // replaces the empty .ui central widget

    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, model, &ProcessModel::refresh);
    timer->start(2000);                // refresh every 2 seconds
    model->refresh();                  // initial fill
}

MainWindow::~MainWindow()
{
    delete ui;
}