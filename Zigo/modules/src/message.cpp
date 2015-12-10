#include "message.h"


Message::Message(Encoding encoding):_type(Unknown), _encoding(encoding), _length(0), _valid(false), _timestamp(0){

}

Message::Message (const char *content) {
  char typeLetter;

  memset(_body, 0, MAX_READ_SIZE);
  memset(_ownerId, 0, 128);
  memset(_messageId, 0, 128);
  memset(_fullId, 0, 128);
  int readValues = sscanf(content, "L %zu \nT %c \nE %d \nI %s \nM %ld \nB %64999c", &_length, &typeLetter, (int *)&_encoding, _fullId, &_timestamp, _body);
  if (readValues < 5) {
    char invalidFormatMessage[LOG_MESSAGE_LENGTH];
    sprintf(invalidFormatMessage, "Invalid message format(%d): %s", readValues, content);
    Logger::error(invalidFormatMessage);
    _valid = false;
    setType(Unknown);
  } else{
    _valid = true;
    sscanf(_fullId, "%[^-]%*c%s", _ownerId, _messageId);
    setType(_letterToType(typeLetter));
  }

}

Message::Message(MessageType type, const char *body, char *ownerId, char *messageId, Encoding encoding, size_t encodeLength):_type(type), _encoding(encoding), _length(0), _valid(true) {
  if (_type == Packet) {
    int fd = open(body, O_RDONLY);
    if(fd < 0){
      _valid = false;
      Logger::error("Failed to open packet file for reading.");
      throw FileOpenException();
    }
    read(fd, _body, MAX_READ_SIZE);
    _length = strlen(_body);
    close(fd);
  } else {
    memset(_body, 0, MAX_READ_SIZE);
    memset(_ownerId, 0, 128);
    memset(_messageId, 0, 128);
    memset(_fullId, 0, 128);
    strcpy(_ownerId, ownerId);
    strcpy(_messageId, messageId);
    sprintf(_fullId, "%s-%s", _ownerId, _messageId);
    _timestamp = (long) time(NULL);
    if (encoding == NoEncoding || encoding == RSAEncryption)
      setBody(body);
    else if (encoding == Base64)
      Crypto::base64Encode(body, encodeLength, _body, MAX_READ_SIZE);
    _length = strlen(body);
  }
}

Message::Message(const Message &other): _type(other._type), _encoding(other._encoding), _length(other._length), _valid(other._valid), _timestamp(other._timestamp){
  memset(_body, 0, MAX_READ_SIZE);
  memset(_ownerId, 0, 128);
  memset(_messageId, 0, 128);
  memset(_fullId, 0, 128);
  strcpy(_ownerId, other._ownerId);
  strcpy(_messageId, other._messageId);
  strcpy(_fullId, other._fullId);
  setBody(other._body);
}


size_t Message::writeFile(const char *fileName) {
  int fd = open(fileName, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (fd < 0) {
    Logger::error("Failed to open packet file for writing.");
    throw FileOpenException();
  }

  size_t writtenBytes = write(fd, _body, MAX_READ_SIZE);
  close(fd);
  return writtenBytes;
}

bool Message::isFileOperation() {
  return (_type == Open || _type == Read || _type == Write || _type == Close || _type == Lseek || _type == UpdateImage);
}

const char *Message::getBytes() const {
  size_t bodyLen = strlen(_body);
  size_t bufferSize = bodyLen + _numberOfDigits(_length) + sizeof(_type) + strlen(_fullId) + _numberOfDigits(_timestamp) + sizeof(_encoding);

  char *buffer = new char[bufferSize + 25];

  sprintf(buffer, "L %zu \nT %c \nE %d\nI %s \nM %ld \nB %s", _length, _typeToLetter(_type), _encoding, _fullId, _timestamp, _body);

  return buffer;
}

bool Message::isValid() const {
  return _valid;
}

bool Message::isTerminationMessage() const {
  return (_type == Terminate || (strcmp(_body, "q") == 0));
}

bool Message::isAcknowledgeSuccess() const {
  return (_type == Acknowledge && strcmp(_body, "1") == 0);
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

MessageType Message:: _letterToType(char typeLetter) const {
  if (typeLetter == 'Q')
    return Request;
  else if (typeLetter == 'R')
    return Reply;
  else if (typeLetter == 'C')
    return Connect;
  else if (typeLetter == 'A')
    return Acknowledge;
  else if (typeLetter == 'Y')
    return Accept;
  else if (typeLetter == 'D')
    return Decline;
  else if (typeLetter == 'Z')
    return Auth;
  else if (typeLetter == 'V')
    return Verify;
  else if (typeLetter == 'P')
    return Ping;
  else if (typeLetter == 'G')
    return Pong;
  else if (typeLetter == 'U')
    return Query;
  else if (typeLetter == 'T')
    return Terminate;
  else if (typeLetter == 'F')
    return Packet;
  else if (typeLetter == 'O')
    return Open;
  else if (typeLetter == 'X')
    return Close;
  else if (typeLetter == 'B')
    return Read;
  else if (typeLetter == 'W')
    return Write;
  else if (typeLetter == 'L')
    return Lseek;
  else if (typeLetter == 'E')
    return Eof;
  else if (typeLetter == 'I')
    return Information;
  else if (typeLetter == 'H')
    return UpdateImage;
  else
    return Unknown;
}

char Message::_typeToLetter(MessageType type) const {
  if (type == Request)
    return 'Q';
  else if (type == Reply)
    return 'R';
  else if (type == Connect)
    return 'C';
  else if (type == Accept)
    return 'Y';
  else if (type == Decline)
    return 'D';
  else if (type == Auth)
    return 'Z';
  else if (type == Verify)
    return 'V';
  else if (type == Acknowledge)
    return 'A';
  else if (type == Ping)
    return 'P';
  else if (type == Pong)
    return 'G';
  else if (type == Query)
    return 'U';
  else if (type == Terminate)
    return 'T';
  else if (type == Packet)
    return 'F';
  else if (type == Open)
    return 'O';
  else if (type == Close)
    return 'X';
  else if (type == Read)
    return 'B';
  else if (type == Write)
    return 'W';
  else if (type == Lseek)
    return 'L';
  else if (type == Eof)
    return 'E';
  else if (type == Information)
    return 'I';
  else if (type == UpdateImage)
    return 'H';
  else
    return 'N';
}
unsigned int Message::_numberOfDigits (unsigned num) const {
  return num > 0 ? (int) log10((double) num) + 1 : 1;
}

void Message::setEncoding(Encoding encoding) {
    _encoding = encoding;
}
Encoding Message::getEncoding() const {
  return _encoding;
}

const char *Message::getOwnerId() const {
  return _ownerId;
}
const char *Message::getMessageId() const {
  return _messageId;
}

Message::~Message() {

}
