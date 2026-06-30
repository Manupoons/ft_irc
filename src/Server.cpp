/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarpio-cyepes <rcarpio-cyepes@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 10:31:47 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/30 08:29:33 by rcarpio-cye      ###   ########.fr       */
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
	Client *newClient = new Client(clientSocket);
	newClient->setIpAddr(inet_ntoa(clientAddr.sin_addr));
	_clients.push_back(newClient);
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
	Client *client = _getClient(fd);
	if (!client)
		return;

	char buffer[512];
	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		client->appendToBuffer(std::string(buffer));
		
		std::string clientBuffer = client->getBuffer();
		size_t pos;
		while ((pos = clientBuffer.find('\n')) != std::string::npos)
		{
			std::string cmdLine = clientBuffer.substr(0, pos);
			if (!cmdLine.empty() && cmdLine[cmdLine.length() - 1] == '\r')
				cmdLine.erase(cmdLine.length() - 1);
				
			clientBuffer.erase(0, pos + 1);
			client->clearBuffer();
			client->appendToBuffer(clientBuffer);
			
			if (!cmdLine.empty())
				_executeCommand(cmdLine, fd);
		}
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
	Client *client = _getClient(fd);
	if (!client)
		return false;
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->hasClient(client))
			return true;
	}
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

//? PARSING

void Server::_executeCommand(const std::string buffer, const int fd)
{
	std::cout << "Executing command from fd " << fd << ": " << buffer << std::endl;
	
	size_t firstSpace = buffer.find(' ');
	std::string command;
	std::string arguments;
	if (firstSpace == std::string::npos)
	{
		command = buffer;
		arguments = "";
	}
	else
	{
		command = buffer.substr(0, firstSpace);
		size_t firstArgChar = buffer.find_first_not_of(" ", firstSpace);
		if (firstArgChar != std::string::npos)
			arguments = buffer.substr(firstArgChar);
		else
			arguments = "";
	}

	command = toupper(command);
	command = _cleanseBuffer(command, "\r\n");

	for (int i = 0; i < _commandListSize; ++i)
	{
		if (command == _commandList[i].cmd)
		{
			(this->*(_commandList[i].handler))(arguments, fd);
			return;
		}
	}
	std::cout << "Unknown command " << fd << ": " << command << std::endl;
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
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->getName() == channel_name)
			return _channels[i];
	}
	return NULL;
}

void Server::_addChannel(Channel *channel)
{
	_channels.push_back(channel);
}

void Server::_removeEmptyChannels()
{
	std::vector<Channel*>::iterator it = _channels.begin();
	while (it != _channels.end())
	{
		if ((*it)->clientCount() == 0)
		{
			delete *it;
			it = _channels.erase(it);
		}
		else
			++it;
	}
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
	Client* client = _getClient(fd);
	if (!client)
		return;
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->hasClient(client))
			_channels[i]->removeClient(client);
	}
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
	Channel *channel = _getChannel(channelName);					//! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
	if (channel)
	{
		std::vector<Client *> members = channel->getMembers();	//! DESCOMENTAR CUANDO ESTA LA CLASE CHANNEL
		for (size_t i = 0; i < members.size(); ++i)
		{
			if (members[i]->getFd() != exclude_fd)
				_sendResponse(members[i]->getFd(), msg);
		}
	}
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