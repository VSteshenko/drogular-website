#pragma once

#include <sys/types.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace website::test {

struct CommandResult {
    int exitCode = -1;
    std::string output;
};

// A real process invocation, using the same executable as production.
CommandResult runCli(const std::vector<std::string>& arguments);

struct HttpResponse {
    int status = 0;
    std::string body;
};

// Make a loopback HTTP/1.1 request without Python or an external HTTP client.
std::optional<HttpResponse> httpGet(unsigned short port, std::string_view path);
unsigned short availableLoopbackPort();

class ServerProcess {
public:
    ServerProcess(unsigned short port, std::string_view environment);
    ServerProcess(const ServerProcess&) = delete;
    ServerProcess& operator=(const ServerProcess&) = delete;
    ~ServerProcess();

    // Throws if the server exits early, instead of returning a timeout.
    bool running();

private:
    pid_t child_ = -1;
};

} // namespace website::test
