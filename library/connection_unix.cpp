#include "connection.h"

#include <cerrno>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int GetProcessId()
{
    return ::getpid();
}

struct BaseConnectionUnix : BaseConnection {
    int sock{-1};
};

static BaseConnectionUnix Connection;
static sockaddr_un PipeAddr{};
static int MsgFlags = MSG_NOSIGNAL;


static const char* GetTempPath()
{
    const char* temp = getenv("XDG_RUNTIME_DIR");
    temp = temp ? temp : getenv("TMPDIR");
    temp = temp ? temp : getenv("TMP");
    temp = temp ? temp : getenv("TEMP");
    temp = temp ? temp : "/tmp";
    return temp;
}

BaseConnection* BaseConnection::Create()
{
    PipeAddr.sun_family = AF_UNIX;
    return &Connection;
}

void BaseConnection::Destroy(BaseConnection*& c)
{
    const auto self = reinterpret_cast<BaseConnectionUnix*>(c);
    self->Close();
    c = nullptr;
}

bool BaseConnection::Open()
{
    const char* tempPath = GetTempPath();
    const auto self = reinterpret_cast<BaseConnectionUnix*>(this);
    self->sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (self->sock == -1) {
        return false;
    }
    fcntl(self->sock, F_SETFL, O_NONBLOCK);

    for (int pipeNum = 0; pipeNum < 10; ++pipeNum) {
        snprintf(
          PipeAddr.sun_path, sizeof(PipeAddr.sun_path), "%s/discord-ipc-%d", tempPath, pipeNum);
        const int err = connect(self->sock, reinterpret_cast<const sockaddr*>(&PipeAddr), sizeof(PipeAddr));
        if (err == 0) {
            self->isOpen = true;
            return true;
        }
    }
    self->Close();
    return false;
}

bool BaseConnection::Close()
{
    const auto self = reinterpret_cast<BaseConnectionUnix*>(this);
    if (self->sock == -1)
    {
        return false;
    }
    close(self->sock);
    self->sock = -1;
    self->isOpen = false;
    return true;
}

bool BaseConnection::Write(const void* data, const size_t length)
{
    const auto self = reinterpret_cast<BaseConnectionUnix*>(this);

    if (self->sock == -1) {
        return false;
    }

    const ssize_t sentBytes = send(self->sock, data, length, MsgFlags);
    if (sentBytes < 0)
    {
        Close();
    }
    return sentBytes == static_cast<ssize_t>(length);
}

bool BaseConnection::Read(void* data, const size_t length)
{
    const auto self = reinterpret_cast<BaseConnectionUnix*>(this);

    if (self->sock == -1)
    {
        return false;
    }

    const int res = static_cast<int>(recv(self->sock, data, length, MsgFlags));
    if (res < 0)
    {
        if (errno == EAGAIN)
        {
            return false;
        }
        Close();
    }
    else if (res == 0)
    {
        Close();
    }
    return res == static_cast<int>(length);
}
