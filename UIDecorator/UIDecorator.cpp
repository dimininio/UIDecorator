#include "UIDecorator.h"
#include "ui_UIDecorator.h"
#include <QDir>
#include <QUiLoader>
#include <QTextStream>
#include <QDebug>
#include <QInputDialog>
#include <QClipboard>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QColorDialog>
#include <QFontDialog>
#include <QMenu>
#include "ResourceDialog.h"
#include "GradientDialog.h"

#define STYLES_FOLDER "Styles"

UIDecorator::UIDecorator(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::UIDecorator)
{
    ui->setupUi(this);
    initialize();
}

UIDecorator::~UIDecorator()
{
    saveSettings();
    delete ui;
}

void UIDecorator::initialize()
{
    if( !QDir(STYLES_FOLDER).exists() )
        QDir().mkdir(STYLES_FOLDER);

    mpSettings = new QSettings("config.ini",QSettings::IniFormat,this);
    mpLayout = new QVBoxLayout(ui->mUIFrame);
    mpLayout->setMargin(0);
    mpResourceDialog = new ResourceDialog(this);
    mpGradientDialog = new GradientDialog(this);

    ui->mUITemplateFrame1->setAutoFillBackground(true);
    ui->mUITemplateFrame2->setAutoFillBackground(true);

    connect(ui->mComboBoxUITemplates,&QComboBox::currentTextChanged,this,[&](const QString& aFilename){ selectUITemplateFile(aFilename); });
    connect(ui->mComboBoxStyles,&QComboBox::currentTextChanged,     this,[&](const QString& aFilename){ selectStyle(aFilename); });
    connect(ui->mButtonNew,&QPushButton::clicked,                   this,[&](){ newStyle(); });
    connect(ui->mButtonDelete,&QPushButton::clicked,                this,[&](){ deleteStyle(ui->mComboBoxStyles->currentText()); });
    connect(ui->mButtonSave,&QPushButton::clicked,                  this,[&](){ saveStyle(ui->mComboBoxStyles->currentText()); });
    connect(ui->mButtonClipboard,&QPushButton::clicked,             this,[&](){ QApplication::clipboard()->setText(ui->mTextEdit->toPlainText()); });
    connect(ui->mTextEdit,&QTextEdit::textChanged,                  this,[&](){ applyStyle(); });
    connect(ui->mButtonOpenUI,&QPushButton::clicked,                this,[&](){ addUITemplateFile(); });
    connect(ui->mButtonFont,&QPushButton::clicked,                  this,[&](){ insertFont(); });

    //Color Menu
    QMenu* mColorSelectionMenu = new QMenu(this);
    ui->mButtonColor->setMenu(mColorSelectionMenu);
    QStringList colorProperties{"color","background-color","alternate-background-color","border-color","border-top-color","border-right-color",
                               "border-bottom-color","border-left-color","gridline-color","selection-color","selection-background-color"};

    for(auto& property : colorProperties)
        mColorSelectionMenu->addAction(property,[=](){insertColor(property);});

    //Resource Menu
    QMenu* mResourceSelectionMenu = new QMenu(this);
    ui->mButtonResource->setMenu(mResourceSelectionMenu);
    QStringList resourceProperties{"image","background-image","border-image"};

    for(auto& property : resourceProperties)
        mResourceSelectionMenu->addAction(property,[=](){insertResource(property);});

    //Gradient Menu
    QMenu* mGradientSelectionMenu = new QMenu(this);
    ui->mButtonGradient->setMenu(mGradientSelectionMenu);

    for(auto& property : colorProperties)
        mGradientSelectionMenu->addAction(property,[=](){insertGradient(property);});

    loadSettings();
    initializeDictionaries();

    refreshUITemplatesList();
    refreshStylesList();

    selectStyle(ui->mComboBoxStyles->currentText());
    selectUITemplateFile(ui->mComboBoxUITemplates->currentText());
}

