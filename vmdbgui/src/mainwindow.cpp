#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QSettings>
#include <QStatusBar>
#include <QFileDialog>
#include <QBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QCoreApplication>

#include <memory>

#include "common.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle("VM Debugger");
    setMinimumSize(800,450);

	QMenuBar * menuBar = new QMenuBar(this);
	setMenuBar(menuBar);    

    // ------------------------------------------------------------
    //   File menu
    // ------------------------------------------------------------

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

    // ------------------------------------------------------------
    //   Debug menu
    // ------------------------------------------------------------

    menu = menuBar->addMenu("&Debug");
	// Step
	action = new QAction("Step", this);
    action->setShortcut(QKeySequence(Qt::Key_F10));
	connect(action, &QAction::triggered, this, &MainWindow::OnStep);
	menu->addAction(action);

    QStatusBar * statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // ------------------------------------------------------------
    //   build the GUI
    // ------------------------------------------------------------

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout();

    QVBoxLayout *cpuinfoLayout = new QVBoxLayout();
    m_regTable = new QTableView(centralWidget);
    m_regTable->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_regTable->horizontalHeader()->setStretchLastSection(true);
    cpuinfoLayout->addWidget(m_regTable,0);

    m_stackTable = new QTableView(centralWidget);
    m_stackTable->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_stackTable->horizontalHeader()->setStretchLastSection(true);
    cpuinfoLayout->addWidget(m_stackTable,2);

    mainLayout->addLayout(cpuinfoLayout);

    m_codeTable = new QTableView(centralWidget);
    m_codeTable->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_codeTable->horizontalHeader()->setStretchLastSection(true);    
    mainLayout->addWidget(m_codeTable,1);

    // setup models
    m_regmodel = new RegModel(&m_vm);
    m_stackmodel = new StackModel(&m_vm);
    m_codemodel = new InstrModel(&m_vm);

    m_regTable->setModel(m_regmodel);
    m_stackTable->setModel(m_stackmodel);
    m_codeTable->setModel(m_codemodel);

    centralWidget->setLayout(mainLayout);
}

MainWindow::~MainWindow()
{
    m_regTable->setModel(nullptr);
    m_stackTable->setModel(nullptr);
    m_codeTable->setModel(nullptr);
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

        FILE *fin = fopen(SelectedFile.toUtf8().data(), "rb");
        if (fin == nullptr)
        {
            qDebug() << "Error loading file " << SelectedFile;
            return;
        }

        fseek(fin,0,SEEK_END);
        size_t bytes = ftell(fin);
        rewind(fin);

        std::unique_ptr<uint8_t> data(new uint8_t[bytes]);
        if (fread(data.get(), 1, bytes, fin) != bytes)
        {
            qDebug() << "Error loading file " << SelectedFile;
            fclose(fin);
            return;            
        }

        m_vm.clear();
        m_vm.load(data.get(), bytes);

        m_regmodel->update();
        m_stackmodel->update();
        m_codemodel->update();

        fclose(fin);
    }
}

void MainWindow::OnStep()
{
    m_vm.step();
    m_codemodel->update();
    m_regmodel->update();
    m_stackmodel->update();
}
