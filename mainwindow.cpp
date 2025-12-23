#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QSslSocket>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);

    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onNetworkReply);

    qDebug() << QSqlDatabase::drivers();
    if (initDatabase()) {
        loadBookmarksFromDb();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Add this method to initialize the database
bool MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("manga_bookmarks.db");

    if (!db.open()) {
        QMessageBox::critical(this,
                              "Database Error",
                              "Failed to open database: " + db.lastError().text());
        return false;
    }

    // Create bookmarks table if it doesn't exist
    QSqlQuery query;
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS bookmarks (
            manga_id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            chapter REAL NOT NULL
        )
    )";

    if (!query.exec(createTable)) {
        QMessageBox::critical(this,
                              "Database Error",
                              "Failed to create table: " + query.lastError().text());
        return false;
    }

    return true;
}

// Save bookmark to database
bool MainWindow::saveBookmarkToDb(const Bookmark &bookmark)
{
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO bookmarks (manga_id, title, chapter) "
                  "VALUES (:manga_id, :title, :chapter)");
    query.bindValue(":manga_id", bookmark.mangaId);
    query.bindValue(":title", bookmark.title);
    query.bindValue(":chapter", bookmark.chapter);

    if (!query.exec()) {
        QMessageBox::critical(this,
                              "Database Error",
                              "Failed to save bookmark: " + query.lastError().text());
        return false;
    }

    return true;
}

// Load all bookmarks from database
bool MainWindow::loadBookmarksFromDb()
{
    bookmarks.clear();
    ui->listWidgetBookmarks->clear();

    QSqlQuery query("SELECT manga_id, title, chapter FROM bookmarks");

    while (query.next()) {
        Bookmark bm;
        bm.mangaId = query.value(0).toString();
        bm.title = query.value(1).toString();
        bm.chapter = query.value(2).toDouble();

        bookmarks[bm.mangaId] = bm;

        QListWidgetItem *item = new QListWidgetItem(bm.title + " (Ch. "
                                                    + QString::number(bm.chapter) + ")");
        item->setData(Qt::UserRole, bm.mangaId);
        item->setData(Qt::UserRole + 1, bm.title);
        ui->listWidgetBookmarks->addItem(item);
    }

    if (query.lastError().isValid()) {
        QMessageBox::critical(this,
                              "Database Error",
                              "Failed to load bookmarks: " + query.lastError().text());
        return false;
    }

    return true;
}

bool MainWindow::deleteBookmarkFromDb(const QString &mangaId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM bookmarks WHERE manga_id = :manga_id");
    query.bindValue(":manga_id", mangaId);

    if (!query.exec()) {
        QMessageBox::critical(this,
                              "Database Error",
                              "Failed to delete bookmark: " + query.lastError().text());
        return false;
    }

    return true;
}

void MainWindow::on_pushButtonSearch_clicked()
{
    // Get the search text from the line edit
    QString searchText = ui->lineEditSearch->text().trimmed();

    // Check if search text is empty
    if (searchText.isEmpty()) {
        QMessageBox::warning(this, "Search Error", "Please enter a manga title to search.");
        return;
    }

    // URL encode the search text to handle special characters and spaces
    QString encodedText = QUrl::toPercentEncoding(searchText);

    // Construct the URL with the search parameter
    QUrl url(QString("https://api.mangadex.org/manga?title=%1").arg(encodedText));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    qDebug() << url;

    // Send GET request
    QNetworkReply *reply = networkManager->get(request);

    reply->setProperty("requestType", MangaSearch);
}

