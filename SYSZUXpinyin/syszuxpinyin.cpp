/***************************************************************************
**
**  <SYSZUXpinyin 1.0 , a chinese input method based on Qt for Embedded linux>
**  Copyright (C) <2010> <Gemfield> <gemfield@civilnet.cn>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License version 3 as published
**  by the Free Software Foundation.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
**  If you have questions regarding the use of this file, please contact
**  Gemfield at gemfield@civilnet.cn or post your questions at
**  http://civilnet.cn/syszux/bbs
**
****************************************************************************/

#include <QtGui>
#include "syszuxpinyin.h"
#include "syszuxim.h"
QString syszux_lower_letter[52]={"1","2","3","4","5","6","7","8","9","0","-","=","backspace","q","w","e","r","t","y","u","i",
                            "o","p","[","]","\\","a","s","d","f","g","h","j","k","l",";","\'","enter","z","x","c","v",
                            "b","n","m",",",".","/","shift"," "};
QString syszux_upper_letter[52]={"!","@","#","$","%","^","&&","*","(",")","_","+","backspace","Q","W","E","R","T","Y","U","I",
                            "O","P","{","}","|","A","S","D","F","G","H","J","K","L",":","\"","enter","Z","X","C","V",
                            "B","N","M","<",">","?","SHIFT"," "};
SyszuxPinyin::SyszuxPinyin(QWSInputMethod* im) :QDialog(0,Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint),button_group(new QButtonGroup(this)),input_method(0),lower_upper(0),page_count(0)
{
    mMoveing=false;
    int ret=0;
    setupUi(this);
    //resize(800,480);
    //setWindowOpacity(0.8);//窗口整体透明
    //setAttribute(Qt::WA_TranslucentBackground, true);//窗口透明度，控件不透明
    this->setStyleSheet("#Dialog{background-color: rgb(255, 85, 255);}");//设置窗口背景不影响控件

    initGb();
    pinyin_file.setFileName(":/syszux/syszuxpinyin.utf8");

    if( !pinyin_file.open(QIODevice::ReadOnly) )
        QMessageBox::warning(0,tr("syszuxpinyin"),tr("can't load"));

    regExp.setCaseSensitivity(Qt::CaseSensitive);
    regExp.setPattern(QString("([a-z]+)"));

    while(!pinyin_file.atEnd())
    {
        QByteArray data = pinyin_file.readLine();
        ret = regExp.indexIn(QString(data.data()),0,QRegExp::CaretAtZero);
        pinyin_map.insert(regExp.cap(1),data.left(ret));
    }
    connect(this,SIGNAL(sendPinyin(QString)),im,SLOT(confirmString(QString)));

    //Qt::WindowStaysOnTopHint 窗口在最顶端，不会拖到任务栏下面
    //setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint |Qt::WindowStaysOnTopHint);
    //this->setStyleSheet("QDialog{border:2px solid green;}");
}
SyszuxPinyin::~SyszuxPinyin()
{
}
void SyszuxPinyin::initGb()
{
    QPushButton *pushButton=new QPushButton(this);
    pushButton->hide();
    pushButton=pushButton_hanzi_left;//第一个按钮
    //所有按键添加到一个按键组，并编号
    //下一个按钮是什么，获得的顺序可以在.ui文件中查看，顺序有对应关系，不要改动
    for(int i=1;i<67;i++)
    {
        button_vector.push_back(pushButton);//按键容器
        button_group->addButton(pushButton,i);//按键组
        pushButton=qobject_cast<QPushButton *>(pushButton->nextInFocusChain());//下一个按钮
    }
    connect(button_group,SIGNAL(buttonClicked(int)),SLOT(buttonClickResponse(int)));//有按键按下的信号槽
}

//按键按下消息处理
void SyszuxPinyin::buttonClickResponse(int gemfield)
{
    if(gemfield==1)//按键<-
    {
        selectHanziPre();
        return;
    }
    else if(gemfield==10)//案件->
    {
        selectHanziNext();
        return;
    }
    else if(gemfield<10)//汉字候选按钮
    {
        lineEdit_window->insert(button_vector.at(gemfield-1)->text());//选择的汉字添加到输入框
        lineEdit_pinyin->clear();//清除拼音输入框
        clearString();////清除汉字候选控件内容
        return;
    }
    else if(gemfield==23)//按键backspace
    {
        deleteString();
        return;
    }
    else if(gemfield==59)//按键shift
    {
         changeLowerUpper();
         return;
    }
    else if(gemfield!=47)//按键Enter
    {

    }
    else if(gemfield>10 && gemfield<=60)//非功能按键（0~9，a~z,标点）
    {
        if(lower_upper)//小写模式时
            event=new QKeyEvent(QEvent::KeyPress, 0, Qt::NoModifier,syszux_upper_letter[gemfield-11]);
        else            //大写模式
            event=new QKeyEvent(QEvent::KeyPress, 0, Qt::NoModifier,syszux_lower_letter[gemfield-11]);
    }
    else if(gemfield==61)//中英模式切换按键
    {
        changeInputMethod();
        return;
    }
    else if(gemfield==62)//按键ok
    {
        affirmString();
        return;
    }
    else if(gemfield>62)//方向按键
    {
        switch(gemfield)
        {
        case 63:
            event=new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
            break;
        case 64:
            event=new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
            break;
        case 65:
            event=new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
            break;
        case 66:
            event=new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
            break;
        }
    }
    if(input_method)//中文输入模式
    {
        lineEdit_pinyin->setFocus();
        QApplication::sendEvent(focusWidget(),event);//向当前焦点控件发送按键消息
        matching(lineEdit_pinyin->text());//匹配汉字并显示
    }
    else//英文输入模式
    {
        lineEdit_window->setFocus();
        QApplication::sendEvent(focusWidget(),event);
    }
}

