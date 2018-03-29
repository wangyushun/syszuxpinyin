#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QWSServer>
#include <syszuxim.h>
#include <syszuxpinyin.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //中文显示支持
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));       //支持Tr中文
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));//支持中文文件名显示
    QFont font;
    font.setPixelSize(15);//
    //font.setFamily("simsun");//宋体
    font.setFamily("unifont");//
    font.setBold(false);
    a.setFont(font);

    //软件盘输入法
    QWSInputMethod* im = new SyszuxIM;
    QWSServer::setCurrentInputMethod(im);

    MainWindow w;
    //w.showFullScreen();//全屏
    //QWSServer::setCursorVisible(false); //不显示鼠标
    w.show();
    
    return a.exec();
}
