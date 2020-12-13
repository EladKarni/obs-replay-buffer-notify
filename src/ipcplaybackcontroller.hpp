#ifndef IPCPLAYBACKCONTROLLER_HPP
#define IPCPLAYBACKCONTROLLER_HPP

#include <QObject>
#include <QProcess>
#include <tuple>
#include "jsoncommunication.hpp"

/** The class that handles the playback child process.
 *  
 *  The IPC is implemented using the stdin and stdout standard streams.
 *  The host and the child processes communicate using a custom JSON protocol 
 */
class IPCPlaybackController : public QObject
{
    Q_OBJECT
private:
    QProcess m_process;
    bool m_initialized;
    /** Receive a JSON message from the child process.
     *  Returns an error message with "INVALIDJSON" content upon JSON parsing exceptions
     */
    JsonCommunication receive();
    /** Send the given message to the child process.
     */
    void send(const JsonCommunication&);
public:
    /** Launches the child process. 
     * Does not send the initialization command. */
    explicit IPCPlaybackController(const QString& executable_file,const QStringList& args,QObject *parent = nullptr);
    /** The current implementation waits for the child process to finish */
    ~IPCPlaybackController();
    /** returns true if the child process has been initialized properly */
    bool initialized() const;

private slots:
    void m_exited(int exitCode, QProcess::ExitStatus exitStatus);
public slots:

    /** Sends the initialization command to the child process.
     * The returned boolean value is set to true if the operation succeeds.
     * The QString potentially contains an error message.
    */
    std::tuple<bool,QString> init();

    /** Tells the child process to shutdown. Does not block. */
    void exit();

    /** Requests playback in the child process. 
     * The returned boolean value is set to true if the operation succeeds.
     * The QString potentially contains an error message.
     * 
     * In reality this method should only fail in case of unexpected or unhandled initialization problems of the child process.
     * No error is reported if the sound is being played at the moment.
    */
    std::tuple<bool,QString> play();

    /** Wraps QProcess:state() */
    QProcess::ProcessState getState() const;
signals:
    /** event fired when the child process exits */
    void exited(int);
};

#endif // IPCPLAYBACKCONTROLLER_HPP
