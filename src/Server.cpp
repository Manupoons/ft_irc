/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarpio-mamaratr <rcarpio-mamaratr@stud    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 10:31:47 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/09 11:58:08 by rcarpio-mam      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include <algorithm>
#include <cerrno>


bool Server::_signal = false;

const Server::commandHandler Server::_commandList[Server::_commandListSize] = {
    {"CAP",     &Server::_handlerClientCap},
    {"JOIN",    &Server::_handlerClientJoin},
    {"QUIT",    &Server::_handlerClientQuit},
    {"PART",    &Server::_handlerClientPart},
    {"MODE",    &Server::_handlerClientMode},
    {"KICK",    &Server::_handlerClientKick},
    {"TOPIC",   &Server::_handlerClientTopic},
    {"INVITE",  &Server::_handlerClientInvite},
    {"PRIVMSG", &Server::_handlerClientPrivmsg},
    {"NICK",    &Server::_handlerClientNickname},
    {"USER",    &Server::_handlerClientUsername},
    {"PASS",    &Server::_handlerClientPassword},
    {"WHO",     &Server::_handlerClientWho}
};


Server::Server()
{
	std::cout << "Server constructor called" << std::endl;
	
}

Server::~Server()
{
	std::cout << "Server destructor called" << std::endl;
}

void Server::init(const std::string &port, const std::string &password)
{
    _port = std::atoi(port.c_str());
    _password = password;
    _hostname = "localhost";
    std::cout << "Server initialization" << std::endl;

    _setServerSocket();
    _addServerSignal();
    _serverLoop();
}


//? SOCKET / LOOP

void Server::_signalHandler(const int signum)
{
	if (signum == SIGINT)
	{
		std::cout << "\nSIGINT received, shutting down server..." << std::endl;
		_signal = true;
	}
}

void	Server::_closeFds()
{
	for (size_t i = 0; i < _fds.size(); ++i)
		close(_fds[i].fd);
}

void	Server::_serverLoop()
{
	while (!_signal)
	{
		int pollCount = poll(_fds.data(), _fds.size(), -1);
		if (pollCount == -1)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "Error in poll: " << strerror(errno) << std::endl;
			break;
		}

		for (size_t i = 0; i < _fds.size(); ++i)
		{
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _serverSocket)
					_acceptNewClient();
				else
					_receiveNewData(_fds[i].fd);
			}
		}
	}

	std::cout << "Shutting down server..." << std::endl;
	_closeFds();
}

void Server::_setServerSocket()
{
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("Error setting socket options");
    
    if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("Error setting non-blocking mode");

    _confSocket.sin_family = AF_INET;
    _confSocket.sin_addr.s_addr = INADDR_ANY;
    _confSocket.sin_port = htons(_port);

    if (bind(_serverSocket, (struct sockaddr *)&_confSocket, sizeof(_confSocket)) == -1)
        throw std::runtime_error("Error binding socket to port");

    if (listen(_serverSocket, 10) == -1)
        throw std::runtime_error("Error listening on socket");

    struct pollfd serverPollFd;
    serverPollFd.fd = _serverSocket;
    serverPollFd.events = POLLIN;
    serverPollFd.revents = 0;

    _fds.push_back(serverPollFd);
}

void Server::_addServerSignal()
{
	struct sigaction sa;
	sa.sa_handler = _signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGINT, &sa, NULL) == -1)
		throw std::runtime_error("Error setting signal handler");
}

void Server::_acceptNewClient()
{
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientSocket = accept(_serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (clientSocket == -1)
	{
		if (errno != EWOULDBLOCK)
			std::cerr << "Error accepting new client: " << strerror(errno) << std::endl;
		return;
	}

	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error setting non-blocking mode for client socket: " << strerror(errno) << std::endl;
		close(clientSocket);
		return;
	}

	struct pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;

	_fds.push_back(clientPollFd);
	std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
}

void Server:: _clearClient(const int fd)
{
	std::vector<struct pollfd>::iterator it = _fds.begin();
	while (it != _fds.end())
	{
		if (it->fd == fd)
		{
			it = _fds.erase(it);
			break;
		}
		else
			++it;
	}
	close(fd);
	std::cout << "Client with fd " << fd << " disconnected and cleaned up." << std::endl;
}

void Server::_receiveNewData(const int fd)
{
	char buffer[512];
	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		std::string data(buffer);
		std::cout << "Received data from fd " << fd << ": " << data << std::endl;
		_executeCommand(data, fd);
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client with fd " << fd << " has closed the connection." << std::endl;
		_clearClient(fd);
	}
	else
	{
		if (errno != EWOULDBLOCK)
			std::cerr << "Error receiving data from client with fd " << fd << ": " << strerror(errno) << std::endl;
	}
}

