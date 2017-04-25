#include "armemus.h"
#include "ui_armemus.h"

#include <QScrollBar>

armemus::armemus(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::armemus)
{
    ui->setupUi(this);

    /****************************************
     *  Connects signals - slots ARMEmuS App
     ****************************************/

    //Cierra conexión generada por el error

    QProcess closeConnection;

    closeConnection.start("bash",QStringList()<<"-c"<<"fuser -k 1234/tcp");
    closeConnection.waitForFinished(-1);
    closeConnection.close();

    //--------------------------------------------------

    connect(ui->actionNew, &QAction::triggered, this, &armemus::actionNew);
    connect(ui->actionOpen, &QAction::triggered, this, &armemus::actionOpen);
    connect(ui->actionSave, &QAction::triggered, this, &armemus::actionSave);
    connect(ui->actionCloseProject, &QAction::triggered, this, &armemus::actionCloseProject);

    connect(ui->actionHelp, &QAction::triggered, this, &armemus::actionHelp);
    connect(ui->actionAbout, &QAction::triggered, this, &armemus::actionAbout);
    connect(ui->actionExit, &QAction::triggered, this, &armemus::actionExit);
    connect(ui->actionBuildOptions, &QAction::triggered, this, &armemus::actionBuildOptions);

    connect(ui->actionNewFile, &QAction::triggered, this, &armemus::actionNewFile);
    connect(ui->actionOpenFile, &QAction::triggered, this, &armemus::actionOpenFile);
    connect(ui->actionCloseFile, &QAction::triggered, this, &armemus::actionCloseFile);

    connect(ui->actionBuild, &QAction::triggered, this, &armemus::actionBuild);
    connect(ui->actionPlay, &QAction::triggered, this, &armemus::actionPlay);
    connect(ui->actionStop, &QAction::triggered, this, &armemus::actionStop);

    existProject = false;

    DisableButtons();
    loadBoards();
    project = new aproject(this, boards);

    BuildProcess.setProcessChannelMode(QProcess::MergedChannels);
    QemuProcess.setProcessChannelMode(QProcess::MergedChannels);
}


armemus::~armemus()
{
    delete ui;
}

void armemus::actionNew()
{
    update_editorStatus();

    if (!existProject)
        setWorkspace();

    else{
        if(QemuProcess.isOpen())
            emit actionStop();
        const QMessageBox::StandardButton sel =
                QMessageBox::question(this,
                                      tr(APROJECT_NAME),
                                      tr("Do you want to close this project and to create a new project?"));

        switch (sel) {
        case QMessageBox::Yes:
            if(confirmSave())
                break;
            else
                return;
        case QMessageBox::No:
            return;
        default:
            break;
        }
        clearWorkspace();
        setWorkspace();
        }
}


void armemus::actionNewFile()
{
    editor->newFile();

}

void armemus::actionOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Proyect"), QString(),
                tr("Proyect Files (*.apf)"));
}

void armemus::actionOpenFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this);

    if (!fileName.isEmpty())
        editor->openFile(fileName);
}

bool armemus::actionSave()
{
    update_editorStatus();

    if(editorStatus[1]){
        editor->findtabUnsaved(tabUnsaved);
        if(editor->save())
            return this->actionSave();
        else
            return false;
    }

    statusBar()->showMessage(tr("Project saved"), 3000);
    ui->actionBuild->setEnabled(true);
    return true;
}

void armemus::actionHelp()
{
    QMessageBox Help (this);

    Help.information(this,tr("Help"),tr("Help Menu"));
}

void armemus::actionAbout()

{
    QMessageBox About(this);

    QLabel link;
    link.setText(APROJECT_NAME"<br>Icons:<a href=\"https://snwh.org/paper/\">Paper Icons</a></br> by Sam Hewitt");
    link.setOpenExternalLinks(true);

    About.about(this,tr("About " APROJECT_NAME), link.text());
}

void armemus::actionExit()
{
    this->close();
}


void armemus::actionBuildOptions()
{
    abuild *buildOptions = new abuild(this, projectInfo.path, projectInfo.name);
    buildOptions->setModal(true);
    buildOptions->show();
}

