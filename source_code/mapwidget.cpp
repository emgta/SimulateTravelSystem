﻿#include "mapwidget.h"

#include <QApplication>
#include <QStateMachine>
#include <QPushButton>
#include <QSignalTransition>
#include <QPropertyAnimation>
#include <QPainter>
#include <QState>
#include <QLabel>

//添加新的timer，使得绘图准确
MapWidget::MapWidget(QWidget *parent) :
    QWidget(parent), state(-1)
{
    this->setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Background, QBrush(QPixmap(":/map.jpg")));
    this->setPalette(palette);

    paintmstimer = new QTimer;
    paintmstimer->start(1000/60);
    QObject::connect(paintmstimer, SIGNAL(timeout()), this, SLOT(update()));
}

//绘图实践，绘制旅行过程
void MapWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    Widget *fatherPtr = (Widget *)parentWidget();
    if (fatherPtr->currentTraveler != -1)
    {
        painter.drawPixmap((setPointPos()), setPointGraph());
    }
}

//根据当前状态、交通方式决定图标
QPixmap MapWidget::setPointGraph()
{
    QPixmap pointGraph;
    switch(state)
    {
    case -2://arrived destination
        pointGraph = QPixmap(":/arrive.ico");
        break;
    case -1://pause waiting
        pointGraph = QPixmap(":/pause.ico");
        break;
    case 0:
        pointGraph = QPixmap(":/car.ico");
        break;
    case 1:
        pointGraph = QPixmap(":/train.ico");
        break;
    case 2:
        pointGraph = QPixmap(":/plane.ico");
        break;
    }

    return pointGraph;
}

//设置当前图标所处位置
QPointF MapWidget::setPointPos()
{
    Widget *fatherPtr = (Widget *)parentWidget();
    static QPointF pointPos;
    std::vector<Attribute> path = fatherPtr->travelers[fatherPtr->currentTraveler].getPlan();
    QDateTime spenttime = fatherPtr->travelers[fatherPtr->currentTraveler].usedTime;
    QDateTime starttime = fatherPtr->travelers[fatherPtr->currentTraveler].startTime;

    //已用时间不小于总时间，当前位置为目的地
    if(spenttime >= fatherPtr->travelers[fatherPtr->currentTraveler].totalTime)
    {
         if(spenttime == fatherPtr->travelers[fatherPtr->currentTraveler].totalTime)
         {
              qDebug() << "Arriving at destination" << path[path.size()-1].to;
         }
         pointPos = getCityCor(path[path.size()-1].to);
         state = -2;
    }
    else
        for (std::vector<Attribute>::size_type index = 0;
            index != path.size(); index++)
        {
            QDateTime departuredatetime = fatherPtr->travelers[fatherPtr->currentTraveler].getCityDepartureDateTime(path[index].from);
            QDateTime cityarrivaltime = fatherPtr->travelers[fatherPtr->currentTraveler].getCityArrivalDateTime(path[index].to);
            QDateTime start2Begin = getSplitTime(starttime, departuredatetime);
            QDateTime start2End = getSplitTime(starttime, cityarrivaltime);
            //已用时间不超过一段路径发车时间，状态为等待
            if (spenttime <= start2Begin)
            {
                pointPos = getCityCor(path[index].from);
                state = -1;
                break;
            }
            //已用时间不超过一段路径的到站时间，状态为运动中
            else if (spenttime <= start2End)
            {
                pointPos = getCityCor(path[index].from);
                pointPos += getMoveDistance(spenttime, start2Begin, start2End, path[index].from, path[index].to);
                state = path[index].vehicle;
                if (spenttime == start2End)
                {
                    qDebug() << "Arriving at " << path[index].to;
                }
                break;
            }
        }
    return pointPos;
}

