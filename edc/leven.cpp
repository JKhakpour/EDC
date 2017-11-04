#include "leven.h"
#include <sstream>

std::vector<int> OPERS(OPERATIONS, OPERATIONS + sizeof OPERATIONS / sizeof OPERATIONS[0]);

Leven::Leven(QStringList* string_list_row,QStringList* string_list_col, int slice_indexes[4], std::vector<int> operations, int max_threshold)
{
    this->max_threshold = max_threshold;
    this->allowed_operations = operations;
    this->string_list_row = string_list_row;
    this->string_list_col = string_list_col;
    row_start = slice_indexes[0];
    row_end   = slice_indexes[1];
    col_start = slice_indexes[2];
    col_end   = slice_indexes[3];
}

QString Leven::ComputeDistances() const
{
    std::vector<int> opers;
    int distace_len;
    QString output = "";
    QTextStream output_stream(&output);
    for(int i=row_start; i < row_end; i++ )
    {
        for(int j=std::max(i,col_start); j < col_end; j++ )
        {
            distace_len = dist((*string_list_row)[i], (*string_list_col)[j], this->allowed_operations, &opers);
            if(distace_len <= this->max_threshold)
            {
                output_stream << i << "\t" << j << "\t" << distace_len << "\t";
                for(std::vector<int>::const_iterator it = opers.begin(); it != opers.end(); it++)
                {
                    output_stream << *it ;
                }
                output_stream << "\n";
            }
            opers.clear();
        }
    }
    return output;
}

inline void print_array(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end)
{
    for(std::vector<int>::const_iterator it = begin; it != end; it++)
    {
        printf ("%-8d", *it);
    }
}


int fill_range(std::vector<int>* vec,int stop,int start,int step)
{
    if((stop - start)*step > 0)
    {
        int len = std::min(int(vec->size()), (stop - start -1)/step +1);

        for(int i=0; i < len; i++)
        {            
            (*vec)[i] = start;
            start += step;
        }
    }
}

int fill_value(std::vector<int>* vec, int stop = -1,int start = 0, int value = 0)
{
    int len = std::min(len, int(vec->size()));
    for(int i=1; i < len; i++)
    {
        (*vec)[i] = value;
    }
}