void armemus::actionBuild()
{
    BuildProcess.setProcessChannelMode(QProcess::MergedChannels);
    connect(&BuildProcess, &QProcess::readyRead, this, &armemus::printProcess);
    connect(&BuildProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &armemus::closeProcess);

    outputBrowser->clear();

    QString root = "/home/malo";
    QString ABuilder = "/Escritorio/Builders/Arduino Builder/";

    QString path_File(FileBoard);
    QString path_Build = projectInfo.path+"/"+projectInfo.name+"/Build";

    switch (projectInfo.boardIndex) {
    case ArduinoDue:
        BuildProcess.start(root+ABuilder+"arduino-builder", QStringList()<<"-compile"<<"-logger"<<"machine"<<"-hardware"<<root+ABuilder+"hardware"<<"-hardware"<<root+ABuilder+"arduino-packages/packages"<<"-tools"<<root+ABuilder+"tools-builder"<<"-tools"<<root+ABuilder+"hardware/tools/avr"<<"-tools"<<root+ABuilder+"arduino-packages/packages"<<"-built-in-libraries"<<root+ABuilder+"libraries"<<"-libraries"<<root+ABuilder+"Arduino/libraries"<<"-fqbn"<<"arduino:sam:arduino_due_x"<<"-build-path"<<path_Build<<"-warnings"<<"none"<<"-prefs"<<"build.warn_data_percentage=75"<<"-prefs"<<"-runtime.tools.bossac.path="+root+ABuilder+"arduino-packages/packages/arduino/tools/bossac/1.6.1-arduino"<<"-prefs"<<"runtime.tools.arm-none-eabi-gcc.path="+root+ABuilder+"arduino-packages/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1"<<"-verbose"<<path_File);
        break;
    case ArduinoZero:
        BuildProcess.start(root+ABuilder+"arduino-builder", QStringList()<<"-compile"<<"-logger"<<"machine"<<"-hardware"<<root+ABuilder+"hardware"<<"-hardware"<<root+ABuilder+"arduino-packages/packages"<<"-tools"<<root+ABuilder+"tools-builder"<<"-tools"<<root+ABuilder+"hardware/tools/avr"<<"-tools"<<root+ABuilder+"arduino-packages/packages"<<"-built-in-libraries"<<root+ABuilder+"libraries"<<"-libraries"<<root+ABuilder+"Arduino/libraries"<<"-fqbn"<<"arduino:samd:arduino_zero_native"<<"-build-path"<<path_Build<<"-warnings"<<"none"<<"-prefs"<<"build.warn_data_percentage=75"<<"-prefs"<<"-runtime.tools.bossac.path="+root+ABuilder+"arduino-packages/packages/arduino/tools/bossac/1.7.0"<<"-prefs"<<"runtime.tools.arm-none-eabi-gcc.path="+root+ABuilder+"arduino-packages/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1"<<"-prefs"<<"runtime.tools.openocd.path="+root+ABuilder+"arduino-packages/packages/arduino/tools/openocd/0.9.0-arduino"<<"-prefs"<<"runtime.tools.CMSIS.path="+root+ABuilder+"arduino-packages/packages/arduino/tools/CMSIS/4.0.0-atmel"<<"-verbose"<<path_File);
        break;
    case Tiva:
        //se debe cambiar en el archivo Makefile del resource la linea
        //TIVAWARE_PATH = /home/malo/Escritorio/Builders/SW-EK-TM4C123GXL
        BuildProcess.start("make", QStringList()<<"-C"<<projectInfo.path+"/"+projectInfo.name+"/"+projectInfo.name);
        break;
    default:
        break;
    }    
}

void armemus::actionPlay()
{
    ui->actionStop->setEnabled(true);
    ui->actionPlay->setEnabled(false);    

    tabs->setCurrentIndex(1);
    outputBrowser->clear();

    QString File=projectInfo.path+"/"+projectInfo.name+"/Build/"+projectInfo.name+".ino.elf";
    QString command="--eval-command";

    board->turnOn();

    board->Painter->drawInputs(IOBoard.pinsboard, IOBoard.whpin, IOBoard.pinRect);

    board->Painter->drawLed(&IOBoard.ledsboard[0], IOBoard.whled, IOBoard.ledRect);

    QemuProcess.start("/home/malo/qemu-armemus/qemu-system-gnuarmeclipse", QStringList()<<"--mcu"<<"SAM3X8E"<<"--gdb"<<"tcp::1234"<<"-L"<<"/home/malo/qemu-armemus"<<"--verbose"<<"--verbose");

    QemuProcess.waitForStarted(-1);    

    GDBprocess.start("arm-none-eabi-gdb",QStringList()<<command<<"target remote :1234"<<command<<"load"<<File<<command<<"c");

    GDBprocess.waitForStarted(-1);

    QemuProcess.waitForBytesWritten(-1);

    connect(&QemuProcess, &QProcess::readyRead, this, &armemus::printProcess);
    connect(&QemuProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &armemus::closeProcess);
}

void armemus::actionStop()
{
    QemuProcess.close();
    GDBprocess.close();

    disconnect(&QemuProcess, &QProcess::readyRead, this, &armemus::printProcess);
    disconnect(&QemuProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &armemus::closeProcess);

    board->turnOff();
    ui->actionPlay->setEnabled(true);
    ui->actionStop->setEnabled(false);
}

void armemus::actionCloseProject()
{
    update_editorStatus();

    if(editorStatus[0])
        if(!confirmSave())
            return;

    clearWorkspace();
}

void armemus::actionCloseFile()
{

    editor->tabClose();

}

void armemus::loadBoards()
{
    QFile fileBoards(":/boards/boards.txt");
    fileBoards.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&fileBoards);
    Board board;

    while (!in.atEnd()) {
        in >> board.name >> board.image;
        if( !board.name.isEmpty() )
            boards.append(board);
    }
}

//Workspace settings

