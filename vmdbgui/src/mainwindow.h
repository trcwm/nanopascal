#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include "opcodes.h"
#include "regmodel.h"
#include "stackmodel.h"
#include "instrmodel.h"
#include "vmwrapper.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    QTableView *m_regTable;
    QTableView *m_stackTable;
    QTableView *m_codeTable;

    RegModel    *m_regmodel;
    StackModel  *m_stackmodel;
    InstrModel  *m_codemodel;
    VMWrapper m_vm;

private:

private slots:
    void OnStep();
    void OnFileNew();
    void OnFileOpen();
};

#endif // MAINWINDOW_H
