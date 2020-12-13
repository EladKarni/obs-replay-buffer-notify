#include "ipcplaybackcontroller.hpp"
#include <QDebug>

IPCPlaybackController::IPCPlaybackController(const QString& executable_file,const QStringList& args,QObject *parent) : QObject(parent)
{
    m_initialized = false;
    connect(&m_process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,&IPCPlaybackController::m_exited);
    m_process.setWorkingDirectory(".");
    m_process.start(executable_file,args);
    m_process.setReadChannel(QProcess::StandardOutput);
    //do not return before the process is either running or dead (crashed)
    m_process.waitForStarted();

}

IPCPlaybackController::~IPCPlaybackController() {
    if(m_initialized)
        this->exit();
    m_process.waitForFinished();
}

void IPCPlaybackController::m_exited(int exitCode, QProcess::ExitStatus exitStatus) {
    emit exited(exitCode);
    Q_UNUSED(exitStatus);
}

JsonCommunication IPCPlaybackController::receive() {
    QByteArray line;
    //removing this will cause issues
    while(!m_process.canReadLine()) {
        m_process.waitForReadyRead();
    }
    line = m_process.readLine();
    std::unique_ptr<JsonCommunication> ret_ptr;
    try{
        ret_ptr.reset(new JsonCommunication(line));
        if(ret_ptr->getKind() == JsonCommunication::ERRORMSG)
            qDebug()<<"Received error: "<<ret_ptr->getContent()<<'\n';
    }catch(...){
        qDebug()<<"Received invalid JSON data. Passing further as error \"INVALIDJSON\".";
        return JsonCommunication::Error("INVALIDJSON");
    }
    return JsonCommunication(*ret_ptr);
}

void IPCPlaybackController::send(const JsonCommunication& msg) {
    auto bytes = msg.toJson();
    bytes.append('\n');
    m_process.write(bytes);
}

void IPCPlaybackController::exit() {
    send(JsonCommunication::Command(JsonCommunication::QUIT));
    m_process.closeWriteChannel();
    m_initialized = false;
}

std::tuple<bool,QString> IPCPlaybackController::play() {
    send(JsonCommunication::Command(JsonCommunication::PLAY));
    auto response = receive();
    if(response.getKind() != JsonCommunication::CONFIRMATION)
        qDebug()<<"Something went wrong.\n";
    return std::make_tuple(response.getKind() == JsonCommunication::CONFIRMATION,response.getContent());
}

std::tuple<bool,QString> IPCPlaybackController::init() {
    send(JsonCommunication::Command(JsonCommunication::INIT));
    auto msg = receive();
    m_initialized = msg.getKind() == JsonCommunication::CONFIRMATION;
    return std::make_tuple(m_initialized,msg.getContent());
}

bool IPCPlaybackController::initialized() const {
    return m_initialized;
}

QProcess::ProcessState IPCPlaybackController::getState() const {
    return m_process.state();
}
