#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportDialog(QWidget *parent = nullptr);

    // Getters — call these after exec() returns Accepted
    QString title()       const;
    QString description() const;
    QString severity()    const;

private slots:
    void validateAndAccept();
    void liveValidate();

private:
    bool validate();

    QLineEdit  *titleEdit;
    QTextEdit  *descEdit;
    QComboBox  *severityBox;
    QLabel     *validationLabel;
    QPushButton *submitBtn;
    QPushButton *cancelBtn;
};