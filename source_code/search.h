#ifndef SEARCH_H
#define SEARCH_H

#include"widget.h"


#include <QDialog>

namespace Ui {
class Search;
}

class Search : public QDialog
{
    Q_OBJECT

public:
    explicit Search(QWidget *parent = nullptr);
    ~Search();



private slots:

    void on_searchBtn_clicked();

    void on_Route_cellDoubleClicked(int row, int column);

private:
    Ui::Search *ui;
    void Show_Route(std::multimap<int, Attribute>);
    int Get_from();
    int Get_to();

    int from;
    int to;
};

#endif // SEARCH_H
