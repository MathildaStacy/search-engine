#ifndef __SOCKETIO_H__
#define __SOCKETIO_H__

#include <cstddef>
#include <cstdint>
#include <string>

struct train_t {
    uint64_t length;
    std::string data;
};

class SocketIO
{
public:
    explicit SocketIO(int fd);
    ~SocketIO();
    bool readTrain(train_t& train);
    int readLine(char *buf, int len);
    bool writeTrain(const train_t& train);

    int readn(char *buf, int len);
    int writen(const char *buf, int len);

private:
    int _fd;
};

#endif
