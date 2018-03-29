#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
//#include <QSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QVariantList>
#include <QTableWidget>
#include <QWSInputMethod>
#include <syszuxim.h>
#include <QWSServer>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle("MyQt");
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(aboutSlot()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(exitSlot()));
    connect(ui->pushButtonInsert, SIGNAL(clicked()), this, SLOT(insertBtnSlot()));
    createConnectToDb();
    initUsersTable();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::aboutSlot()
{
    QMessageBox::about(this, tr("关于"), (tr("我的Qt测试APP.")));
}

void MainWindow::exitSlot()
{
    this->close();
}


bool MainWindow::createConnectToDb(QString dbName, QString connectName)
{
    database = QSqlDatabase::addDatabase("QSQLITE", connectName);
    database.setDatabaseName(dbName);
    if(!database.open())
    {
        qDebug()<<database.lastError();
        QMessageBox::warning(this, tr("错误"), tr("Connect to database failed"));
    }
    //检查表是否已创建
    if(database.tables().contains("users"))
    {
           return false;
    }
    //创建表
    QSqlQuery sql_query(database);
    QString create_sql = "CREATE TABLE users (id int primary key, name varchar(30), address varchar(255))"; //创建数据表
    if(!sql_query.exec(create_sql))
    {
        return false;
    }
    return true;
}

void MainWindow::insertBtnSlot()
{
    QString name = ui->lineEditUserName->text().simplified();
    if(name.isEmpty())
    {
        QMessageBox::warning(this, tr("警告"), tr("用户名输入不能为空"));
        return;
    }
    if(isUserExist(name))//存在
    {
        QMessageBox::warning(this, tr("警告"), tr("用户已存在"));
        return;
    }
    QString addr = ui->lineEditUserAddr->text();
    QSqlQuery query(database);
    //获得下一个id
    qint64 id = getNextId("users");
    if(id < 0)
        return;
    //插入数据
    query.prepare("INSERT INTO users (id, name, address) VALUES (:id, :name, :address)");
    query.bindValue(":id", id);
    query.bindValue(":fname", name);
    query.bindValue(":address", addr);
    //QString sql = QString("INSERT INTO users (id, name, address) VALUES ('%1', '%2', '%3')").arg(id).arg(name, addr);
    if(!query.exec())
        QMessageBox::warning(this, tr("警告"), QString("insert error:%1").arg(query.lastError().text()));
    showUsersMsg();
}

//获得一个表的下一个id
qint64 MainWindow::getNextId(QString tableName)
{
    if(tableName.isEmpty())
    {
        return -1;
    }
    QString sql = QString("select max(id) from %1").arg(tableName);
    QSqlQuery query(database);
    if(!query.exec(sql))
    {
        return -1;
    }
    if(!query.next())//空表
    {
        return 1;
    }
    return query.value(0).toLongLong()+1;
}

void MainWindow::showUsersMsg()
{
    QSqlQuery query(database);
    if(query.exec("SELECT * FROM users;"))
    {
        int row = 0;
        //ui->tableWidgetUsers->clear();
        //ui->tableWidgetUsers->setHorizontalHeaderLabels(QStringList()<<"id"<<"name"<<"address");
        ui->tableWidgetUsers->clearContents();
        while(query.next())
        {
            setTableText(row, 0, query.value(0).toString());
            setTableText(row, 1, query.value(1).toString());
            setTableText(row, 2, query.value(2).toString());
            row++;
        }
    }
}

void MainWindow::setTableText(int row, int column, QString text)
{
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setText(text);
    if(ui->tableWidgetUsers->rowCount() <=row)
    {
        ui->tableWidgetUsers->setRowCount(row + 1);
    }
    ui->tableWidgetUsers->setItem(row, column, item);
}


void MainWindow::initUsersTable()
{
    ui->tableWidgetUsers->setHorizontalHeaderLabels(QStringList()<<tr("id")<<tr("name")<<tr("address"));
    ui->tableWidgetUsers->setRowCount(5);
    ui->tableWidgetUsers->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    showUsersMsg();
}


void MainWindow::on_pushButtonDelete_clicked()
{
    qint32 row = ui->tableWidgetUsers->currentRow();
    if(row < 0)
        return;
    QTableWidgetItem * item = ui->tableWidgetUsers->item(row, 0);
    if(item == 0)
        return;
    QString strid = item->text();
    if(strid.isEmpty())
        return;
    //qint64 id = strid.toLongLong();
    QSqlQuery query(database);
    if(query.exec(QString("DELETE FROM users WHERE id=%1").arg(strid)))
    {
        showUsersMsg();
    }
    else
    {
        QMessageBox::warning(this, tr("警告"), QString(tr("删除错误：%1")).arg(query.lastError().text()));
    }
}

//出错或存在返回true
bool MainWindow::isUserExist(QString userName)
{
    QSqlQuery query(database);
    if(!query.exec(QString("SELECT count(*) FROM users WHERE name='%1'").arg(userName)))
        return true;
    query.next();
    qint32 count = query.value(0).toInt();
    qDebug()<<count;
    if(count == 0)
        return false;
    return true;
}

/*int main(int argc, char *argv[])
{
    if(database.open())
    {
        qDebug()<<"Database Opened";

        QSqlQuery sql_query;
        QString create_sql = "create table member (id int primary key, name varchar(30), address varchar(30))"; //创建数据表
        QString insert_sql = "insert into member values(?,?,?)";    //插入数据

        QString select_all_sql = "select * from member";

        sql_query.prepare(create_sql); //创建表
        if(!sql_query.exec()) //查看创建表是否成功
        {
            qDebug()<<QObject::tr("Table Create failed");
            qDebug()<<sql_query.lastError();
        }
        else
        {
            qDebug()<< "Table Created" ;

            //插入数据
            sql_query.prepare(insert_sql);

            QVariantList GroupIDs;
            GroupIDs.append(0);
            GroupIDs.append(1);
            GroupIDs.append(2);

            QVariantList GroupNames;
            GroupNames.append("hsp");
            GroupNames.append("rl");
            GroupNames.append("spl");

            QVariantList GroupAddress;
            GroupAddress.append("南充");
            GroupAddress.append("宝鸡");
            GroupAddress.append("南充");

            sql_query.addBindValue(GroupIDs);
            sql_query.addBindValue(GroupNames);
            sql_query.addBindValue(GroupAddress);
    */

