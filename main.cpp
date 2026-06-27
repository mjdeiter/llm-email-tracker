#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QList>
#include <QStringList>
#include <QIcon>

class EmailRow : public QWidget {
    Q_OBJECT
public:
    QLineEdit* emailEdit;
    QRadioButton* usedButton;
    QLabel* timestampLabel;
    QLineEdit* manualDateTimeEdit;
    QString rawTimestamp;

    EmailRow(QWidget* parent = nullptr) : QWidget(parent) {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 4, 0, 4);
        layout->setSpacing(10);

        emailEdit = new QLineEdit(this);
        emailEdit->setPlaceholderText("Enter email address...");
        emailEdit->setFixedWidth(300);

        usedButton = new QRadioButton("Used", this);
        usedButton->setAutoExclusive(false);
        usedButton->setFixedWidth(80);

        timestampLabel = new QLabel(this);
        timestampLabel->setFixedWidth(200);
        timestampLabel->setFont(QFont("monospace", 10));

        manualDateTimeEdit = new QLineEdit(this);
        manualDateTimeEdit->setFixedWidth(180);
        manualDateTimeEdit->setStyleSheet(
            "QLineEdit {"
            "  color: #9c36b5;"
            "  font-family: monospace;"
            "  border: 1px solid #ced4da;"
            "  border-radius: 4px;"
            "  padding: 2px;"
            "}"
            "QLineEdit:focus {"
            "  border: 1px solid #9c36b5;"
            "}"
        );

        layout->addWidget(emailEdit);
        layout->addWidget(usedButton);
        layout->addWidget(timestampLabel);
        layout->addWidget(manualDateTimeEdit);
        layout->addStretch();

        connect(usedButton, &QRadioButton::toggled, this, &EmailRow::handleToggled);
    }

    void displayTimestamp() {
        if (rawTimestamp.isEmpty()) {
            timestampLabel->clear();
            return;
        }

        QStringList parts = rawTimestamp.split(" ");
        if (parts.size() == 2) {
            QString htmlText = QString("<span style='color: #d9480f; font-weight: bold;'>%1</span> "
            "<span style='color: #2b8a3e;'>%2</span>").arg(parts[0], parts[1]);
            timestampLabel->setText(htmlText);
        } else {
            timestampLabel->setText(rawTimestamp);
        }
    }

    // Resets Status radio, Auto Timestamp, and Resets field for this row
    void resetRow() {
        usedButton->blockSignals(true);
        usedButton->setChecked(false);
        usedButton->blockSignals(false);

        rawTimestamp.clear();
        displayTimestamp();

        manualDateTimeEdit->clear();
    }

private slots:
    void handleToggled(bool checked) {
        if (checked) {
            QString dateStr = QDateTime::currentDateTime().toString("MM/dd/yyyy");
            QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");
            rawTimestamp = dateStr + " " + timeStr;
        } else {
            rawTimestamp.clear();
        }
        displayTimestamp();
    }
};

class MainWidget : public QWidget {
    Q_OBJECT
private:
    QVBoxLayout* rowsLayout;
    QWidget* scrollContainer;
    QList<EmailRow*> rows;
    QString savePath;

public:
    MainWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setWindowTitle("Email Status Tracker");
        resize(850, 500);

        savePath = QDir::homePath() + "/.config/email_tracker_data.json";

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QHBoxLayout* headerLayout = new QHBoxLayout();
        headerLayout->setContentsMargins(10, 5, 10, 5);
        headerLayout->setSpacing(10);

        QString headerStyle = "font-weight: bold; color: #ffffff; font-size: 12px;";

        QLabel* emailHeader = new QLabel("Email Address", this);
        emailHeader->setFixedWidth(300);
        emailHeader->setStyleSheet(headerStyle);

        QLabel* statusHeader = new QLabel("Status", this);
        statusHeader->setFixedWidth(80);
        statusHeader->setStyleSheet(headerStyle);

        QLabel* timeHeader = new QLabel("Auto Timestamp (Date / Time)", this);
        timeHeader->setFixedWidth(200);
        timeHeader->setStyleSheet(headerStyle);

        QLabel* resetsHeader = new QLabel("Resets", this);
        resetsHeader->setFixedWidth(180);
        resetsHeader->setStyleSheet(headerStyle);

        headerLayout->addWidget(emailHeader);
        headerLayout->addWidget(statusHeader);
        headerLayout->addWidget(timeHeader);
        headerLayout->addWidget(resetsHeader);
        headerLayout->addStretch();