void MainWindow::populateMangaList(const QJsonObject& jsonObj)
{
    ui->listWidgetManga->clear();
    QJsonArray dataArray = jsonObj["data"].toArray();

    for (const QJsonValue& value : dataArray) {
        QJsonObject manga = value.toObject();
        QJsonObject attributes = manga["attributes"].toObject();
        QJsonObject titleObj = attributes["title"].toObject();

        // Try to get title in order of preference: en -> ja-ro -> ja -> any other
        QString title;

        if (titleObj.contains("en") && !titleObj["en"].toString().isEmpty()) {
            title = titleObj["en"].toString();
        } else if (titleObj.contains("ja-ro") && !titleObj["ja-ro"].toString().isEmpty()) {
            title = titleObj["ja-ro"].toString();
        } else if (titleObj.contains("ja") && !titleObj["ja"].toString().isEmpty()) {
            title = titleObj["ja"].toString();
        } else {
            // Get the first available title
            QStringList keys = titleObj.keys();
            if (!keys.isEmpty()) {
                title = titleObj[keys.first()].toString();
            } else {
                title = "Unknown Title";
            }
        }

        // Extract other data
        QString mangaId = manga["id"].toString();
        QString year = QString::number(attributes["year"].toInt());
        QString status = attributes["status"].toString();

        // Create list item with title
        QListWidgetItem *item = new QListWidgetItem(title);

        // Store the ID and other data
        item->setData(Qt::UserRole, mangaId);
        item->setData(Qt::UserRole + 1, year);
        item->setData(Qt::UserRole + 2, status);

        // Optional: Add tooltip
        item->setToolTip(QString("Year: %1\nStatus: %2\nID: %3")
                             .arg(year, status, mangaId));

        ui->listWidgetManga->addItem(item);
    }
}

void MainWindow::populateChapterList(const QJsonObject& jsonObj)
{
    ui->listWidgetChapter->clear();
    qDebug() << "\n========== CHAPTER LIST ==========";

    QJsonArray dataArray = jsonObj["data"].toArray();

    qDebug() << "Total chapters:" << dataArray.size();
    qDebug() << "";

    maxChapterNum = -1.0;
    QString maxChapterStr;

    for (int i = 0; i < dataArray.size(); ++i) {
        QJsonObject chapter = dataArray[i].toObject();
        QJsonObject attributes = chapter["attributes"].toObject();

        QString chapterId = chapter["id"].toString();
        QString chapterNum = attributes["chapter"].toString();
        QString title = attributes["title"].toString();
        QString volume = attributes["volume"].toString();
        int pages = attributes["pages"].toInt();
        QString language = attributes["translatedLanguage"].toString();

        // Track the largest chapter number
        if (!chapterNum.isEmpty()) {
            bool ok;
            double numValue = chapterNum.toDouble(&ok);
            if (ok && numValue > maxChapterNum) {
                maxChapterNum = numValue;
                maxChapterStr = chapterNum;
            }
        }

        // Create display text for the list item
        QString displayText = QString("Ch. %1%2 (%3 pages) [%4]")
                                  .arg(chapterNum.isEmpty() ? "?" : chapterNum)
                                  .arg(title.isEmpty() ? "" : " - " + title)
                                  .arg(pages)
                                  .arg(language);

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, chapterId);
        item->setData(Qt::UserRole + 1, chapterNum);
        item->setData(Qt::UserRole + 2, language);

        ui->listWidgetChapter->addItem(item);

        qDebug() << "Chapter" << chapterNum << ":";
        qDebug() << "  ID:" << chapterId;
        qDebug() << "  Title:" << (title.isEmpty() ? "(No title)" : title);
        qDebug() << "  Volume:" << (volume.isEmpty() ? "N/A" : volume);
        qDebug() << "  Pages:" << pages;
        qDebug() << "  Language:" << language;
        qDebug() << "---";
    }

    if (maxChapterNum >= 0) {
        qDebug() << "Largest chapter number:" << maxChapterStr << "(" << maxChapterNum << ")";
    } else {
        qDebug() << "No valid chapter numbers found";
    }

    qDebug() << "==================================\n";

    // Update label if this was loaded from a bookmark
    if (loadingFromBookmark) {
        loadingFromBookmark = false; // Reset flag

        if (bookmarks.contains(selected.mangaId)) {
            Bookmark currentBm = bookmarks[selected.mangaId];

            if (currentBm.chapter < maxChapterNum) {
                ui->labelStatus->setText(
                    QString("Manga: %1, Last read: Ch. %2, New Chapter: Yes (Latest: Ch. %3)")
                        .arg(currentBm.title)
                        .arg(currentBm.chapter)
                        .arg(maxChapterNum)
                    );
            } else {
                ui->labelStatus->setText(
                    QString("Manga: %1, Last read: Ch. %2, New Chapter: No")
                        .arg(currentBm.title)
                        .arg(currentBm.chapter)
                    );
            }
        }
    }
}

