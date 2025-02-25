#include "mainwindow.h"
#include <QInputDialog>
#include <QDateTimeEdit>
#include <QMessageBox>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), centralWidget(new QWidget(this)), mainLayout(new QVBoxLayout)
{
    setCentralWidget(centralWidget);
    centralWidget->setLayout(mainLayout);


    //タグフィルター
    tagFilterComboBox = new QComboBox(this);

    tagFilterComboBox->addItem("すべてのタグ");  // 全件表示
    populateTagComboBox();  // タグ一覧を取得
    connect(tagFilterComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateTaskList);

    mainLayout->addWidget(tagFilterComboBox);

    // taskListLayout を初期化（1回だけ）
    taskListLayout = new QVBoxLayout();

    // 並び替え用のコンボボックスを追加
    QComboBox *sortComboBox = new QComboBox(this);
    sortComboBox->addItem("タスク名で並び替え");
    sortComboBox->addItem("締切日で並び替え");
    sortComboBox->addItem("タグで並び替え");
    mainLayout->addWidget(sortComboBox);

    // 並び替えの選択変更を接続
    connect(sortComboBox, &QComboBox::currentTextChanged, this, &MainWindow::sortTaskList);

    // データベースの初期化
    initializeDatabase();
    // 🔄 アプリ起動時にタスク一覧を更新
    updateTaskList();  // ここで既存のタスクを読み込む
    setupTaskTable();
    tableView->setVisible(false);



    QSqlQuery query;
    query.exec("ALTER TABLE tasks ADD COLUMN is_completed INTEGER DEFAULT 0;");

    while (query.next()) {
        QString taskName = query.value(1).toString();  // タスク名
        QString taskDeadline = query.value(2).toString();  // 締切
        QString taskTag = query.value(3).toString();  // タグ
        QLabel *taskLabel = new QLabel(taskName + " (" + taskTag + " - " + taskDeadline + ")", this);

        QDateTime deadlineDateTime = QDateTime::fromString(taskDeadline, "yyyy-MM-dd HH:mm:ss");
        if (deadlineDateTime < QDateTime::currentDateTime())  {
            taskLabel->setStyleSheet("color: red; font-weight: bold;");
        }
    }

    // +ボタン
    addInitialButton = new QPushButton("+", this);
    mainLayout->addWidget(addInitialButton);
    connect(addInitialButton, &QPushButton::clicked, this, &MainWindow::showInputFields);

    // タスク入力エリア（最初は非表示）
    taskInputArea = new QWidget(this);
    tagInput = new QLineEdit(this);  // タグ入力用の QLineEdit を追加
    tagInput->setPlaceholderText("タグを入力 (例: 仕事, 家事)");

    QHBoxLayout *inputLayout = new QHBoxLayout(taskInputArea);
    taskInput = new QLineEdit(this);
    deadlineInput = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    deadlineInput->setCalendarPopup(true);
    addTaskButton = new QPushButton("追加", this);

    inputLayout->addWidget(taskInput);
    inputLayout->addWidget(tagInput);  // タグ入力エリアを追加
    inputLayout->addWidget(addTaskButton);
    taskInputArea->setLayout(inputLayout);
    taskInputArea->hide();

    mainLayout->addWidget(taskInputArea);
    connect(addTaskButton, &QPushButton::clicked, this, &MainWindow::addTask);

    // taskListLayout を初期化した後、QScrollArea を作成
    QWidget *container = new QWidget();
    container->setLayout(taskListLayout);
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);

    // mainLayout に scrollArea を追加
    mainLayout->addWidget(scrollArea);

    // リマインダー用のタイマー
    reminderTimer = new QTimer(this);
    connect(reminderTimer, &QTimer::timeout, this, &MainWindow::checkReminders);
    reminderTimer->start(60000); // 1分ごとにチェック



    // QSS (スタイルシート) の設定
    this->setStyleSheet(
        "QWidget {"
        "  background-color: #f0f0f0;"
        "}"
        "QLineEdit {"
        "  border: 2px solid #bdc3c7;"
        "  border-radius: 6px;"
        "  padding: 6px;"
        "  font-size: 14px;"
        "}"
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border-radius: 8px;"
        "  padding: 8px 16px;"
        "  font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2980b9;"
        "}"
        );
}

MainWindow::~MainWindow() {

}

void MainWindow::showInputFields() {
    addInitialButton->hide();  // +ボタンを非表示
    taskInputArea->show();     // 入力エリアを表示
}