int dist(QString seq1, QString seq2, std::vector<int> allowed_opers, std::vector<int>* opers)
{

    int min_cost;

    if (seq2.size() < seq1.size())
    {
        min_cost = dist(seq2, seq1, allowed_opers, opers);
        std::transform(opers->begin(), opers->end(), opers->begin(), action_reverse);
        return min_cost;
    }
    // now we can assume l1 <= l2
    seq1 = PHI + seq1;
    seq2 = PHI + seq2;
    int l1 = seq1.size(), l2 = seq2.size();

    std::vector<int> *costs_two_ago = new std::vector<int>(l2);
    std::vector<int> *costs_one_ago = new std::vector<int>(l2);
    std::vector<int> *costs_this_row = new std::vector<int>(l2);
    std::vector<std::vector<int>> *opers_two_ago = new std::vector<std::vector<int>>(l2);
    std::vector<std::vector<int>> *opers_one_ago = new std::vector<std::vector<int>>(l2);
    std::vector<std::vector<int>> *opers_this_row = new std::vector<std::vector<int>>(l2);

    std::vector<int> *foo;//just used for swapping values
    std::vector<std::vector<int>> *opers_foo;//just used for swapping values


    for(int y=0; y<l2; y++)
    {
        (*opers_one_ago)[y] = std::vector<int>(y,ADD);
    }
    fill_range(costs_this_row,l2 +2,0,1);

    for (int x = 0; x < l1; x++)
    {

        foo = costs_two_ago;
        costs_two_ago = costs_one_ago;
        costs_one_ago = costs_this_row;
        costs_this_row = foo;
        fill_value(costs_this_row,l2+1);
        (*costs_this_row)[0] = x;

        opers_foo = opers_two_ago;
        opers_two_ago = opers_one_ago;
        opers_one_ago = opers_this_row;
        opers_this_row = opers_foo;
        (*opers_this_row)[0] = std::vector<int>(x,DEL);

        for (int y=1; y < l2; y++)
        {
            if (seq1[x] == seq2[y]){
                (*costs_this_row)[y] = (*costs_one_ago)[y-1];
                (*opers_this_row)[y] = (*opers_one_ago)[y-1];
                (*opers_this_row)[y].push_back(PASS);
            }
            else{
                std::vector<int> costs(allowed_opers.size());
                for(int i = 0; i < allowed_opers.size(); i++ ){
                    switch(allowed_opers[i]){
                    case DEL:
                        costs[i] = (*costs_one_ago)[y] + 1;
                        break;
                    case ADD:
                        costs[i] = (*costs_this_row)[y-1] + 1;
                        break;
                    case SUB:
                        costs[i] = (*costs_one_ago)[y-1] + 1;
                        break;
                    case TRANS:
                        if (x > 1 && y > 1 && seq1[x] == seq2[y-1] and seq1[x-1] == seq2[y]){
                            costs[i] = (*costs_two_ago)[y-2] + 1;
                        }
                        else{
                            costs[i] = BigM;
                        }
                        break;
                    }

                }
                std::vector<int>::iterator min_elem = std::min_element(std::begin(costs), std::end(costs));
                int min_index = std::distance(costs.begin(), min_elem);
                (*costs_this_row)[y] = *min_elem;

                switch(allowed_opers[min_index]){
                    case DEL :
                        (*opers_this_row)[y] = (*opers_one_ago)[y];
                        (*opers_this_row)[y].push_back(DEL);
                        break;
                    case ADD:
                            (*opers_this_row)[y] = (*opers_this_row)[y-1];
                            (*opers_this_row)[y].push_back(ADD);
                            break;

                    case TRANS:
                            (*opers_this_row)[y] = (*opers_two_ago)[y-2];
                            (*opers_this_row)[y].push_back(TRANS);
                            break;
                     case SUB:
                            (*opers_this_row)[y] = (*opers_one_ago)[y-1];
                            (*opers_this_row)[y].push_back(SUB);
                            break;
                 }
            }
        }
    }

    opers->insert(opers->end(),(*opers_this_row)[l2-1].begin(), (*opers_this_row)[l2-1].end());

    min_cost = (*costs_this_row)[l2-1];

    delete costs_one_ago;
    delete costs_this_row;

    delete opers_one_ago;
    delete opers_this_row;

    return min_cost;

}

int dist(QString seq1, QString seq2, std::vector<int>* opers){
    return dist(seq1, seq2, OPERS, opers);
}


QHash<QString, int> ngram(QString input, int q){
    QHash<QString, int> ng;
    int l = input.length() - q;
    if (l < 0){
        // do nothing, return empty hash
    }
    else if (l == 0){
        ng[input] = 1;
    }
    else{
        for (int i = 0; i <= l; i++) {
            if (ng.contains(input.mid(i, q))){
                ng[input.mid(i, q)]++;
            }
            else{
                ng[input.mid(i, q)] = 1;
            }
        }
    }
    return ng;
}

int qgram_dist(QString s1, QString s2, int q){
    if (!s1.length()) {
       return std::max(0, s2.length() - q + 1);
   }
   if (!s2.length()) {
       return std::max(0, s1.length() - q + 1);
   }
   if (s1 == s2) {
       return 0;
   }
   QHash<QString, int> ngram1 = ngram(s1, q);
   QHash<QString, int> ngram2 = ngram(s2, q);
   int similarity = 0;
   foreach(const QString &key, ngram1.keys()){
       if(ngram2.contains(key)){
           similarity += std::min(ngram1[key], ngram2[key]);
       }
   }

   return std::max(0, s1.length() - q + 1) + std::max(0, s2.length() - q + 1) - 2*similarity;
}