void UIDecorator::saveSettings()
{
    mpSettings->beginGroup("UITemplates");
    mpSettings->beginWriteArray("files");
    int  i(0);
    for(auto& name : mUITemplates){
        mpSettings->setArrayIndex(i++);
        mpSettings->setValue("files",name);
    }
    mpSettings->endArray();
    mpSettings->endGroup();
}

void UIDecorator::loadSettings()
{
    mUITemplates.clear();
    mpSettings->beginGroup("UITemplates");
    int size = mpSettings->beginReadArray("files");
    for(int i = 0; i < size; i++){
        mpSettings->setArrayIndex(i);
        mUITemplates.append( mpSettings->value("files").toString());
    }
    mpSettings->endArray();
    mpSettings->endGroup();
}

void UIDecorator::initializeDictionaries()
{
    QTextCharFormat classesFormat,cssFormat,valuesFormat,controlsFormat,statesFormat;

    classesFormat.setForeground(Qt::red);
    classesFormat.setFontWeight(QFont::Bold);

    cssFormat.setForeground(Qt::blue);
    cssFormat.setFontWeight(QFont::Bold);

    valuesFormat.setForeground(Qt::darkGray);
    valuesFormat.setFontWeight(QFont::Bold);

    controlsFormat.setForeground(Qt::darkRed);
    controlsFormat.setFontWeight(QFont::Bold);

    statesFormat.setForeground(Qt::magenta);
    statesFormat.setFontWeight(QFont::Bold);

    //initializeDictionaries("://resources/values.txt","://resources/values.png",valuesFormat);
    initializeDictionaries("://resources/classes.txt","://resources/classes.png",classesFormat);
    initializeDictionaries("://resources/css.txt","://resources/css.png",cssFormat);
    initializeDictionaries("://resources/sub-controls.txt","://resources/sub-controls.png",controlsFormat);
    initializeDictionaries("://resources/pseudo-states.txt","://resources/pseudo-states.png",statesFormat);
}

void UIDecorator::initializeDictionaries(const QString& aFilename, const QString aIcon, const QTextCharFormat& aFormat)
{    
    ui->mTextEdit->addCompletionDictionary(aFilename,aIcon);
    ui->mTextEdit->addHighlightionRule(aFilename,aFormat);
}

void UIDecorator::refreshStylesList(const QString &aFoldername)
{
    auto files = QDir(aFoldername).entryList(QStringList() << "*.css",QDir::Files);
    ui->mComboBoxStyles->blockSignals(true);
    ui->mComboBoxStyles->clear();
    ui->mComboBoxStyles->addItems(files);
    ui->mComboBoxStyles->blockSignals(false);
}

void UIDecorator::refreshUITemplatesList()
{
    ui->mComboBoxUITemplates->blockSignals(true);
    ui->mComboBoxUITemplates->clear();
    for(auto& filename : mUITemplates)
    {
        QFileInfo f(filename);
        if(f.exists())
            ui->mComboBoxUITemplates->addItem(filename);
    }
    ui->mComboBoxUITemplates->blockSignals(false);
}

void UIDecorator::selectUITemplateFile(const QString &aFilename)
{
    QUiLoader loader;
    QFile file(aFilename);
    file.open(QFile::ReadOnly);
    if(file.isOpen())
    {
        mpCurrentWidget = loader.load(&file,this);
        mpCurrentWidget->setAutoFillBackground(true);
        file.close();
        while(!mpLayout->isEmpty()) {
          delete mpLayout->takeAt(0);
        }
        mpLayout->addWidget(mpCurrentWidget);
        applyStyle();
    }
}

void UIDecorator::addUITemplateFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open UI template"), "", tr("UI Files (*.ui)"));

    if(fileName.isEmpty())
        return;

    if(!mUITemplates.contains(fileName))
    {
        mUITemplates.append(fileName);
        refreshUITemplatesList();
    }
    ui->mComboBoxUITemplates->blockSignals(true);
    ui->mComboBoxUITemplates->setCurrentText(fileName);
    ui->mComboBoxUITemplates->blockSignals(false);
    selectUITemplateFile(fileName);
}