void Server::_sendResponse(const int fd, const std::string &response)
{
	ssize_t bytesSent = send(fd, response.c_str(), response.length(), 0);
	if (bytesSent == -1)
		std::cerr << "Error sending response to client with fd " << fd << ": " << strerror(errno) << std::endl;
	else
		std::cout << "Sent response to fd " << fd << ": " << response << std::endl;
}


//? VALIDATION

void Server::_isValidPort(const std::string &port)
{
	for (size_t i = 0; i < port.length(); ++i)
	{
		if (!isdigit(port[i]))
			throw std::invalid_argument("Port must be a number");
	}

	int portNum = std::atoi(port.c_str());
	if (portNum < 1 || portNum > 65535)
		throw std::out_of_range("Port number must be between 1 and 65535");
}

bool Server::_isClientInAnyChannel(const int fd)
{
	// for (size_t i = 0; i < _channels.size(); ++i)
	// {
	// 	if (_channels[i]->hasClient(fd))						//! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
	// 		return true;
	// }
	return false;
}

bool Server::_clientIsReadyToLogin(const int fd)
{
	Client *client = _getClient(fd);
	return client && !client->getNickname().empty() && !client->getUsername().empty();
}

bool Server::_isValidNickname(const std::string &nickname)
{
	if (nickname.empty() || nickname.length() > 9)
		return false;

	for (size_t i = 0; i < nickname.length(); ++i)
	{
		if (!isalnum(nickname[i]) && nickname[i] != '-' && nickname[i] != '_')
			return false;
	}

	return true;
}

bool Server::_isNicknameInUse(const int fd, const std::string &nickname)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getNickname() == nickname && _clients[i]->getFd() != fd)
			return true;
	}
	return false;
}

//? COMMAND HANDLERS

void Server::_handlerClientCap(const std::string &buffer, const int fd)
{
	std::cout << "Handling CAP command from fd " << fd << ": " << buffer << std::endl;
	// Implement CAP command handling logic here
}
void Server::_handlerClientJoin(const std::string &buffer, const int fd)
{
	std::cout << "Handling JOIN command from fd " << fd << ": " << buffer << std::endl;
	// Implement JOIN command handling logic here
}

void Server::_handlerClientQuit(const std::string &buffer, const int fd)
{
	std::cout << "Handling QUIT command from fd " << fd << ": " << buffer << std::endl;
}

void Server::_handlerClientPart(const std::string &buffer, const int fd)
{
	std::cout << "Handling PART command from fd " << fd << ": " << buffer << std::endl;
	// Implement PART command handling logic here
}

void Server::_handlerClientMode(const std::string &buffer, const int fd)
{
	std::cout << "Handling MODE command from fd " << fd << ": " << buffer << std::endl;
	// Implement MODE command handling logic here
}

void Server::_handlerClientKick(const std::string &buffer, const int fd)
{
	std::cout << "Handling KICK command from fd " << fd << ": " << buffer << std::endl;
	// Implement KICK command handling logic here
}

void Server::_handlerClientTopic(const std::string &buffer, const int fd)
{
	std::cout << "Handling TOPIC command from fd " << fd << ": " << buffer << std::endl;
	// Implement TOPIC command handling logic here
}

void Server::_handlerClientInvite(const std::string &buffer, const int fd)
{
	std::cout << "Handling INVITE command from fd " << fd << ": " << buffer << std::endl;
	// Implement INVITE command handling logic here
}

void Server::_handlerClientPrivmsg(const std::string &buffer, const int fd)
{
	std::cout << "Handling PRIVMSG command from fd " << fd << ": " << buffer << std::endl;
	// Implement PRIVMSG command handling logic here
}

void Server::_handlerClientNickname(const std::string &buffer, const int fd)
{
	std::cout << "Handling NICK command from fd " << fd << ": " << buffer << std::endl;
	// Implement NICK command handling logic here
}

void Server::_handlerClientUsername(const std::string &buffer, const int fd)
{
	std::cout << "Handling USER command from fd " << fd << ": " << buffer << std::endl;
	// Implement USER command handling logic here
}

void Server::_handlerClientPassword(const std::string &buffer, const int fd)
{
	std::cout << "Handling PASS command from fd " << fd << ": " << buffer << std::endl;

	std::string passwd = buffer.substr(buffer.find(' ') + 1);
	std::cout << "Extracted password: " << passwd << std::endl;

	
	// Implement PASS command handling logic here
}

