#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>    // for QTimer
#include <QUrlQuery> // for QUrlQuery
#include <qjsonarray.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    double maxChapterNum;

    struct Bookmark {
        QString mangaId;
        QString title;
        double chapter;
    };

    Bookmark selected;

    QMap<QString, Bookmark> bookmarks;

    Bookmark bm;



private slots:
    void on_pushButtonSearch_clicked();
    void onNetworkReply(QNetworkReply *reply);

    void on_listWidgetManga_itemPressed(QListWidgetItem *item);

    void on_listWidgetChapter_itemPressed(QListWidgetItem *item);

    void on_pushButtonLastRead_clicked();

    void on_listWidgetBookmarks_itemClicked(QListWidgetItem *item);

    void on_pushButtonDelete_clicked();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;

    void populateMangaList(const QJsonObject& jsonObj);
    void populateChapterList(const QJsonObject& jsonObj);

    QLabel *coverLabel;

    void fetchMangaCover(const QString &mangaId);
    void fetchCoverImage(const QString &mangaId, const QString &coverId);
    void displayCoverImage(const QByteArray &imageData);

    enum RequestType { MangaSearch, ChapterFeed, MangaDetails, CoverImage };

    bool loadingFromBookmark = false;

    QSqlDatabase db;

    bool initDatabase();
    bool saveBookmarkToDb(const Bookmark &bookmark);
    bool loadBookmarksFromDb();
    bool deleteBookmarkFromDb(const QString &mangaId);
};
#endif
