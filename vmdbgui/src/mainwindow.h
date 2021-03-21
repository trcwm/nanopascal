#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include "opcodes.h"
#include "regmodel.h"
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
    QTableView *m_memTable;
    QTableView *m_codeTable;

    RegModel  *m_regmodel;
    VMWrapper m_vm;

private:

private slots:
    void OnFileNew();
    void OnFileOpen();
};

#endif // MAINWINDOW_H
