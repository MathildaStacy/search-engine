#include "reactor/SocketIO.h"
#include <cstdint>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "spdlog/common.h"
#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

SocketIO::SocketIO(int fd)
: _fd(fd)
{

}

SocketIO::~SocketIO()
{
    close(_fd);
}

//len = 10000    1500/6     1000/1
bool SocketIO::readTrain(train_t& train)
{
    uint64_t data_size = 0;
    if (recv(_fd, &data_size, sizeof(data_size), MSG_WAITALL) != sizeof(data_size)) {
        return false;
    }

    spdlog::debug("File SocketIO.cc Line 34");
    spdlog::debug("File SocketIO.cc Line 35 data_size = {}", data_size);
    char* buffer = new char[data_size + 2]();
    if (!buffer) {
        return false;
    }

    bool success = recv(_fd, buffer, data_size, MSG_WAITALL) == static_cast<ssize_t>(data_size);
    spdlog::debug("File SocketIO.cc Line 42");

    train.length = data_size;
    train.data = buffer;

    delete[] buffer;
    return success;
}

int SocketIO::readLine(char *buf, int len)
{
    int left = len - 1;
    char *pstr = buf;
    int ret = 0, total = 0;

    while(left > 0)
    {
        //MSG_PEEK不会将缓冲区中的数据进行清空,只会进行拷贝操作
        ret = recv(_fd, pstr, left, MSG_PEEK);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }
        else if(-1 == ret)
        {
            perror("readLine error -1");
            return -1;
        }
        else if(0 == ret)
        {
            break;
        }
        else
        {
            for(int idx = 0; idx < ret; ++idx)
            {
                if(pstr[idx] == '\n')
                {
                    int sz = idx + 1;
                    readn(pstr, sz);
                    pstr += sz;
                    *pstr = '\0';//C风格字符串以'\0'结尾

                    return total + sz;
                }
            }

            readn(pstr, ret);//从内核态拷贝到用户态
            total += ret;
            pstr += ret;
            left -= ret;
        }
    }
    *pstr = '\0';

    return total - left;

}

bool SocketIO::writeTrain(const train_t& train)
{
    uint64_t size = train.length;

    if (send(_fd, &size, sizeof(size), 0) != sizeof(size)) {
        return false;
    }

    if (send(_fd, train.data.c_str(), size, 0) != static_cast<ssize_t>(size)) {
        return false;
    }

    return true;
}


int SocketIO::readn(char *buf, int len)
{
    int left = len;
    char *pstr = buf;
    int ret = 0;

    while(left > 0)
    {
        ret = read(_fd, pstr, left);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }
        else if(-1 == ret)
        {
            perror("read error -1");
            return -1;
        }
        else if(0 == ret)
        {
            break;
        }
        else
        {
            pstr += ret;
            left -= ret;
        }
    }

    return len - left;
}
