#include "jsoncommunication.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>


JsonCommunication::JsonCommunication() {
    m_kind = kind::UNKNOWN_KIND;
    m_command = command::UNKNOWN_COMMAND;
}

JsonCommunication::JsonCommunication(const QByteArray& rawJson)
{
    m_command = UNKNOWN_COMMAND;
    m_kind = UNKNOWN_KIND;
    QJsonParseError err;
    QJsonObject obj = QJsonDocument::fromJson(rawJson,&err).object();
    if(err.error != QJsonParseError::NoError) {
        throw std::runtime_error("Failed to parse JSON: "+std::string(rawJson.data()));
    }
    auto n_kind = obj["kind"].toInt(-1);
    m_kind = (kind)n_kind;
    switch(m_kind) {
        case kind::COMMAND: {
            m_command = (command)obj["content"].toInt(-1);
            switch(m_command) {
                case command::QUIT:
                case command::PLAY:
                case command::INIT: {
                    //no need to actually do anything
                    break;
                }
                default: {
                    throw std::runtime_error("Invalid JSON message command number.");
                }
            }

            break;
        }
        case kind::ERRORMSG: {
            content = obj["content"].toString("null");
            break;
        }
        case kind::CONFIRMATION: {
            //no need to do anything
            break;
        }
        default: {
            throw std::runtime_error("Invalid JSON message kind number.");
        }
    }
}

JsonCommunication JsonCommunication::Error(const QString& message) {
    JsonCommunication ret;
    ret.m_kind = kind::ERRORMSG;
    ret.m_command = command::UNKNOWN_COMMAND;
    ret.content = message;
    return ret;
}

JsonCommunication JsonCommunication::Confirmation() {
    JsonCommunication ret;
    ret.m_kind = kind::CONFIRMATION;
    ret.m_command = command::UNKNOWN_COMMAND;
    return ret;
}

JsonCommunication JsonCommunication::Command(command cmd) {
    JsonCommunication ret;
    ret.m_kind = kind::COMMAND;
    ret.m_command = cmd;
    return ret;
}

JsonCommunication::kind JsonCommunication::getKind() const {
    return m_kind;
}

QString JsonCommunication::getContent() const {
    return content;
}
JsonCommunication::command JsonCommunication::getCommand() const {
    return m_command;
}
QByteArray JsonCommunication::toJson() const {
    QJsonObject obj;
    switch(m_kind) {
        case kind::COMMAND: {
            obj["kind"] = kind::COMMAND;
            switch(m_command) {
                case command::QUIT:
                case command::PLAY:
                case command::INIT: {
                    obj["content"] = m_command;
                    break;
                }
                default: {
                    throw std::runtime_error("Invalid JSON message command number.");
                }
            }

            break;
        }
        case kind::ERRORMSG: {
            obj["kind"] = kind::ERRORMSG;
            obj["content"] = content;
            break;
        }
        case kind::CONFIRMATION: {
            obj["kind"] = kind::CONFIRMATION;
            break;
        }
        default: {
            throw std::runtime_error("Invalid JSON message kind number.");
        }
    }

    QJsonDocument output(obj);
    return output.toJson(QJsonDocument::JsonFormat::Compact);
}
