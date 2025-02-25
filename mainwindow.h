#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDateTimeEdit>
#include <QTimer>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QScrollArea>
#include <QComboBox>
#include <QSqlTableModel>
#include <QTableView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Task {
public:
    Task(QLabel *label, const QDateTime &dt, const QString &tag)  // 引数をconst参照に変更
        : taskLabel(label), deadline(dt), tagText(tag) {}

    QLabel *taskLabel;
    QDateTime deadline;
    QString tagText;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showInputFields();   // ＋ボタンで入力ボックスを表示
    void addTask();           // 追加ボタンでタスクを追加
    void editTask(int taskId, QString taskName, QString taskTag, QString taskDeadline); // 変更ボタンでタスクを編集
    void deleteTask(int taskId); // 削除ボタンでタスクを削除
    void checkReminders(); // 🔔 リマインダー確認
    void initializeDatabase();
    void saveTaskToDatabase(const QString &taskText, const QDateTime &deadline, const QString &tagText);
    void updateTaskList();
    void loadTasksFromDatabase();
    void completeTask(int taskId);
    void populateTagComboBox();
    void setupTaskTable();
    void sortTaskList(const QString &sortOption);

private:
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QDateTimeEdit *deadlineInput; // 期限入力用

    QPushButton *addInitialButton;
    QWidget *taskInputArea;
    QLineEdit *taskInput;
    QLineEdit *tagInput;

    QPushButton *addTaskButton;
    QTimer *reminderTimer; // ⏳ リマインダー用タイマー


    QVBoxLayout *taskListLayout;
    QList<Task> taskList;  // タスクリストをTask型に変更
    QComboBox *tagFilterComboBox;
    QComboBox *sortComboBox ;
    QSqlTableModel *model;
    QTableView *tableView;


};

#endif // MAINWINDOW_H