        mainLayout->addLayout(headerLayout);

        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);

        scrollContainer = new QWidget();
        rowsLayout = new QVBoxLayout(scrollContainer);
        rowsLayout->setAlignment(Qt::AlignTop);
        rowsLayout->setContentsMargins(10, 0, 10, 0);
        scrollContainer->setLayout(rowsLayout);
        scrollArea->setWidget(scrollContainer);

        mainLayout->addWidget(scrollArea);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* addButton = new QPushButton("Add Row", this);
        QPushButton* saveButton = new QPushButton("Save Config", this);
        saveButton->setStyleSheet("font-weight: bold;");

        QPushButton* resetAllButton = new QPushButton("Reset All", this);
        resetAllButton->setStyleSheet(
            "QPushButton {"
            "  color: #c92a2a;"
            "  font-weight: bold;"
            "  border: 1px solid #c92a2a;"
            "  border-radius: 4px;"
            "  padding: 4px 12px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #c92a2a;"
            "  color: #ffffff;"
            "}"
            "QPushButton:pressed {"
            "  background-color: #a61e1e;"
            "  color: #ffffff;"
            "}"
        );

        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(saveButton);
        buttonLayout->addStretch();
        buttonLayout->addWidget(resetAllButton);
        mainLayout->addLayout(buttonLayout);

        connect(addButton, &QPushButton::clicked, this, &MainWidget::addNewRow);
        connect(saveButton, &QPushButton::clicked, this, &MainWidget::saveData);
        connect(resetAllButton, &QPushButton::clicked, this, &MainWidget::resetAll);

        loadData();
    }

    void addNewRow() {
        EmailRow* row = new EmailRow(scrollContainer);
        rowsLayout->addWidget(row);
        rows.append(row);
    }

    void saveData() {
        QJsonArray jsonArray;
        for (EmailRow* row : rows) {
            if (row->emailEdit->text().isEmpty() &&
                !row->usedButton->isChecked() &&
                row->manualDateTimeEdit->text().isEmpty()) {
                continue;
                }
                QJsonObject rowObject;
            rowObject["email"] = row->emailEdit->text();
            rowObject["used"] = row->usedButton->isChecked();
            rowObject["timestamp"] = row->rawTimestamp;
            rowObject["resets"] = row->manualDateTimeEdit->text();
            jsonArray.append(rowObject);
        }

        QJsonDocument doc(jsonArray);
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
        }
    }

    void loadData() {
        QFile file(savePath);
        if (!file.open(QIODevice::ReadOnly)) {
            addNewRow();
            return;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray jsonArray = doc.array();

        if (jsonArray.isEmpty()) {
            addNewRow();
            return;
        }

        for (int i = 0; i < jsonArray.size(); ++i) {
            QJsonObject rowObject = jsonArray[i].toObject();
            EmailRow* row = new EmailRow(scrollContainer);

            row->emailEdit->setText(rowObject["email"].toString());

            row->usedButton->blockSignals(true);
            row->usedButton->setChecked(rowObject["used"].toBool());
            row->usedButton->blockSignals(false);

            row->rawTimestamp = rowObject["timestamp"].toString();
            row->displayTimestamp();

            row->manualDateTimeEdit->setText(rowObject["resets"].toString());

            rowsLayout->addWidget(row);
            rows.append(row);
        }
    }

public slots:
    void resetAll() {
        for (EmailRow* row : rows) {
            row->resetRow();
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWidget window;

    // --- Embedded Custom Colored Icon ---
    QString iconPath = QDir::homePath() + "/.config/email_tracker_icon.svg";
    if (!QFile::exists(iconPath)) {
        QFile iconFile(iconPath);
        if (iconFile.open(QIODevice::WriteOnly)) {
            // Standard escaped string literal to avoid parsing errors
            QString svgContent =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 64 64\">\n"
            "  <rect x=\"4\" y=\"4\" width=\"56\" height=\"56\" rx=\"12\" fill=\"#1e1e2f\"/>\n"
            "  <rect x=\"4\" y=\"4\" width=\"56\" height=\"56\" rx=\"12\" fill=\"url(#grad)\" opacity=\"0.9\"/>\n"
            "  <defs>\n"
            "    <linearGradient id=\"grad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">\n"
            "      <stop offset=\"0%\" stop-color=\"#3b82f6\" />\n"
            "      <stop offset=\"100%\" stop-color=\"#8b5cf6\" />\n"
            "    </linearGradient>\n"
            "  </defs>\n"
            "  <rect x=\"24\" y=\"18\" width=\"28\" height=\"4\" rx=\"2\" fill=\"#ffffff\" opacity=\"0.9\"/>\n"
            "  <rect x=\"24\" y=\"30\" width=\"24\" height=\"4\" rx=\"2\" fill=\"#ffffff\" opacity=\"0.9\"/>\n"
            "  <rect x=\"24\" y=\"42\" width=\"20\" height=\"4\" rx=\"2\" fill=\"#ffffff\" opacity=\"0.9\"/>\n"
            "  <path d=\"M12 18 l4 4 l8 -8\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"4\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"
            "  <path d=\"M12 30 l4 4 l8 -8\" fill=\"none\" stroke=\"#22c55e\" stroke-width=\"4\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"
            "  <circle cx=\"16\" cy=\"44\" r=\"4\" fill=\"none\" stroke=\"#94a3b8\" stroke-width=\"3\"/>\n"
            "</svg>";
            iconFile.write(svgContent.toUtf8());
            iconFile.close();
        }
    }

    window.setWindowIcon(QIcon(iconPath));
    // ------------------------------------

    window.show();
    return app.exec();
}

#include "main.moc"
