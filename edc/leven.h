#ifndef LEVEN_H
#define LEVEN_H

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QThread>
#include <QObject>
#include <QHash>
#include <array>
#include <utility>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <limits>


const int PASS = 0;
const int DEL = 1;
const int ADD = 2;
const int SUB = 3;
const int TRANS = 4;
const int PASS_REVERSE = PASS;
const int DEL_REVERSE = ADD;
const int ADD_REVERSE = DEL;
const int SUB_REVERSE = SUB;
const int TRANS_REVERSE = TRANS;
const int OPERATIONS[] = {ADD, DEL, SUB, TRANS};
const int BigM = std::numeric_limits<int>::max();
const int MAX_THRESHOLD_DEFAULT = 15;
const int BUCKET_SIZE_DEFAULT = 1000;


int inline action_reverse(int op)
{
    switch(op)
    {
        case PASS  : return PASS_REVERSE;
        case ADD   : return ADD_REVERSE;
        case DEL   : return DEL_REVERSE;
        case SUB   : return SUB_REVERSE;
        case TRANS : return TRANS_REVERSE;
    }

}

int inline cost(int op)
{
    switch(op)
    {
        case PASS  : return PASS_REVERSE;
        case ADD   : return ADD_REVERSE;
        case DEL   : return DEL_REVERSE;
        case SUB   : return SUB_REVERSE;
        case TRANS : return TRANS_REVERSE;
    }

}


const QString PHI = QString("ϕ");


class Leven
{
//    Q_OBJECT
public: 
    Leven(QStringList* string_list_row, QStringList *string_list_col, int slice_indexes[4], std::vector<int> operations, int max_threshold = MAX_THRESHOLD_DEFAULT);
    QString ComputeDistances() const;
//    QTextStream *input_stream;
//    QTextStream stream_in;
    std::vector<int> allowed_operations;
    int max_threshold;
    QStringList* string_list_row;
    QStringList* string_list_col;
    std::vector<QString> vocab;
    bool _computation_completed;
    void start_compute(int vocab_size = -1);
    std::vector<std::pair<QString,QString>> operations;
    int row_start, row_end, col_start, col_end;

};


int distance(QString w1, QString w2);
int dist(QString seq1, QString seq2, std::vector<int> allowed_opers, std::vector<int> *opers);
int dist(QString seq1, QString seq2, std::vector<int>* opers);
std::vector<int> fil_range(std::vector<int>* vec,int stop=-1, int start=0,int step=1);
inline void print_array(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end);
int action_reverse(int op);
int qgram_dist(QString s1, QString s2, int q);





#endif // LEVEN_H

