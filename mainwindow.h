#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QList>

namespace Ui {
class MainWindow;
}

struct Rating {
    QString name;
    int time;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startNewGame();
    void updateTime();
    void handleCellClick(int row, int column);
    void saveGameState();
    void loadGameState();
    void showRatings();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    bool **mineField;
    int **mineCountGrid;
    bool **flaggedCells;
    int rows, cols, mineCount;
    int elapsedTime;
    QList<Rating> ratings;

    void initializeGame();
    void generateMines();
    void updateGameBoard();
    void revealCell(int row, int col);
    void revealAdjacentCells(int row, int col);
    bool checkWinCondition();
    void gameOver(bool win);
    void saveRating(const QString &name, int time);
    void loadRatings();
};

#endif // MAINWINDOW_H
