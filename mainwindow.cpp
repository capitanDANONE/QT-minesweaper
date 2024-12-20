#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTimerEvent>

int **mineCountGrid;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    elapsedTime(0),
    mineField(nullptr),
    flaggedCells(nullptr)
{
    ui->setupUi(this);

    connect(ui->newGameButton, &QPushButton::clicked, this, &MainWindow::startNewGame);
    connect(ui->gameBoard, &QTableWidget::cellClicked, this, &MainWindow::handleCellClick);
    connect(ui->saveGameButton, &QPushButton::clicked, this, &MainWindow::saveGameState);
    connect(ui->loadGameButton, &QPushButton::clicked, this, &MainWindow::loadGameState);
    connect(ui->showRatingsButton, &QPushButton::clicked, this, &MainWindow::showRatings);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);

    loadRatings();
}

MainWindow::~MainWindow()
{
    delete ui;
    for (int i = 0; i < rows; ++i) {
        delete[] mineField[i];
        delete[] flaggedCells[i];
        delete[] mineCountGrid[i];
    }
    delete[] mineField;
    delete[] flaggedCells;
    delete[] mineCountGrid;
}

void MainWindow::startNewGame()
{
    QString level = ui->levelComboBox->currentText();
    if (level == "Лёгкий") {
        rows = 8;
        cols = 8;
        mineCount = 10;
    } else if (level == "Средний") {
        rows = 16;
        cols = 16;
        mineCount = 40;
    } else {
        rows = 30;
        cols = 16;
        mineCount = 99;
    }

    initializeGame();
    generateMines();
    updateGameBoard();
    elapsedTime = 0;
    timer->start(1000);
}

void MainWindow::initializeGame()
{
    if (mineField) {
        for (int i = 0; i < rows; ++i) {
            delete[] mineField[i];
            delete[] flaggedCells[i];
            delete[] mineCountGrid[i];
        }
        delete[] mineField;
        delete[] flaggedCells;
        delete[] mineCountGrid;
    }

    mineField = new bool*[rows];
    flaggedCells = new bool*[rows];
    mineCountGrid = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        mineField[i] = new bool[cols]();
        flaggedCells[i] = new bool[cols]();
        mineCountGrid[i] = new int[cols]();
    }

    ui->gameBoard->setRowCount(rows);
    ui->gameBoard->setColumnCount(cols);
}

void MainWindow::generateMines()
{
    int minesPlaced = 0;
    while (minesPlaced < mineCount) {
        int r = rand() % rows;
        int c = rand() % cols;
        if (!mineField[r][c]) {
            mineField[r][c] = true;
            ++minesPlaced;

            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int adjRow = r + i;
                    int adjCol = c + j;
                    if (adjRow >= 0 && adjRow < rows && adjCol >= 0 && adjCol < cols) {
                        if (!mineField[adjRow][adjCol]) {
                            ++mineCountGrid[adjRow][adjCol];
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::updateGameBoard()
{
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem();

            if (flaggedCells[row][col]) {
                if (mineField[row][col]) {
                    item->setText("💣");
                } else {
                    int count = mineCountGrid[row][col];
                    item->setText(count > 0 ? QString::number(count) : "");
                }
            } else {
                item->setText("*");
            }

            ui->gameBoard->setItem(row, col, item);
        }
    }

    int cellSize = 25;
    for (int i = 0; i < rows; ++i) {
        ui->gameBoard->setRowHeight(i, cellSize);
    }
    for (int j = 0; j < cols; ++j) {
        ui->gameBoard->setColumnWidth(j, cellSize);
    }
}

void MainWindow::handleCellClick(int row, int col)
{
    if (flaggedCells[row][col]) return;

    if (mineField[row][col]) {
        gameOver(false);
    } else {
        revealCell(row, col);

        if (mineCountGrid[row][col] == 0) {
            revealAdjacentCells(row, col);
        }

        if (checkWinCondition()) {
            gameOver(true);
        }
    }
}

void MainWindow::revealCell(int row, int col)
{
    if (flaggedCells[row][col] || mineField[row][col]) return;
    flaggedCells[row][col] = true;

    updateGameBoard();
}

void MainWindow::revealAdjacentCells(int row, int col)
{
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int adjRow = row + i;
            int adjCol = col + j;
            if (adjRow >= 0 && adjRow < rows && adjCol >= 0 && adjCol < cols) {
                if (!flaggedCells[adjRow][adjCol]) {
                    revealCell(adjRow, adjCol);

                    if (mineCountGrid[adjRow][adjCol] == 0) {
                        revealAdjacentCells(adjRow, adjCol);
                    }
                }
            }
        }
    }
}

bool MainWindow::checkWinCondition()
{
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (!mineField[row][col] && !flaggedCells[row][col]) {
                return false;
            }
        }
    }
    return true;
}

void MainWindow::gameOver(bool win)
{
    timer->stop();
    QString message = win ? "Вы выиграли!" : "Вы проиграли!";
    QMessageBox::information(this, "Игра окончена", message);

    if (win) {
        saveRating("Player", elapsedTime);
    }

    startNewGame();
}

void MainWindow::updateTime()
{
    ++elapsedTime;
    ui->timeLabel->setText(QString("Время: %1").arg(elapsedTime));
}

void MainWindow::saveRating(const QString &name, int time)
{
    QFile file("ratings.txt");
    if (file.open(QIODevice::Append)) {
        QTextStream out(&file);
        out << name << " " << time << "\n";
        file.close();
    }
}

void MainWindow::loadRatings()
{
    QFile file("ratings.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            Rating rating;
            in >> rating.name >> rating.time;
            ratings.append(rating);
        }
        file.close();
    }
}

void MainWindow::showRatings()
{
    QString ratingText = "Рейтинги:\n";
    for (const Rating &rating : ratings) {
        ratingText += QString("%1 - %2 секунд\n").arg(rating.name).arg(rating.time);
    }
    QMessageBox::information(this, "Рейтинги", ratingText);
}

void MainWindow::saveGameState()
{
    QFile file("savegame.dat");
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << rows << cols << mineCount << elapsedTime;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                out << mineField[i][j] << flaggedCells[i][j] << mineCountGrid[i][j];
            }
        }

        file.close();
    }
}

void MainWindow::loadGameState()
{
    QFile file("savegame.dat");
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in >> rows >> cols >> mineCount >> elapsedTime;

        initializeGame();

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                in >> mineField[i][j] >> flaggedCells[i][j] >> mineCountGrid[i][j];
            }
        }

        updateGameBoard();
        timer->start(1000);

        file.close();
    }
}
