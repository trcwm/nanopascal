#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QSettings>
#include <QStatusBar>
#include <QFileDialog>
#include <QBoxLayout>
#include <QHeaderView>
#include <QCoreApplication>

#include "common.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle("VM Debugger");
    setMinimumSize(800,450);

	QMenuBar * menuBar = new QMenuBar(this);
	setMenuBar(menuBar);    

    QMenu * menu = menuBar->addMenu("&File");

	// New
	QAction * action = new QAction("&New", this);
	connect(action, &QAction::triggered, this, &MainWindow::OnFileNew);
	menu->addAction(action);

	// Open
	action = new QAction("&Open", this);
	connect(action, &QAction::triggered, this, &MainWindow::OnFileOpen);
	menu->addAction(action);

  	menu->addSeparator();

    // Quit
    action = new QAction("E&xit", this);
    connect(action, &QAction::triggered, qApp, &QCoreApplication::quit, Qt::QueuedConnection);
    menu->addAction(action);

    QStatusBar * statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // build the GUI
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout();

    QVBoxLayout *cpuinfoLayout = new QVBoxLayout();
    m_regTable = new QTableView(centralWidget);
    m_regTable->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_regTable->horizontalHeader()->setStretchLastSection(true);
    cpuinfoLayout->addWidget(m_regTable,0);

    m_memTable = new QTableView(centralWidget);
    cpuinfoLayout->addWidget(m_memTable,2);

    mainLayout->addLayout(cpuinfoLayout);

    m_codeTable = new QTableView(centralWidget);
    mainLayout->addWidget(m_codeTable,1);

    // setup models
    m_regmodel = new RegModel(&m_vm);
    m_regTable->setModel(m_regmodel);


    centralWidget->setLayout(mainLayout);
}

MainWindow::~MainWindow()
{
    m_regTable->setModel(nullptr);
}

void MainWindow::OnFileNew()
{

}

void MainWindow::OnFileOpen()
{
    
    QSettings MySettings; // Will be using application informations
                          // for correct location of your settings

    QString SelectedFile  = QFileDialog::getOpenFileName(
        this, 
        tr("Open binary"), 
        MySettings.value(SettingsKeys::LAST_DIR_KEY).toString(), 
        tr("Binary files (*.bin);;All files (*.*)"), 
        0, QFileDialog::DontUseNativeDialog);

    if (!SelectedFile.isEmpty()) 
    {
        QDir CurrentDir;
        MySettings.setValue(SettingsKeys::LAST_DIR_KEY,
                            CurrentDir.absoluteFilePath(SelectedFile));

        qDebug() << "Loading " << SelectedFile;
    }
}