void MainWindow::addTask() {
    QSqlQuery query;

    QString taskText = taskInput->text().trimmed();
    QString tagText = tagInput->text().trimmed();
    QDateTime deadline = deadlineInput->dateTime();

    while (query.next()) {
        int taskId = query.value(0).toInt();  // タスクID
        QString taskName = query.value(1).toString();  // タスク名
        QString taskDeadline = query.value(2).toString();  // 締切
        QString taskTag = query.value(3).toString();  // タグ



        QLabel *taskLabel = new QLabel(taskName + " (" + taskTag + " - " + taskDeadline + ")", this);
        QDateTime deadlineDateTime = QDateTime::fromString(taskDeadline, "yyyy-MM-dd HH:mm:ss");
        if (deadlineDateTime < QDateTime::currentDateTime())  {
            taskLabel->setStyleSheet("color: red; font-weight: bold;");
        }
        QPushButton *editTaskButton = new QPushButton("編集", this);

        connect(editTaskButton, &QPushButton::clicked, this, [this, taskId, taskName, taskTag, taskDeadline]() {
            editTask(taskId, taskName, taskTag, taskDeadline);
        });

        QHBoxLayout *taskLayout = new QHBoxLayout();
        taskLayout->addWidget(taskLabel);
        taskLayout->addWidget(editTaskButton);

        QWidget *taskWidget = new QWidget();
        taskWidget->setLayout(taskLayout);
        taskListLayout->addWidget(taskWidget);
    }

    if (!taskText.isEmpty()) {
        int taskId = query.value(0).toInt();  // タスクID
        QString taskName = query.value(1).toString();  // タスク名
        QString taskDeadline = query.value(2).toString();  // 締切
        QString taskTag = query.value(3).toString();  //タグ
        saveTaskToDatabase(taskText, deadline, tagText); // データベースに保存

        qDebug() << "タスク追加:" << taskText << deadline.toString("yyyy/MM/dd HH:mm") << tagText;


        // **新しいタスクをその場でリストに追加**
        QWidget *taskRow = new QWidget(this);
        QHBoxLayout *taskLayout = new QHBoxLayout(taskRow);

        QLabel *taskLabel = new QLabel(taskText + " (期限: " + deadline.toString("yyyy/MM/dd HH:mm") + ")", this);
        QLabel *tagLabel = new QLabel(tagText.isEmpty() ? "なし" : tagText, this);
        tagLabel->setStyleSheet("color: #888; font-size: 12px;");

        QPushButton *editTaskButton = new QPushButton("変更", this);
        QPushButton *deleteTaskButton = new QPushButton("削除", this);
        QPushButton *completeTaskButton = new QPushButton("完了", this);

        taskLayout->addWidget(taskLabel);
        taskLayout->addWidget(tagLabel);
        taskLayout->addWidget(editTaskButton);
        taskLayout->addWidget(deleteTaskButton);
        taskLayout->addWidget(completeTaskButton);
        taskRow->setLayout(taskLayout);

        taskListLayout->addWidget(taskRow);

        connect(editTaskButton, &QPushButton::clicked, this, [this, taskId, taskName, taskTag, taskDeadline]() {
            editTask(taskId, taskName, taskTag, taskDeadline);
        });


        connect(deleteTaskButton, &QPushButton::clicked, [this, taskId,taskRow]() {
            deleteTask(taskId);   // taskRow ではなく taskId を渡す
            taskRow->deleteLater();  // UIから削除
        });

        connect(completeTaskButton, &QPushButton::clicked, [this, taskId,taskRow]() {
            completeTask(taskId);   // taskRow ではなく taskId を渡す
        });

        // 新しいタスクを追加後、リストを更新
        updateTaskList();

        taskInput->clear();
        tagInput->clear();
    }



}

// **変更ボタンの処理**
void MainWindow::editTask(int taskId, QString taskName, QString taskTag, QString taskDeadline) {
    QDialog dialog(this);
    dialog.setWindowTitle("タスク編集");

    QVBoxLayout layout(&dialog);

    QLineEdit taskInputEdit;
    taskInputEdit.setText(taskName);
    QLineEdit tagInputEdit;
    tagInputEdit.setText(taskTag);
    QDateTimeEdit deadlineInputEdit;
    deadlineInputEdit.setDateTime(QDateTime::fromString(taskDeadline, Qt::ISODate));

    QPushButton saveButton("保存");
    QPushButton cancelButton("キャンセル");

    layout.addWidget(new QLabel("タスク名:"));
    layout.addWidget(&taskInputEdit);
    layout.addWidget(new QLabel("タグ:"));
    layout.addWidget(&tagInputEdit);
    layout.addWidget(new QLabel("期限:"));
    layout.addWidget(&deadlineInputEdit);
    layout.addWidget(&saveButton);
    layout.addWidget(&cancelButton);

    connect(&saveButton, &QPushButton::clicked, [&]() {
        QString newTaskName = taskInputEdit.text();
        QString newTag = tagInputEdit.text();
        QString newDeadline = deadlineInputEdit.dateTime().toString(Qt::ISODate);

        QSqlQuery query;
        query.prepare("UPDATE tasks SET taskText = ?, tagText = ?, deadline = ? WHERE id = ?");
        query.addBindValue(newTaskName);
        query.addBindValue(newTag);
        query.addBindValue(newDeadline);
        query.addBindValue(taskId);

        if (!query.exec()) {
            qDebug() << "タスク編集エラー:" << query.lastError().text();
            return;
        }

        qDebug() << "タスク編集成功: ID =" << taskId;
        updateTaskList();  // 編集後にリストを更新
        dialog.accept();
    });

    connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}