void MainWindow::onNetworkReply(QNetworkReply *reply)
{
    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(this, "Network Error",
                              QString("Error: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    // Get request type from reply property FIRST (before reading data)
    RequestType requestType = static_cast<RequestType>(reply->property("requestType").toInt());

    // Read response data
    QByteArray responseData = reply->readAll();

    // Handle cover image separately (it's binary data, not JSON)
    if (requestType == CoverImage) {
        QString mangaId = reply->property("mangaId").toString();

        // Parse JSON to get filename
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            QJsonObject data = obj["data"].toObject();
            QJsonObject attributes = data["attributes"].toObject();
            QString filename = attributes["fileName"].toString();

            qDebug() << "Cover filename:" << filename;

            // Now fetch the actual image
            QString imageUrl = QString("https://uploads.mangadex.org/covers/%1/%2")
                                   .arg(mangaId, filename);

            qDebug() << "Fetching cover image from:" << imageUrl;

            QNetworkRequest imageRequest{QUrl(imageUrl)};
            QNetworkReply *imageReply = networkManager->get(imageRequest);
            imageReply->setProperty("requestType", 999); // Special marker for actual image
        }

        reply->deleteLater();
        return;
    }

    // Handle actual image download
    if (reply->property("requestType").toInt() == 999) {
        displayCoverImage(responseData);
        reply->deleteLater();
        return;
    }

    // Parse JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, "JSON Parse Error",
                              QString("Parse error at %1: %2")
                                  .arg(parseError.offset)
                                  .arg(parseError.errorString()));
        reply->deleteLater();
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        //qDebug() << "test:" << obj;


        // Get request type from reply property
        RequestType requestType = static_cast<RequestType>(
            reply->property("requestType").toInt()
            );

        switch (requestType) {
        case MangaSearch:
            qDebug() << "Received manga search results";
            populateMangaList(obj);
            break;
        case ChapterFeed:
            qDebug() << "Received chapter feed";
            populateChapterList(obj);
            break;
        case MangaDetails: {
            qDebug() << "Received manga details for cover";

            // Extract cover art ID from relationships
            QJsonObject data = obj["data"].toObject();
            QJsonArray relationships = data["relationships"].toArray();

            QString coverId;

            for (const QJsonValue &rel : relationships) {
                QJsonObject relObj = rel.toObject();
                if (relObj["type"].toString() == "cover_art") {
                    coverId = relObj["id"].toString();
                    break;
                }
            }
            if (!coverId.isEmpty()) {
                QString mangaId = reply->property("mangaId").toString();
                qDebug() << "Found cover ID:" << coverId;
                fetchCoverImage(mangaId, coverId);
            } else {
                qDebug() << "No cover art found for this manga";
                ui->labelCover->setText("No cover available");
            }
            break;
        }
        default:
            qDebug() << "Unknown request type";
            break;
        }
    }
    reply->deleteLater(); // Clean up
}

void MainWindow::on_listWidgetManga_itemPressed(QListWidgetItem *item)
{

    if (!item) return;

    // Retrieve the hidden data
    QString mangaId = item->data(Qt::UserRole).toString();
    QString year = item->data(Qt::UserRole + 1).toString();
    QString status = item->data(Qt::UserRole + 2).toString();
    QString title = item->text();

    // qDebug() << "Selected Manga:";
    // qDebug() << "  Title:" << title;
    // qDebug() << "  ID:" << mangaId;
    // qDebug() << "  Year:" << year;
    // qDebug() << "  Status:" << status;

    // Make request to get chapters
    QString url = QString("https://api.mangadex.org/manga/%1/feed?limit=100&translatedLanguage[]=en&order[chapter]=asc")
                      .arg(mangaId);

    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->get(request);

    // Set property to identify this request as ChapterFeed
    reply->setProperty("requestType", ChapterFeed);
    reply->setProperty("mangaId", mangaId);
    reply->setProperty("mangaTitle", title);

    selected.title = title;
    selected.mangaId = mangaId;
    selected.chapter = -1;

    fetchMangaCover(mangaId);
}

