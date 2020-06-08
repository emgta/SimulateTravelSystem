#include "search.h"
#include "ui_search.h"
#include "widget.h"

Search::Search(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Search)
{
    ui->setupUi(this);
}

Search::~Search()
{
    delete ui;
}

void Search::Show_Route(std::multimap<int, Attribute>database)
{
    qDebug() << "Search route successfully";
    ui->Route->clearContents();
    int rowIndex = 0;
    std::multimap<int,Attribute>::iterator iter = database.find(from);
    for(int k = 0;k != database.count(from);k++,iter++){

        if(iter->second.to==to){

            rowIndex+=1;
            ui->Route->setRowCount(rowIndex);

            if (iter->second.vehicle == 0)
                ui->Route->setItem(rowIndex-1,0,new QTableWidgetItem(QString::fromWCharArray(L"(汽)") +iter->second.num));
            else if (iter->second.vehicle == 1)
                ui->Route->setItem(rowIndex-1,0,new QTableWidgetItem(QString::fromWCharArray(L"(铁)") +iter->second.num));
            else if (iter->second.vehicle == 2)
                ui->Route->setItem(rowIndex-1,0,new QTableWidgetItem(QString::fromWCharArray(L"(机)") +iter->second.num));
            ui->Route->setItem(rowIndex-1,1,new QTableWidgetItem(iter->second.begin.toString()));
            ui->Route->setItem(rowIndex-1,2,new QTableWidgetItem(iter->second.end.toString()));
            ui->Route->setItem(rowIndex-1,3,new QTableWidgetItem(QString::fromWCharArray(L"￥")+QString::number(iter->second.cost)));
        }

    }
     ui->Route->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
     ui->Route->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
     ui->Route->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
     ui->Route->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

int Search::Get_from()
{
    return ui->from->currentIndex();
}

int Search::Get_to()
{
    return ui->to->currentIndex();
}

void Search::on_searchBtn_clicked()
{
    from = Get_from();
    to = Get_to();
    Show_Route(Schedule::database);
}
void Search::on_Route_cellDoubleClicked(int row,int column){

}


