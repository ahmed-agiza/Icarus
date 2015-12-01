#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

#define MAX_READ_SIZE 65000

enum MessageType {
    Request = 0, // Q
    Reply = 1, // R
    Connect = 2, // C
    Accept = 3, // A
    Ping = 4, // P
    Pong = 5, //G
    Query = 6, //U
    Terminate = 7,
    Acknowledge = 8,
    Packet = 9, //F
    Open = 10, //O
    Close = 11, //X
    Read = 12, //D
    Write = 13, //W
    Lseek = 14, //L
    Eof = 15, //E
    Unknown = -1 // N
};

class Message {
private:
    MessageType _type;
    size_t _length;
    char _body[MAX_READ_SIZE];
    MessageType _letterToType(char typeLetter) const;
    char _typeToLetter(MessageType type) const;
    bool _valid;
public:
    Message(); //message constructor -empty
    Message (const char *content); //message constructor with content
    Message(MessageType type, const char *body); //overloading constructor

    Message (const Message &other);

    size_t writeFile(const char *fileName);
    bool isFileOperation();

    const char *getBytes() const;
    size_t getMessagSize() const;

    void setType(MessageType type);
    MessageType getType() const;

    void setLength(size_t length);
    size_t getLength() const;

    void setBody(const char *body);
    const char *getBody() const;

    bool isValid() const;
    bool isTerminationMessage() const;
    bool isAcknowledgeSuccess() const;

    ~Message();
};
#endif // MESSAGE_H