void MainWindow::on_listWidgetChapter_itemPressed(QListWidgetItem *item)
{
    if (!item) return;

    // Retrieve the hidden data
    QString chapterId = item->data(Qt::UserRole).toString();
    int chapterNum = item->data(Qt::UserRole + 1).toInt();
    QString language = item->data(Qt::UserRole + 2).toString();
    selected.chapter = chapterNum;



//     qDebug() << "chapterId" << chapterId;
//     qDebug() << "chapterNum" << chapterNum;
//     selectedChapter = chapterNum;
//     qDebug() << "selectedChapter Var:" << selectedChapter;
//     qDebug() << "lan" << language;
}


void MainWindow::on_pushButtonLastRead_clicked()
{
    bookmarks[bm.mangaId] = bm; // add OR update
    if (selected.chapter != -1) {
        bm.mangaId = selected.mangaId;
        bm.title = selected.title;
        bm.chapter = selected.chapter;
        // Save to database
        if (saveBookmarkToDb(bm)) {
            qDebug() << "Saved bookmark to database:";
            qDebug() << bm.title << "chapter" << bm.chapter;
        }
    } else {
        QMessageBox::information(this, "warning", "pilih chapter");
    }

    // Refresh the bookmark list from database
    loadBookmarksFromDb();
}

void MainWindow::on_listWidgetBookmarks_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    // Retrieve the hidden data
    QString mangaId = item->data(Qt::UserRole).toString();
    QString title = item->data(Qt::UserRole + 1).toString();

    // Set flag to indicate we're loading from bookmark
    loadingFromBookmark = true;

    // Make request to get chapters
    QString url = QString("https://api.mangadex.org/manga/%1/feed?limit=100&translatedLanguage[]=en&order[chapter]=asc")
                      .arg(mangaId);

    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->get(request);

    // Set property to identify this request as ChapterFeed
    reply->setProperty("requestType", ChapterFeed);
    reply->setProperty("mangaId", mangaId);
    reply->setProperty("mangaTitle", title);

    selected.title = title;
    selected.mangaId = mangaId;
    selected.chapter = -1;

    fetchMangaCover(mangaId);
}

void MainWindow::on_pushButtonDelete_clicked()
{
    QListWidgetItem *item = ui->listWidgetBookmarks->currentItem();
    if (!item) {
        QMessageBox::warning(this, "No Selection", "Please select a bookmark to delete.");
        return;
    }

    QString mangaId = item->data(Qt::UserRole).toString();
    QString title = item->data(Qt::UserRole + 1).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Delete Bookmark",
                                  "Delete bookmark for " + title + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (deleteBookmarkFromDb(mangaId)) {
            bookmarks.remove(mangaId);
            loadBookmarksFromDb();
            qDebug() << "Deleted bookmark:" << title;
        }
    }
}

void MainWindow::fetchMangaCover(const QString &mangaId)
{
    // First, get manga details to find the cover art ID
    QString url = QString("https://api.mangadex.org/manga/%1?includes[]=cover_art").arg(mangaId);

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->get(request);
    reply->setProperty("requestType", MangaDetails);
    reply->setProperty("mangaId", mangaId);

    qDebug() << "Fetching manga details for cover from:" << url;
}

void MainWindow::fetchCoverImage(const QString &mangaId, const QString &coverId)
{
    // Get cover details to find filename
    QString url = QString("https://api.mangadex.org/cover/%1").arg(coverId);

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->get(request);
    reply->setProperty("requestType", CoverImage);
    reply->setProperty("mangaId", mangaId);

    qDebug() << "Fetching cover details from:" << url;
}

void MainWindow::displayCoverImage(const QByteArray &imageData)
{
    QPixmap pixmap;
    if (pixmap.loadFromData(imageData)) {
        // Scale the image to fit the label while maintaining aspect ratio
        QPixmap scaledPixmap = pixmap.scaled(ui->labelCover->size(),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
        ui->labelCover->setPixmap(scaledPixmap);
        qDebug() << "Cover image displayed successfully";
    } else {
        qDebug() << "Failed to load cover image";
        ui->labelCover->setText("Failed to load cover");
    }
}