void Server::_handlerClientWho(const std::string &buffer, const int fd)
{
	std::cout << "Handling WHO command from fd " << fd << ": " << buffer << std::endl;
	
	// Implement WHO command handling logic here
}

//? PARSING

void Server::_executeCommand(const std::string buffer, const int fd)
{
	std::cout << "Executing command from fd " << fd << ": " << buffer << std::endl;
	std::string command  = buffer.substr(0, buffer.find(' '));
	command = _cleanseBuffer(command, "\r\n");

	for (int i = 0; i < _commandListSize; ++i)
	{
		if (command == _commandList[i].cmd)
		{
			(this->*(_commandList[i].handler))(buffer, fd);
			return;
		}
	}
	std::cout << "Unknown command " << fd << ": " << command << std::endl;
	// Implement command parsing and execution logic here
}

std::string Server::_cleanseBuffer(const std::string &buffer, const std::string &chars_to_remove)
{
	std::string cleansed = buffer;
	for (size_t i = 0; i < chars_to_remove.length(); ++i)
		cleansed.erase(std::remove(cleansed.begin(), cleansed.end(), chars_to_remove[i]), cleansed.end());
	return cleansed;
}

std::vector<std::string> Server::_splitBuffer(const std::string &buffer, const std::string &delimiter)
{
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = buffer.find(delimiter);
	while (end != std::string::npos)
	{
		tokens.push_back(buffer.substr(start, end - start));
		start = end + delimiter.length();
		end = buffer.find(delimiter, start);
	}
	tokens.push_back(buffer.substr(start));
	return tokens;
}

//? LOOKUPS

Client* Server::_getClient(const int fd)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getFd() == fd)
			return _clients[i];
	}
	return NULL;
}
Client* Server::_getClient(const std::string nickname)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getNickname() == nickname)
			return _clients[i];
	}
	return NULL;
}

Channel* Server::_getChannel(const std::string &channel_name)
{
	// for (size_t i = 0; i < _channels.size(); ++i)
	// {
		// if (_channels[i]->getName() == channel_name)			//! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
			// return _channels[i];
	// }
	return NULL;
}

void Server::_addChannel(Channel *channel)
{
	_channels.push_back(channel);
}

void Server::_removeEmptyChannels()
{
	// _channels.erase(std::remove_if(_channels.begin(), _channels.end(),
		// [](Channel *channel) { return channel->isEmpty(); }), _channels.end()); //! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
}

//? CLIENT CYCLE

void Server::_removeClientFd(const int fd)
{
	std::vector<struct pollfd>::iterator it = _fds.begin();
	while (it != _fds.end())
	{
		if (it->fd == fd)
		{
			it = _fds.erase(it);
			break;
		}
		else
			++it;
	}
}

void Server::_removeClientFromChannel(const int fd)
{
	// for (size_t i = 0; i < _channels.size(); ++i)
	// {
		// if (_channels[i]->hasClient(fd))						//! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
			// _channels[i]->removeClient(fd);					//! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
			// break;
	// }
}

void Server::_removeClientFromServer(const int fd)
{
	_removeClientFd(fd);
	_removeClientFromChannel(fd);
	std::vector<Client *>::iterator it = _clients.begin();
	while (it != _clients.end())
	{
		if ((*it)->getFd() == fd)
		{
			delete *it;
			it = _clients.erase(it);
			break;
		}
		else
			++it;
	}
}

void Server::_sendWelcome(Client *client)
{
	std::string welcomeMsg = "Welcome to the IRC server, " + client->getNickname() + "!";
	_sendResponse(client->getFd(), welcomeMsg);
}

//? BROADCAST

void Server::_broadcastToAll(const std::string &msg, int exclude_fd)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getFd() != exclude_fd)
			_sendResponse(_clients[i]->getFd(), msg);
	}
}

void Server::_broadcastToChannel(const std::string &channelName, const std::string &msg, int exclude_fd)
{
	// Channel *channel = _getChannel(channelName);					//! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
	// if (channel)
	// {
		// std::vector<Client *> members = channel->getMembers();	//! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
		// for (size_t i = 0; i < members.size(); ++i)
		// {
			// if (members[i]->getFd() != exclude_fd)
				// _sendResponse(members[i]->getFd(), msg);
		// }
	// }
}

//? HOSTNAME

std::string Server::_getHostname()
{
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == -1)
		return "localhost";
	return std::string(hostname);
}

std::string toupper(const std::string &str)
{
	std::string result = str;
	for (size_t i = 0; i < result.length(); ++i)
		result[i] = std::toupper(result[i]);
	return result;
}