// **削除ボタンの処理**
void MainWindow::deleteTask(int taskId) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "削除確認", "このタスクを削除してもよろしいですか？",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM tasks WHERE id = ?");
        query.addBindValue(taskId);

        if (!query.exec()) {
            qDebug() << "タスク削除エラー:" << query.lastError().text();
            return;
        }

        qDebug() << "タスク削除成功: ID =" << taskId;
        updateTaskList();  // 削除後、リストを更新
    }
}

// **完了ボタンの処理**
void MainWindow::completeTask(int taskId) {
    QSqlQuery query;
    query.prepare("UPDATE tasks SET is_completed = 1 WHERE id = :id");
    query.bindValue(":id", taskId);

    if (!query.exec()) {
        qDebug() << "タスク完了の更新に失敗しました:" << query.lastError().text();
        return;
    }

    qDebug() << "タスクが完了しました: ID =" << taskId;

    updateTaskList();  // UI を更新
}


void MainWindow::checkReminders() {
    QDateTime now = QDateTime::currentDateTime();

    for (const auto &task : taskList) {
        if (task.deadline <= now.addSecs(60)) {  // 期限が1分以内
            QMessageBox::warning(this, "リマインダー", "タスク期限が近づいています: " + task.taskLabel->text());
        }
    }
}

void MainWindow::initializeDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tasks.db");  // データベースファイル名を指定

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", "Failed to open database: " + db.lastError().text());
        return;
    }

    // 🔽 ここで接続状態をチェックする 🔽
    if (!QSqlDatabase::database().isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database connection is not open.");
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS tasks ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "taskText TEXT, "
               "deadline TEXT, "
               "tagText TEXT)");

    qDebug() << "Database initialized successfully.";
}


void MainWindow::saveTaskToDatabase(const QString &taskText, const QDateTime &deadline, const QString &tagText) {
    if (!QSqlDatabase::database().isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database connection is not open.");
        qDebug() << "Database Error: Connection is not open!";
        return;
    }

    qDebug() << "Saving Task:" << taskText << deadline.toString("yyyy/MM/dd HH:mm") << tagText;

    QSqlQuery query;
    query.prepare("INSERT INTO tasks (taskText, deadline, tagText) VALUES (:taskText, :deadline, :tagText)");
    query.bindValue(":taskText", taskText);
    query.bindValue(":deadline", deadline.toString("yyyy/MM/dd HH:mm"));
    query.bindValue(":tagText", tagText);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to save task: " + query.lastError().text());
        qDebug() << "SQL Error:" << query.lastError().text();
    } else {
        qDebug() << "Task saved successfully!";
    }
}

