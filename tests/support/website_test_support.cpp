#include "support/website_test_support.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace website::test {
namespace {

std::runtime_error systemError(const char* operation) {
    return std::runtime_error(std::string(operation) + ": " + std::strerror(errno));
}

void closeFd(int fd) noexcept {
    if (fd >= 0) {
        (void)::close(fd);
    }
}

// Called only after fork; no allocations or iostream calls in the child.
[[noreturn]] void launch(const std::vector<char*>& argv) {
    if (::chdir(WEBSITE_TEST_SOURCE_ROOT) != 0) {
        ::_exit(126);
    }
    ::execv(WEBSITE_TEST_BINARY, argv.data());
    ::_exit(127);
}

std::vector<char*> commandArguments(const std::vector<std::string>& args) {
    std::vector<char*> result;
    result.reserve(args.size() + 2);
    result.push_back(const_cast<char*>(WEBSITE_TEST_BINARY));
    for (const auto& arg : args) {
        result.push_back(const_cast<char*>(arg.c_str()));
    }
    result.push_back(nullptr);
    return result;
}

int exitCode(int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return WIFSIGNALED(status) ? 128 + WTERMSIG(status) : -1;
}

int connectLoopback(unsigned short port) {
    const int socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        throw systemError("socket");
    }

    timeval timeout{};
    timeout.tv_sec = 2;
    (void)::setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    (void)::setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#ifdef SO_NOSIGPIPE
    int one = 1;
    (void)::setsockopt(socketFd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::connect(socketFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        closeFd(socketFd);
        return -1;
    }
    return socketFd;
}

} // namespace

CommandResult runCli(const std::vector<std::string>& arguments) {
    int pipeFds[2]{};
    if (::pipe(pipeFds) != 0) {
        throw systemError("pipe");
    }

    const auto argv = commandArguments(arguments);
    const pid_t pid = ::fork();
    if (pid < 0) {
        closeFd(pipeFds[0]);
        closeFd(pipeFds[1]);
        throw systemError("fork");
    }
    if (pid == 0) {
        closeFd(pipeFds[0]);
        if (::dup2(pipeFds[1], STDOUT_FILENO) < 0 ||
            ::dup2(pipeFds[1], STDERR_FILENO) < 0) {
            ::_exit(126);
        }
        closeFd(pipeFds[1]);
        launch(argv);
    }

    closeFd(pipeFds[1]);
    CommandResult result;
    char buffer[4096];
    ssize_t length = 0;
    while ((length = ::read(pipeFds[0], buffer, sizeof(buffer))) > 0) {
        result.output.append(buffer, static_cast<std::size_t>(length));
    }
    const int readError = length < 0 ? errno : 0;
    closeFd(pipeFds[0]);

    int status = 0;
    while (::waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            throw systemError("waitpid");
        }
    }
    if (readError != 0) {
        throw std::runtime_error("read subprocess output: " +
            std::string(std::strerror(readError)));
    }
    result.exitCode = exitCode(status);
    return result;
}

unsigned short availableLoopbackPort() {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw systemError("socket");
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    if (::bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        const auto error = systemError("bind");
        closeFd(fd);
        throw error;
    }
    socklen_t size = sizeof(address);
    if (::getsockname(fd, reinterpret_cast<sockaddr*>(&address), &size) != 0) {
        const auto error = systemError("getsockname");
        closeFd(fd);
        throw error;
    }
    const auto port = ntohs(address.sin_port);
    closeFd(fd);
    return port;
}

std::optional<HttpResponse> httpGet(unsigned short port, std::string_view path) {
    const int fd = connectLoopback(port);
    if (fd < 0) {
        return std::nullopt;
    }
    const std::string message = "GET " + std::string(path) +
        " HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    std::size_t sent = 0;
    while (sent < message.size()) {
#ifdef MSG_NOSIGNAL
        constexpr int flags = MSG_NOSIGNAL;
#else
        constexpr int flags = 0;
#endif
        const auto count = ::send(fd, message.data() + sent, message.size() - sent, flags);
        if (count <= 0) {
            closeFd(fd);
            return std::nullopt;
        }
        sent += static_cast<std::size_t>(count);
    }

    std::string raw;
    char buffer[8192];
    ssize_t count = 0;
    while ((count = ::recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        raw.append(buffer, static_cast<std::size_t>(count));
        if (raw.size() > 1024 * 1024) {
            closeFd(fd);
            throw std::runtime_error("Website HTTP response exceeds 1 MiB");
        }
    }
    closeFd(fd);

    const auto statusEnd = raw.find("\r\n");
    const auto bodyStart = raw.find("\r\n\r\n");
    if (statusEnd == std::string::npos || bodyStart == std::string::npos) {
        return std::nullopt;
    }
    std::istringstream statusLine(raw.substr(0, statusEnd));
    std::string httpVersion;
    HttpResponse response;
    if (!(statusLine >> httpVersion >> response.status)) {
        return std::nullopt;
    }
    response.body = raw.substr(bodyStart + 4);
    return response;
}

ServerProcess::ServerProcess(unsigned short port, std::string_view environment) {
    const std::vector<std::string> args = {
        "--port", std::to_string(port), "--env", std::string(environment)
    };
    const auto argv = commandArguments(args);
    const pid_t pid = ::fork();
    if (pid < 0) {
        throw systemError("fork");
    }
    if (pid == 0) {
        launch(argv);
    }
    child_ = pid;
}

bool ServerProcess::running() {
    if (child_ <= 0) {
        return false;
    }
    int status = 0;
    const pid_t state = ::waitpid(child_, &status, WNOHANG);
    if (state == child_) {
        child_ = -1;
        throw std::runtime_error("Website server exited unexpectedly (status " +
            std::to_string(exitCode(status)) + ")");
    }
    if (state < 0) {
        throw systemError("waitpid");
    }
    return true;
}

ServerProcess::~ServerProcess() {
    if (child_ <= 0) {
        return;
    }
    int status = 0;
    const pid_t state = ::waitpid(child_, &status, WNOHANG);
    if (state == child_ || (state < 0 && errno == ECHILD)) {
        return;
    }
    (void)::kill(child_, SIGTERM);
    for (int i = 0; i < 30; ++i) {
        const auto done = ::waitpid(child_, &status, WNOHANG);
        if (done == child_ || (done < 0 && errno == ECHILD)) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    (void)::kill(child_, SIGKILL);
    (void)::waitpid(child_, &status, 0);
}

} // namespace website::test