//通过输入的拼音匹配汉字
void SyszuxPinyin::matching(QString gemfield)
{
    pinyin_list = pinyin_map.values(gemfield);
    changePage(0);//显示匹配到的汉字第一页
    page_count=0;//记录当前汉字页
}

//匹配到的汉字列表分页显示
void SyszuxPinyin::changePage(int index)
{
    int count = pinyin_list.size();
    int i=index*8,j=0;
    while(++j != 9 )
        button_vector.at(j)->setText(tr(pinyin_list.value(count-(++i)).toAscii()));
    if(index==0)
        pushButton_hanzi_left->setEnabled(false);//第一页时不能使能<-按键
    else
        pushButton_hanzi_left->setEnabled(true);
    if(pinyin_list.size()>(index*8+8))
        pushButton_hanzi_right->setEnabled(true);
    else
        pushButton_hanzi_right->setEnabled(false);//最后一页时不能使能->按键
}

void SyszuxPinyin::selectHanziPre()
{
    changePage(--page_count);
}

void SyszuxPinyin::selectHanziNext()
{
    changePage(++page_count);
}
//清除汉字候选控件内容
void SyszuxPinyin::clearString()
{
    int i=0;
    while(++i!=9)
        button_vector.at(i)->setText("");
}

//中/英输入模式切换
void SyszuxPinyin::changeInputMethod()
{
    if(pushButton_shift->text()=="SHIFT")//shift大写输入模式时，不能中英切换
        return;
   lineEdit_pinyin->clear();
   if(pushButton_is_hanzi->text()=="Eng")
       input_method=1,pushButton_is_hanzi->setText("CH");
   else
       input_method=0,pushButton_is_hanzi->setText("Eng");
}

//大/小写输入模式切换
void SyszuxPinyin::changeLowerUpper()
{
    if(pushButton_shift->text()=="shift")//切换到大写模式
    {
        lower_upper=1,pushButton_shift->setText("SHIFT");
        input_method=0,pushButton_is_hanzi->setText("Eng");//输入法改为英文输入
        int i=9;
        while(++i!=59)//设置大写模式时的按键显示字符
            button_vector.at(i)->setText(syszux_upper_letter[i-10]);
    }
    else//切换到小写模式
    {
        lower_upper=0,pushButton_shift->setText("shift");
        int i=9;
        while(++i!=59)//设置小写模式时的按键显示字符
            button_vector.at(i)->setText(syszux_lower_letter[i-10]);
    }
}

//退格键消息处理
void SyszuxPinyin::deleteString()
{
    event=new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    //如果输入法窗口的输入框和拼音输入框都为空，删除当前焦点的控件的内容
    if(lineEdit_pinyin->text().isEmpty() && lineEdit_window->text().isEmpty())
    {
        QApplication::sendEvent(QApplication::focusWidget(),event);
    }
    else if(input_method)//中文输入模式，先删除拼音输入框内容，再删除输入框内容
    {
        lineEdit_pinyin->text().isEmpty()?lineEdit_window->setFocus():lineEdit_pinyin->setFocus();
        QApplication::sendEvent(focusWidget(),event);
        matching(lineEdit_pinyin->text());
    }
    else//英文输入模式，直接删除输入框内容
    {
        lineEdit_window->setFocus();
        QApplication::sendEvent(focusWidget(),event);
    }
}

//ok按键
void SyszuxPinyin::affirmString()
{
    emit sendPinyin(lineEdit_window->text());//发送完成输入信号
    lineEdit_window->clear();
    lineEdit_pinyin->clear();
    this->hide();//隐藏输入法窗口
}


//重写鼠标按下事件
void SyszuxPinyin::mousePressEvent(QMouseEvent *event)
{
    mMoveing = true;
    //记录下鼠标相对于窗口的位置
    //event->globalPos()鼠标按下时，鼠标相对于整个屏幕位置
    //pos() this->pos()鼠标按下时，窗口相对于整个屏幕位置
    mMovePosition = event->globalPos() - pos();
    return QDialog::mousePressEvent(event);
}

//重写鼠标移动事件
void SyszuxPinyin::mouseMoveEvent(QMouseEvent *event)
{
    //(event->buttons() && Qt::LeftButton)按下是左键
    //鼠标移动事件需要移动窗口，窗口移动到哪里呢？就是要获取鼠标移动中，窗口在整个屏幕的坐标，然后move到这个坐标，怎么获取坐标？
    //通过事件event->globalPos()知道鼠标坐标，鼠标坐标减去鼠标相对于窗口位置，就是窗口在整个屏幕的坐标
    if (mMoveing && (event->buttons() && Qt::LeftButton)
        && (event->globalPos()-mMovePosition).manhattanLength() > QApplication::startDragDistance())
    {
        move(event->globalPos()-mMovePosition);
        mMovePosition = event->globalPos() - pos();
    }
    return QDialog::mouseMoveEvent(event);
}
void SyszuxPinyin::mouseReleaseEvent(QMouseEvent *event)
{
    mMoveing = false;
}


