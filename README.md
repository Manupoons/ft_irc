# ft_irc
*This project has been created as part of the 42 curriculum by mamaratr, rcarpio-.*

## Description
**ft_irc** is a 42 school project focused on building a fully functional IRC (Internet Relay Chat) server from scratch. The goal is to understand and implement the standard protocols that allow connected computers to communicate over the network, by handling multiple simultaneous client connections, parsing the IRC protocol, and supporting real-time messaging between users and channels.

The server is built in **C++98**, using non-blocking I/O and a **poll()**-based event loop to manage all client sockets without threading or forking. Connections are tested against a real IRC client to validate protocol compliance.

## Features
- **Multi-client Server**: Handles multiple simultaneous connections using a single `poll()` event loop, without blocking.
- **Authentication**: Password-protected server access with `PASS`, `NICK`, and `USER` registration flow.
- **Channel Management**: Create, join, and leave channels with `JOIN` and `PART`.
- **Private & Channel Messaging**: Real-time message delivery via `PRIVMSG`.
- **Operator Commands**: Channel moderation with `KICK`, `INVITE`, `TOPIC`, and `MODE` (invite-only, topic restrictions, user limits, operator privileges, channel keys).
- **Protocol Compliance**: Correct parsing and formatting of IRC messages, including numeric replies and edge cases in malformed input.
- **Memory & Resource Management**: Clean handling of client disconnects and server shutdown, ensuring no leaks or dangling file descriptors.

## Instructions

### Compilation
To compile the project, run:
```bash
make
```
This will generate the executable using the provided Makefile.

### Cleaning
To remove object files:
```bash
make clean
```
To remove object files and the executable:
```bash
make fclean
```
To recompile everything from scratch:
```bash
make re
```

### Execution
After compilation, run the server with:
```bash
./ircserv <port> <password>
```
Then connect using an IRC client (e.g. HexChat) pointed at `localhost:<port>` with the matching password.

## Resources

### Documentation & References
- [RFC 1459 - Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459) - The core specification for IRC message formatting and commands.
- [RFC 2812 - IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812) - Updated client-server protocol details.
- [HexChat](https://hexchat.github.io/) - Reference IRC client used for manual testing and protocol validation.

### AI Usage
AI tools were used in the following capacity for this project:
- **Code review and debugging**: Used to identify protocol-level bugs, including malformed IRC message formatting, invite-only channel logic, and NICK broadcast issues.
- **Learning resources**: Generating explanations for the IRC protocol, socket handling with `poll()`, and C++98 constraints.
- **Environment troubleshooting**: Assistance diagnosing conflict when setting up HexChat for testing.

**Note**: All core logic and implementation were developed independently by the project authors. AI was used as a supplementary tool for learning and refinement, not for generating the primary codebase.

---
*For questions or issues, please contact the project authors: **mamaratr** or **rcarpio-**.*