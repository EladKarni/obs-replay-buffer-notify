#ifndef JSONCOMMUNICATION_HPP
#define JSONCOMMUNICATION_HPP
#include <QByteArray>
#include <QString>

/** Implements a custom extensible JSON communication protocol meant for IPC between the playback service and the plugin
 */
class JsonCommunication
{


public:
    /** Represents the kind of information carried by a given message */
    enum kind {
        UNKNOWN_KIND = -1,
        CONFIRMATION,
        ERRORMSG, /**< could not have been "ERROR" because it makes MSVC angry */
        COMMAND
    };
    /** Used to differentiate between commands send to the child process */
    enum command {
        UNKNOWN_COMMAND = -1,
        INIT,
        PLAY,
        QUIT
    };
    /** This constructor should only be used internally to parse a JSON message. 
     * Throws in case of invalid data.*/
    JsonCommunication(const QByteArray& rawJson);

    /** A named constructor to create error messages */
    static JsonCommunication Error(const QString& message);

    /** A named constructor to create confirmation messages */
    static JsonCommunication Confirmation();

    /** A named constructor to create command messages */
    static JsonCommunication Command(command cmd);

    kind getKind() const;
    QString getContent() const;
    command getCommand() const;
    QByteArray toJson() const;
private:
    //meant to be private-only
    JsonCommunication();
    
    QString content;
    kind m_kind;
    command m_command;
};

#endif // JSONCOMMUNICATION_HPP