inline void armemus::clearWorkspace(){

    existProject=false;
    DisableButtons();
    delete outputBrowser;
    //delete editor;
    //delete board;
    delete tabs;
    IOBoard.clear();
}


inline void armemus::DisableButtons()
{

    //menu
    ui->actionNewFile->setEnabled(false);
    ui->actionOpenFile->setEnabled(false);
    ui->actionCloseFile->setEnabled(false);
    ui->actionCloseProject->setEnabled(false);
    ui->actionBuildOptions->setEnabled(false);

    //toolbar
    ui->actionSave->setEnabled(false);
    ui->actionBuild->setEnabled(false);
    ui->actionPlay->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionStep->setEnabled(false);
}

inline void armemus::setWorkspace()
{
    project->clear();
    project->exec();
    project->getInfo(projectInfo);

    if ( !projectInfo.name.isEmpty() && !projectInfo.path.isEmpty() ){

        outputBrowser = new QTextBrowser;

        tabs = new QTabWidget;

        editor = new aeditortab(projectInfo.boardIndex);
        board = new aboardtab;
        IOBoard.setBoard(projectInfo.boardIndex);

        tabs->addTab(editor,"Code");
        tabs->addTab(board,"Board");

        ui->mainLayout->addWidget(tabs);
        ui->outputLayout->addWidget(outputBrowser);

        tabs->setCurrentIndex(0);

        project->getFilePath(FileBoard);
        editor->openFile(FileBoard);
        board->loadFile(":/boards/"+boards[projectInfo.boardIndex].image);

        existProject = true;

        ui->actionNewFile->setEnabled(true);
        ui->actionOpenFile->setEnabled(true);
        ui->actionCloseFile->setEnabled(true);
        ui->actionCloseProject->setEnabled(true);
        ui->actionBuildOptions->setEnabled(true);
        ui->actionSave->setEnabled(true);

    }
}

void  armemus::printProcess()

{
    QByteArray processReader;

    if(BuildProcess.isReadable()){
        processReader=BuildProcess.readAll();
        outputBrowser->insertPlainText(processReader);
        outputBrowser->verticalScrollBar()->setValue(outputBrowser->verticalScrollBar()->maximum());
    }

    else{           //if(QemuProcess.isReadable()){
        processReader=QemuProcess.readAll();
        outputBrowser->insertPlainText(processReader);
        outputBrowser->verticalScrollBar()->setValue(outputBrowser->verticalScrollBar()->maximum());

        if(processReader.contains("HIGH") || processReader.contains("LOW")){
            int index=getIndex(processReader);

            if(!board->Painter->outputStr.contains("Pin"+QString::number(index)))
                board->Painter->drawPin(&IOBoard.pinsboard[index], IOBoard.whpin, IOBoard.pinRect);

            if(IOBoard.pinsboard[index].led!=-1){
                if(!board->Painter->outputStr.contains("Led"+QString::number(IOBoard.pinsboard[index].led)))
                    board->Painter->drawLed(&IOBoard.ledsboard[IOBoard.pinsboard[index].led], IOBoard.whled, IOBoard.ledRect);

                QByteArray copyOut(processReader);
                copyOut=copyOut.mid(copyOut.indexOf(":"));
                copyOut.insert(0,"led");
                board->Painter->drawStatus(copyOut, IOBoard.pinsboard[index].led);
            }
            board->Painter->drawStatus(processReader, index);
        }
    }
}



bool armemus::confirmSave()

{
    if(editorStatus[1]){
        const QMessageBox::StandardButton sel =
                QMessageBox::question(this,
                                      tr(APROJECT_NAME),
                                      tr("Some files have been modified\nDo you want to save before closing it?"));
        switch (sel) {
        case QMessageBox::Yes:
            if(!this->actionSave())
                return false;
        case QMessageBox::No:
            break;
        default:
            break;
        }
    }

    return true;
}

void armemus::closeProcess()
{
    if(BuildProcess.isOpen()){
        BuildProcess.close();
        disconnect(&BuildProcess, &QProcess::readyRead, this, &armemus::printProcess);
        disconnect(&BuildProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &armemus::closeProcess);
        ui->actionPlay->setEnabled(true);
    }

    if(QemuProcess.isOpen())
        emit actionStop();
}


void armemus::update_editorStatus()
{


    if(existProject)
        editor->update_editorStatus(editorStatus, tabUnsaved);

    else{
        editorStatus[0]=false;
        editorStatus[1]=false;
    }


}

int armemus::getIndex(const QByteArray &processReader)
{
    QString Reader(processReader);
    int index;

    Reader=Reader.simplified();
    Reader.replace(Reader.indexOf(":"),Reader.size(),"");
    index=IOBoard.pinString.indexOf(Reader);

    return index;
}


void armemus::closeEvent (QCloseEvent *event)
{
    update_editorStatus();

    if(QemuProcess.isOpen())
        emit actionStop();

    if(existProject && editorStatus[0]){
        if(editorStatus[1]){
            if(confirmSave()){
                clearWorkspace();
                event->accept();
            }
            else
                event->ignore();
        }
    }
    else
        event->accept();
}
