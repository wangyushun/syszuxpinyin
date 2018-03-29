#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool createConnectToDb(QString dbName="db.sqlite3", QString connectName="");
    qint64 getNextId(QString tableName);
    void showUsersMsg();
    void setTableText(int row, int column, QString text);
    void initUsersTable();
    bool isUserExist(QString userName);

public slots:
    void aboutSlot();
    void exitSlot();
    void insertBtnSlot();

    
private slots:
    void on_pushButtonDelete_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase database;
};

#endif // MAINWINDOW_H