void UIDecorator::loadStyle(const QString &aFilename)
{
    QFile file("Styles/"+aFilename);
    file.open(QFile::ReadOnly);
    if(file.isOpen())
    {
        QTextStream in(&file);
        ui->mTextEdit->setText(in.readAll());
        applyStyle();
        file.close();
    }
}

void UIDecorator::newStyle()
{
    bool ok;
    QString fileName = QInputDialog::getText(this, tr("New Style"),
                                             tr("Style name:"), QLineEdit::Normal,
                                             "StyleNew", &ok);
    if (!fileName.isEmpty())
    {
        saveStyle(fileName + ".css");        
        refreshStylesList();
        ui->mComboBoxStyles->blockSignals(true);
        ui->mComboBoxStyles->setCurrentText(fileName + ".css");
        ui->mComboBoxStyles->blockSignals(false);
    }
}

void UIDecorator::saveStyle(const QString& aFilename)
{
    QFile file(QString("%1/%2").arg(STYLES_FOLDER).arg(aFilename));

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << ui->mTextEdit->toPlainText();
        file.close();
        ui->mTextEdit->setTextChangedState(false);
    }
}

void UIDecorator::deleteStyle(const QString &aFilename)
{
    QFile file("Styles/"+aFilename);
    auto res = QMessageBox::question(this, "UIDecorator", tr("Delete style: %1").arg(aFilename), QMessageBox::Yes|QMessageBox::No);
    if(res == QMessageBox::Yes)
        file.remove();
    refreshStylesList();
    mCurrentStyleName = ui->mComboBoxStyles->currentText();
    loadStyle(mCurrentStyleName);
}

void UIDecorator::applyStyle()
{
    if(mpCurrentWidget!=nullptr)
        mpCurrentWidget->setStyleSheet( ui->mTextEdit->toPlainText() );
}

void UIDecorator::selectStyle(const QString& aFilename)
{
    if(ui->mTextEdit->isTextChanged())
    {
        auto res = QMessageBox::question(this, "UIDecorator", tr("The style %1 has been changed.Do you want to save changes?").arg(aFilename), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

        if(res == QMessageBox::Yes)
        {
            saveStyle(mCurrentStyleName);
            ui->mTextEdit->setTextChangedState(false);
        }

        if(res == QMessageBox::No)
            ui->mTextEdit->setTextChangedState(false);

        if(res == QMessageBox::Cancel)
        {
            ui->mComboBoxStyles->blockSignals(true);
            ui->mComboBoxStyles->setCurrentText(mCurrentStyleName);
            ui->mComboBoxStyles->blockSignals(false);
            return;
        }
    }

    loadStyle(aFilename);
    mCurrentStyleName = aFilename;
}

void UIDecorator::insertGradient(const QString &aProperty)
{
    QString gradient =  mpGradientDialog->getGradient();
    if(!gradient.isEmpty())
        ui->mTextEdit->insertLine( aProperty + " : " + gradient + ";");
}

void UIDecorator::insertResource(const QString &aProperty)
{
    QString resource = mpResourceDialog->getResource();
    if(!resource.isEmpty())
        ui->mTextEdit->insertLine( aProperty + " : " + resource + ";");
}

void UIDecorator::insertColor(const QString& aProperty)
{
    QColor color = QColorDialog::getColor();
    ui->mTextEdit->insertLine( aProperty + " : " + color.name(QColor::NameFormat::HexArgb) + ";");
}

void UIDecorator::insertFont()
{
    bool ok;
    QFont font =QFontDialog::getFont(&ok);
    if(ok)
    {
        QString fontStyle = font.style() ? " italic" : "";
        QString fontWeight = font.bold() ? " bold" : "";
        ui->mTextEdit->insertLine(QString("font :%1%2 %3pt \"%4\";").arg(fontWeight).arg(fontStyle).arg(font.pointSize()).arg(font.family()));
    }
}