void MainWindow::updateTaskList() {

    // 🔹 tagFilterComboBox の nullptr チェック
    if (!tagFilterComboBox) {
        qDebug() << "tagFilterComboBox が nullptr です";
        return;
    }

    QString selectedTag = tagFilterComboBox->currentText();

    QString queryStr = "SELECT * FROM tasks";
    QSqlQuery query;

    if (selectedTag != "すべてのタグ" && !selectedTag.isEmpty()) {
        queryStr += " WHERE tagText = :tag";
        query.prepare(queryStr);
        query.bindValue(":tag", selectedTag);
    } else {
        query.prepare(queryStr);
    }

    if (!query.exec()) {
        qDebug() << "クエリの実行に失敗しました:" << query.lastError().text();
        return;
    }

    // 🔹 クエリにデータがあるか確認
    if (!query.next()) {
        qDebug() << "タスクがデータベースに存在しません。";
        return;
    }
    query.previous();  // 最初のレコードに戻す

    // 🔹 過去のリストをクリア
    while (QLayoutItem *child = taskListLayout->takeAt(0)) {
        if (child->widget()) {
            qDebug() << "削除: " << child->widget()->objectName();  // 確認用ログ

            child->widget()->deleteLater();
        }
        delete child;
    }

    while (query.next()) {
        int taskId = query.value("id").toInt();
        QString taskName = query.value("taskText").toString();
        QString taskTag = query.value("tagText").toString();
        QDateTime taskDeadline = query.value("deadline").toDateTime();
        bool isCompleted = query.value("is_completed").toBool();
        QString deadlineStr = taskDeadline.toString("yyyy-MM-dd HH:mm");

        qDebug() << "取得したタスク: " << taskName << " (" << taskTag << "), 期限: " << deadlineStr;

        QLabel *taskLabel = new QLabel(taskName + " (" + taskTag + ") 期限: " + deadlineStr, this);
        QPushButton *editButton = new QPushButton("編集");
        QPushButton *deleteButton = new QPushButton("削除");
        QPushButton *completeButton = new QPushButton("完了", this);

        if (taskDeadline < QDateTime::currentDateTime()) {
            taskLabel->setStyleSheet("color: red; font-weight: bold;");
        }

        QWidget *taskRow = new QWidget(this);
        QHBoxLayout *taskLayout = new QHBoxLayout(taskRow);
        taskLayout->addWidget(taskLabel);
        taskLayout->addWidget(editButton);
        taskLayout->addWidget(deleteButton);
        taskLayout->addWidget(completeButton);
        taskRow->setLayout(taskLayout);

        taskListLayout->addWidget(taskRow);

        // 🔹 完了済みタスクの処理
        if (isCompleted) {
            taskLabel->setStyleSheet("color: gray; text-decoration: line-through;");
            completeButton->setEnabled(false);
        }

        // 🔹 削除ボタンの動作
        connect(deleteButton, &QPushButton::clicked, this, [this, taskId]() {
            deleteTask(taskId);
        });

        // 🔹 編集ボタンの動作
        connect(editButton, &QPushButton::clicked, this, [this, taskId, taskName, taskTag, deadlineStr]() {
            editTask(taskId, taskName, taskTag, deadlineStr);
        });

        // 🔹 完了ボタンの動作
        connect(completeButton, &QPushButton::clicked, this, [this, taskId]() {
            QSqlQuery query;
            query.prepare("UPDATE tasks SET is_completed = 1 WHERE id = :id");
            query.bindValue(":id", taskId);
            if (!query.exec()) {
                qDebug() << "タスクの完了に失敗しました:" << query.lastError().text();
            }
            updateTaskList();
        });
    }

    qDebug() << "updateTaskList() 完了";
}

void MainWindow::populateTagComboBox()
{
    QSqlQuery query;
    query.exec("SELECT DISTINCT tag FROM tasks");

    // 既存のタグをすべて追加
    while (query.next()) {
        QString tag = query.value(0).toString();
        tagFilterComboBox->addItem(tag);  // 取得したタグを追加
    }

    // 他の静的なタグ（例えば、予め決められたタグ）
    tagFilterComboBox->addItem("drink");
    tagFilterComboBox->addItem("fruit");
    tagFilterComboBox->addItem("snack");
}
void MainWindow::setupTaskTable()
{
    model = new QSqlTableModel(this);
    model->setTable("tasks");  // データベースの tasks テーブルを使用
    model->setSort(2, Qt::AscendingOrder);  // 2番目のカラム（deadline）で昇順ソート
    model->select();  // データを取得

    // QTableViewを作成
    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSortingEnabled(true);  // ヘッダークリックで並び替え可能

    // レイアウトに追加
    mainLayout->addWidget(tableView);
}

