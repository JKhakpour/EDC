#include <QCoreApplication>
#include <QTimer>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <iostream>
#include <fstream>
#include <functional>
#include "leven.h"

QString input_file_1;
QString input_file_2;
QString output_file_path;

void AddToFile(QString &foo, const QString &lv_output)
{
    QFile leven_output_file(output_file_path);
    leven_output_file.open(QFile::ReadWrite | QFile::Text | QFile::Append);
    QTextStream leven_output_stream(&leven_output_file);
//    std::cout << "writing to output file!" << std::endl;
    leven_output_stream << lv_output;
    leven_output_file.close();
}

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = 0) : QObject(parent){}
//    QString input_file_1;
//    QString input_file_2;
//    QString output_file_path;
    std::vector<int> OPERS;
    int bsize;
    int max_threshold;

public slots:
    void run()
    {
        ComputeDistances();
        std::cout << "Done!" << std::endl;
        emit finished();
    }
public:
    QStringList get_vocab(QString input_file_path)
    {
        QFile vocabulary(input_file_path);
        if(!vocabulary.open(QFile::ReadOnly | QFile::Text))
        {
            std::cout << "couldnt open input file!" << std::endl;
            return QStringList();
        }
        QTextStream input_stream(&vocabulary);
        QStringList string_list;
        while (!input_stream.atEnd())
            string_list << input_stream.readLine();
        vocabulary.close();
        return string_list;
    }

    void ComputeDistances()
    {
        QStringList string_list_row = get_vocab(input_file_1);
        QStringList string_list_col = get_vocab(input_file_2);
        int L1 = string_list_row.size();
        int L2 = string_list_row.size();
        int i = 0;
        int c = 0;
        QList<Leven> leven_objs;
        while(i < L1)
        {
             int row_start = i;
             int row_end = std::min(i + bsize, L1);
             int j = i;
             while (j < L2)
             {
                 int col_start = std::max(i+1,j);
                 int col_end = std::min(j + bsize, L2);
                 int slices[4] = {row_start, row_end, col_start, col_end};
                 leven_objs.append(Leven(&string_list_row, &string_list_col, slices, this->OPERS));
                 j += bsize;
                 c++;
             }
             i += bsize;
        }
        QtConcurrent::blockingMappedReduced(leven_objs, &Leven::ComputeDistances, &AddToFile, QtConcurrent::UnorderedReduce);
    }


signals:
    void finished();
};

#include <main.moc>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("Edit Distance ");
    QCoreApplication::setApplicationVersion("0.0.9");
    Task *task = new Task(&a);
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));
    QTimer::singleShot(0, task, SLOT(run()));


    QCommandLineParser parser;
    parser.setApplicationDescription("");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("dest file", "main", "file to write the ouput");
    parser.addPositionalArgument("source 1", "List Number 1 of strings(one per line)");
    parser.addPositionalArgument("source 2", "List Number 2 of strings(one per line)");


    QCommandLineOption max_threshold_option("t", QString("Maximum edit distance threshold to write into file. Ignores edit distances bigger than threshhold value. default: %1").arg(MAX_THRESHOLD_DEFAULT), "treshold");
    parser.addOption(max_threshold_option);

    QCommandLineOption bucket_size_option("b", QString("Bucket size, Application breaks source file to sections of bukets size to process them in parallel. default: %1").arg(BUCKET_SIZE_DEFAULT), "bucket");
    parser.addOption(bucket_size_option);

    QCommandLineOption opers_option(QStringList() << "o" << "operations", QString("Operation used in Edit Distance Calculation. Order of operations changes their priority in returned path(and path itself!).\nIf you don't pass any of a(Add) or d(Delete), program will prioritise them as first and second operations(and the rest goes after them).\n Allowed Operations are:\n\t   a: add\n\t   d: delete\n\t   s: Substitute\n\t   t: Transform\n default: adst"), "operations");
    parser.addOption(opers_option);

    // Process the actual command line arguments given by the user
    parser.process(a);

    const QStringList args = parser.positionalArguments();
    if(args.length() < 2){
        std::cout << "An output file and at least one source is required." << std::endl;
        parser.showHelp(1);
    }else if(args.length() > 3){
        std::cout << "Received more than 3 files! One output file and one or two source file is needed." << std::endl;
        parser.showHelp(1);
    }

    output_file_path = args.at(0);
    input_file_1 = args.at(1);
    input_file_2 = args.at(args.length()-1); // either second or third argument
    if (parser.isSet(max_threshold_option)){
        task->max_threshold = parser.value(max_threshold_option).toInt();
    }else{
        task->max_threshold = MAX_THRESHOLD_DEFAULT;
    }
    if (parser.isSet(bucket_size_option)){
        task->bsize = parser.value(bucket_size_option).toInt();
    }else{
        task->bsize = BUCKET_SIZE_DEFAULT;
    }
    if (parser.isSet(opers_option)){
        QString operations = parser.value(opers_option);
        QHash<QChar, int> operations_qstr({{'a', ADD}, {'d', DEL}, {'s', SUB}, {'t', TRANS}});
        if (!operations.contains('a')){
            task->OPERS.push_back(ADD);
        }
        if (!operations.contains('d')){
            task->OPERS.push_back(DEL);
        }

        while(!operations.isEmpty()){
            QChar op = operations[0];
            if(operations_qstr.contains(op)){
                if(std::find(task->OPERS.begin(), task->OPERS.end(), operations_qstr[op]) == task->OPERS.end()){
                    task->OPERS.push_back(operations_qstr[op]);
                    operations.remove(0,1);

                }else{
                    std::cout << QString("duplicate '%1' value for operations").arg(operations_qstr[op]).toStdString() << std::endl;
                    parser.showHelp(1);
                }
            }else{
                std::cout << QString("unknown operations %1!").arg(op).toStdString() << std::endl;
                parser.showHelp(1);
            }
        }
    }else{
        task->OPERS = std::vector<int>(OPERATIONS, OPERATIONS + sizeof OPERATIONS / sizeof OPERATIONS[0]);
    }
    return a.exec();
}