//中途更改计划时调用，用于获得新计划的始发地
int MapWidget::nextCity()
{
    Widget *fatherPtr = (Widget *)parentWidget();
    std::vector<Attribute> path = fatherPtr->travelers[fatherPtr->currentTraveler].getPlan();
    int nextCity2Arrive;
    QDateTime spenttime = fatherPtr->Get_Spent_Time();

    //已经到达目的地，则无法改变目的地
    if(spenttime >= fatherPtr->travelers[fatherPtr->currentTraveler].totalTime)
    {
         nextCity2Arrive = -1;
         state = -2;
    }
    else
        for (std::vector<Attribute>::size_type index = 0;
            index != path.size(); index++)
        {
            QDateTime starttime = fatherPtr->travelers[fatherPtr->currentTraveler].startTime;
            QDateTime departuredatetime = fatherPtr->travelers[fatherPtr->currentTraveler].getCityDepartureDateTime(path[index].from);
            QDateTime cityarrivaltime = fatherPtr->travelers[fatherPtr->currentTraveler].getCityArrivalDateTime(path[index].to);
            //当前处于等待状态，新计划始发地为当前所处地点
            if (spenttime <= getSplitTime(starttime, departuredatetime))
            {
                nextCity2Arrive = path[index].from;
                state = -1;
                break;
            }
            //当前处于运行状态，新计划为即将到达的城市
            else if (spenttime <=
                     getSplitTime(starttime, cityarrivaltime))
            {
                nextCity2Arrive = path[index].to;
                state = path[index].vehicle;
                break;
            }
        }
    qDebug() << "Next city to arrive is " << nextCity2Arrive;
    return nextCity2Arrive;
}

//获得两时间点之间的时间差，判断当前所处的状态
QDateTime MapWidget::getSplitTime(QDateTime former, QDateTime later)
{
    int durationSec = (later.time().second() - former.time().second());
    int durationMin = (later.time().minute() - former.time().minute() - (int)((durationSec >= 0)?0:1));
    int durationHour = (later.time().hour() - former.time().hour() - (int)((durationMin >= 0)?0:1));
    int durationDay = (later.date().day() - former.date().day() - (int)((durationHour >= 0)?0:1) + former.date().daysInMonth())
            % former.date().daysInMonth();
    durationSec = (durationSec + 60) % 60;
    durationMin = (durationMin + 60) % 60;
    durationHour = (durationHour + 24) % 24;

    return QDateTime(QDate(1, 1, durationDay+1), QTime(durationHour, durationMin, durationSec, 999));
}

//获得图形中各个城市的位置
QPointF MapWidget::getCityCor(int city)
{
    int x, y;
    switch (city)
    {
    case 0: //北京
        x = 695 - 22;
        y = 318 - 24;
        break;
    case 1: //上海
        x = 807 - 22;
        y = 498 - 24;
        break;
    case 2: //广州
        x = 686 -22;
        y = 668 -24;
        break;
    case 3: //重庆
        x = 550 - 22;
        y = 545 - 24;
        break;
    case 4: //青岛
        x = 758 - 22;
        y = 390 - 24;
        break;
    case 5: //郑州
        x = 668 - 22;
        y = 438 - 24;
        break;
    case 6: //兰州
        x = 503 - 22;
        y = 422 - 24;
        break;
    case 7: //乌鲁木齐
        x = 232 - 22;
        y = 255 - 24;
        break;
    case 8: //贵阳
        x = 565 - 22;
        y = 610 - 24;
        break;
    case 9: //哈尔滨
        x = 826 - 22;
        y = 150 - 24;
        break;
    }

    return QPointF(x, y);
}

///获得两个时间段的时间差，用于计算坐标增量
double MapWidget::getTimeDifference(QDateTime former, QDateTime later)
{

    int durationSec = (later.time().second() - former.time().second());
    int durationMin = (later.time().minute() - former.time().minute() - (int)((durationSec >= 0)?0:1));
    int durationHour = (later.time().hour() - former.time().hour() - (int)((durationMin >= 0)?0:1));
    int durationDay = (later.date().day() - former.date().day() - (int)((durationHour >= 0)?0:1) + former.date().daysInMonth())
            % former.date().daysInMonth();
    durationSec = (durationSec + 60) % 60;
    durationMin = (durationMin + 60) % 60;
    durationHour = (durationHour + 24) % 24;

    return (double)(durationDay * 86400 + durationHour * 3600 + durationMin * 60 + durationSec);
}

//计算坐标增量
QPointF MapWidget::getMoveDistance(QDateTime spentTime, QDateTime start2Begin, QDateTime start2End,
                                   int from, int to)
{
    double increaseRatio = getTimeDifference(start2Begin, spentTime)/getTimeDifference(start2Begin, start2End);
    double xIncrease = (getCityCor(to) - getCityCor(from)).x() * increaseRatio;
    double yIncrease = (getCityCor(to) - getCityCor(from)).y() * increaseRatio;

    return QPointF(xIncrease, yIncrease);
}