void MainWindow::sortTaskList(const QString &sortOption)
{
    // 並び替え基準を決定
    if (sortOption == "タスク名で並び替え") {
        model->setSort(1, Qt::AscendingOrder);  // taskText（1番目のカラム）
    } else if (sortOption == "締切日で並び替え") {
        model->setSort(2, Qt::AscendingOrder);  // deadline（2番目のカラム）
    } else if (sortOption == "タグで並び替え") {
        model->setSort(3, Qt::AscendingOrder);  // tagText（3番目のカラム）
    }

    model->select(); // データを取得し直す

    // **リストをクリア**
    while (QLayoutItem *child = taskListLayout->takeAt(0)) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    QSqlQuery query;
    query.exec("SELECT DISTINCT tag FROM tasks");

    // **並び替え後のデータを取得してリストを再描画**

    for (int row = 0; row < model->rowCount(); ++row) {
        int taskId = model->index(row, 0).data().toInt();  // id (0番目のカラム)
        QString taskName = model->index(row, 1).data().toString();  // taskText
        QString taskDeadline = model->index(row, 2).data().toString();  // deadline
        QString taskTag = model->index(row, 3).data().toString();  // tagText
        bool isCompleted = model->index(row, 4).data().toBool(); // 完了状態を取得

        QLabel *taskLabel = new QLabel(taskName + " (" + taskTag + " - " + taskDeadline + ")", this);


        // 完了しているタスクにはスタイルを適用
        if (isCompleted) {
            taskLabel->setStyleSheet("color: grey; text-decoration: line-through;");
        }
        // 締切日が過ぎている場合は赤色で太字に
        QDateTime deadlineDateTime = QDateTime::fromString(taskDeadline, "yyyy-MM-dd HH:mm");
        if (deadlineDateTime < QDateTime::currentDateTime()) {
            taskLabel->setStyleSheet("color: red; font-weight: bold;");
        }
        // 完了ボタンを作成
        QPushButton *completeButton = new QPushButton(isCompleted ? "未完了" : "完了", this);
        connect(completeButton, &QPushButton::clicked, this, [this, taskId, isCompleted]() {
            QSqlQuery query;
            query.prepare("UPDATE tasks SET is_completed = :isCompleted WHERE id = :id");
            query.bindValue(":id", taskId);
            query.bindValue(":isCompleted", !isCompleted);  // 完了状態を切り替え
            if (!query.exec()) {
                qDebug() << "タスクの完了に失敗しました:" << query.lastError().text();
            }
            updateTaskList();
        });

        // 削除ボタンを作成
        QPushButton *deleteButton = new QPushButton("削除", this);
        connect(deleteButton, &QPushButton::clicked, this, [this, taskId]() {
            deleteTask(taskId);
        });

        // 変更ボタンを作成
        QPushButton *editButton = new QPushButton("変更", this);
        connect(editButton, &QPushButton::clicked, this, [this, taskId, taskName, taskTag, taskDeadline]() {
            editTask(taskId, taskName, taskTag, taskDeadline);
        });

        // タスクラベルとボタンをレイアウトに追加
        QHBoxLayout *taskLayout = new QHBoxLayout();
        taskLayout->addWidget(taskLabel);
        taskLayout->addWidget(completeButton);
        taskLayout->addWidget(deleteButton);
        taskLayout->addWidget(editButton);
        QWidget *taskWidget = new QWidget(this);
        taskWidget->setLayout(taskLayout);
        taskListLayout->addWidget(taskWidget);  // 新しい順序でリストを作成

    }

    // QTableViewが表示されていないかを確認
    if (tableView) {
        tableView->setVisible(false);  // もしQTableViewが表示されている場合、非表示にする
    }
}




void MainWindow::loadTasksFromDatabase() {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return;
    }

    QSqlQuery query;
    if (!query.exec("SELECT id, taskText, tagText, deadline FROM tasks")) {
        qDebug() << "Failed to load tasks:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString taskText = query.value(1).toString();
        QString tagText = query.value(2).toString();
        QDateTime deadline = query.value(3).toDateTime();


        QWidget *taskRow = new QWidget(this);
        QHBoxLayout *taskLayout = new QHBoxLayout(taskRow);

        QLabel *taskLabel = new QLabel(taskText + " (期限: " + deadline.toString("yyyy/MM/dd HH:mm") + ")", this);
        QLabel *tagLabel = new QLabel(tagText.isEmpty() ? "なし" : tagText, this);

        tagLabel->setStyleSheet("color: #888; font-size: 12px;");

        QPushButton *editTaskButton = new QPushButton("変更", this);
        QPushButton *deleteTaskButton = new QPushButton("削除", this);

        taskLayout->addWidget(taskLabel);
        taskLayout->addWidget(tagLabel);
        taskLayout->addWidget(editTaskButton);
        taskLayout->addWidget(deleteTaskButton);
        taskRow->setLayout(taskLayout);

        taskListLayout->addWidget(taskRow);
        taskList.append(Task(taskLabel, deadline, tagText));
        taskRow->update();
        update();  // 🔴 UIの更新を試す

        // 編集・削除ボタンの処理
        QString deadlineStr = deadline.toString("yyyy-MM-dd HH:mm:ss");  // QDateTime → QString に変換

        connect(editTaskButton, &QPushButton::clicked, this, [this,id, taskText, tagText, deadlineStr]() {
            editTask(id, taskText, tagText, deadlineStr);
        });

        connect(deleteTaskButton, &QPushButton::clicked, this, [this, id]() {
            deleteTask(id);
        });
    }



    query.finish();
    qDebug() << "Finished loading tasks.";
}
