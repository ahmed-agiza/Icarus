#include "message.h"


Message::Message() {

}
Message::Message (const char *content) {
    char typeLetter;
    sscanf(content, "L %zu T %c B %64999c", &_length, &typeLetter, _body);

    if (typeLetter == 'Q')
        setType(Request);
    else if (typeLetter == 'R')
        setType(Reply);
    else if (typeLetter == 'U'){
        setType(Unknown);
    } else
        throw "Unknown message type.";
}

Message::Message(MessageType type, size_t length, const char *body):_type(type), _length(length) {
    setBody(body);
}

const char *Message::getBytes() const {
    size_t bodyLen = strlen(_body);
    size_t bufferSize = bodyLen + sizeof(_length) + sizeof(_type);
    char *buffer = new char[bufferSize];
    sprintf(buffer, "L %zu\nT %c\nB %s", _length, (_type == Request)? 'Q' : (_type == Reply)? 'R' : 'U', _body);
    return buffer;
}

void Message::setType(MessageType type) {
    _type = type;
}
MessageType Message::getType() const {
    return _type;
}

void Message::setLength(size_t length) {
    _length = length;
}
size_t Message::getLength() const {
    return _length;
}

size_t Message::getMessagSize() const {
  const char *tempBody = getBytes();
  size_t messageSize = strlen(tempBody) + 1;
  delete tempBody;
  return messageSize;
}

void Message::setBody(const char *body) {
    strcpy(_body, body);
}
const char *Message::getBody() const {
    return _body;
}
