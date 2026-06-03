/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 10:31:47 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/03 12:14:17 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

bool Server::_signal = false;

const Server::commandHandler Server::_commandList[_commandListSize] = {
	{"CAP",     &Server::_handlerClientCap},
	{"INVITE",  &Server::_handlerClientInvite},
	{"JOIN",    &Server::_handlerClientJoin},
	{"KICK",    &Server::_handlerClientKick},
	{"MODE",    &Server::_handlerClientMode},
	{"NICK",    &Server::_handlerClientNickname},
	{"PART",    &Server::_handlerClientPart},
	{"PASS",    &Server::_handlerClientPassword},
	{"PRIVMSG", &Server::_handlerClientPrivmsg},
	{"QUIT",    &Server::_handlerClientQuit},
	{"TOPIC",   &Server::_handlerClientTopic},
	{"USER",    &Server::_handlerClientUsername},
	{"WHO",     &Server::_handlerClientWho},
};

// ── Constructor / Destructor ──────────────────────────────────────────────────

Server::Server(void)
	: _port(0),
	  _password(""),
	  _serverSocket(-1),
	  _hostname("ft_irc.42.fr"),
	  _replyCode(0)
{
	memset(&_confSocket, 0, sizeof(_confSocket));
}

Server::~Server()
{
	for (size_t i = 0; i < _clients.size(); i++)
		delete _clients[i];
	_clients.clear();

	for (size_t i = 0; i < _channels.size(); i++)
		delete _channels[i];
	_channels.clear();
}

// ── Public init ───────────────────────────────────────────────────────────────

void Server::init(const std::string &port, const std::string &passwd)
{
	_isValidPort(port);
	this->_port   = std::atoi(port.c_str());
	this->_password = passwd;

	try
	{
		_addServerSignal();
		_setServerSocket();
		std::cout << GREEN << "Server listening on port " << _port << RESET << std::endl;
		_serverLoop();
		_closeFds();
	}
	catch (const std::exception &e)
	{
		_closeFds();
		std::cerr << e.what() << std::endl;
	}
}

// ── Helpers ───────────────────────────────────────────────────────────────────

std::string toupper(const std::string &str)
{
	std::string s;
	s.reserve(str.size());
	for (size_t i = 0; i < str.size(); i++)
		s.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(str[i]))));
	return s;
}

std::string Server::_getHostname()
{
	return _hostname;
}

// ── Validation ────────────────────────────────────────────────────────────────

void Server::_isValidPort(const std::string &port)
{
	int val = std::atoi(port.c_str());
	if (port.find_first_not_of("0123456789") != std::string::npos
		|| val < 1024 || val > 65535)
		throw std::invalid_argument("Invalid port: must be a number between 1024 and 65535");
}

bool Server::_isClientInAnyChannel(const int fd)
{
	Client *client = _getClient(fd);
	for (size_t i = 0; i < _channels.size(); i++)
		if (_channels[i]->hasClient(client))
			return true;
	return false;
}

bool Server::_clientIsReadyToLogin(const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return false;
	return (!client->getUname().empty()
		&& !client->getNname().empty()
		&& client->getIsAuthenticated()
		&& !client->getIsLogged());
}

bool Server::_isValidNickname(const std::string &nickname)
{
	if (nickname.empty() || nickname.size() > 9)
		return false;
	// First char: letter or special ( [ ] \ ` ^ { } | _ - )
	const std::string special = "[]\\`^{}|_-";
	if (!std::isalpha(static_cast<unsigned char>(nickname[0]))
		&& special.find(nickname[0]) == std::string::npos)
		return false;
	for (size_t i = 1; i < nickname.size(); i++)
	{
		char c = nickname[i];
		if (!std::isalnum(static_cast<unsigned char>(c))
			&& special.find(c) == std::string::npos)
			return false;
	}
	return true;
}

bool Server::_isNicknameInUse(const int fd, const std::string &nickname)
{
	for (size_t i = 0; i < _clients.size(); i++)
		if (_clients[i]->getNname() == nickname && _clients[i]->getFd() != fd)
			return true;
	return false;
}

// ── Signal ────────────────────────────────────────────────────────────────────

void Server::_signalHandler(const int signum)
{
	(void)signum;
	std::cout << "\nSignal received — shutting down." << std::endl;
	Server::_signal = true;
}

// ── Socket setup ──────────────────────────────────────────────────────────────

void Server::_setServerSocket()
{
	struct sockaddr_in serverAddr;
	int                enable = 1;

	serverAddr.sin_family      = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port        = htons(this->_port);

	this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverSocket == -1)
		throw std::runtime_error("Failed to create socket");

	if (setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1)
		throw std::runtime_error("Failed to set SO_REUSEADDR");

	if (fcntl(this->_serverSocket, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set O_NONBLOCK on server socket");

	if (bind(this->_serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		throw std::runtime_error("Failed to bind socket");

	if (listen(this->_serverSocket, SOMAXCONN) == -1)
		throw std::runtime_error("Failed to listen on socket");

	struct pollfd pfd;
	pfd.fd      = this->_serverSocket;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	this->_fdsPoll.push_back(pfd);
}

void Server::_addServerSignal()
{
	signal(SIGINT,  Server::_signalHandler);
	signal(SIGQUIT, Server::_signalHandler);
	signal(SIGPIPE, SIG_IGN); // prevent crash when writing to closed socket
}

// ── Main loop ─────────────────────────────────────────────────────────────────

void Server::_serverLoop()
{
	while (!Server::_signal)
	{
		if (poll(&this->_fdsPoll[0], this->_fdsPoll.size(), -1) == -1 && !Server::_signal)
			throw std::runtime_error("poll() failed");

		for (size_t i = 0; i < this->_fdsPoll.size(); i++)
		{
			if (!(this->_fdsPoll[i].revents & POLLIN))
				continue;

			if (this->_fdsPoll[i].fd == this->_fdPpalSocket)
				_acceptNewClient();
			else
				_receiveNewData(this->_fdsPoll[i].fd);
		}
	}
}

// ── Accept / disconnect ───────────────────────────────────────────────────────

void Server::_acceptNewClient()
{
	struct sockaddr_in cliAddr;
	socklen_t          len = sizeof(cliAddr);

	memset(&cliAddr, 0, sizeof(cliAddr));

	int fd = accept(this->_fdPpalSocket, (sockaddr *)&cliAddr, &len);
	if (fd == -1)
	{
		std::cerr << "accept() failed" << std::endl;
		return;
	}

	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl() failed on client fd" << std::endl;
		close(fd);
		return;
	}

	struct pollfd pfd;
	pfd.fd      = fd;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	this->_fdsPoll.push_back(pfd);

	Client *client = new Client(fd);
	client->setIpAddr(inet_ntoa(cliAddr.sin_addr));
	this->_clients.push_back(client);

	std::cout << GREEN << "Client <" << fd << "> connected from "
		<< client->getIpAddr() << RESET << std::endl;
}

void Server::_clearClient(const int fd)
{
	_removeClientFromChannels(fd);
	_removeClientFromServer(fd);
	_removeClientFd(fd);
	_removeEmptyChannels();
}

// ── Receive / parse ───────────────────────────────────────────────────────────

void Server::_receiveNewData(const int fd)
{
	char    buffer[1024];
	Client *cli;
	ssize_t bytes;

	std::memset(buffer, 0, sizeof(buffer));
	cli = _getClient(fd);
	if (!cli)
		return;

	bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> disconnected" << RESET << std::endl;
		_clearClient(fd);
		return;
	}

	buffer[bytes] = '\0';
	cli->appendToBuffer(buffer);

	std::string buf = cli->getBuffer();
	size_t      pos;

	while ((pos = buf.find('\n')) != std::string::npos)
	{
		std::string line = buf.substr(0, pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (!line.empty())
		{
			_executeCommand(line, fd);

			// Client may have been removed by the command (QUIT, wrong PASS, etc.)
			cli = _getClient(fd);
			if (!cli)
				return;
		}
		buf.erase(0, pos + 1);
	}

	cli = _getClient(fd);
	if (!cli)
		return;
	cli->clearBuffer();
	if (!buf.empty())
		cli->appendToBuffer(buf);
}

void Server::_sendResponse(const int fd, const std::string &response)
{
	if (response.empty())
		return;

	std::string out = response;
	if (out.size() < 2 || out.substr(out.size() - 2) != "\r\n")
		out += "\r\n";

	if (send(fd, out.c_str(), out.size(), 0) == -1)
		std::cerr << "send() failed for fd " << fd << std::endl;
}

// ── Parsing ───────────────────────────────────────────────────────────────────

std::string Server::_cleanseBuffer(const std::string &buffer, const std::string &chars)
{
	size_t pos = buffer.find_first_of(chars);
	if (pos != std::string::npos)
		return buffer.substr(0, pos);
	return buffer;
}

std::vector<std::string> Server::_splitBuffer(const std::string &buffer, const std::string &delimiter)
{
	std::vector<std::string> tokens;
	std::istringstream       iss(buffer);
	std::string              command, parameters;

	iss >> command;
	tokens.push_back(command);
	std::getline(iss >> std::ws, parameters);
	parameters.erase(0, parameters.find_first_not_of(delimiter));
	tokens.push_back(parameters);
	return tokens;
}

void Server::_executeCommand(const std::string buffer, const int fd)
{
	if (buffer.empty())
		return;

	std::string              clean  = _cleanseBuffer(buffer, CRLF);
	std::vector<std::string> parts  = _splitBuffer(clean, SPACE);
	std::string              cmd    = parts.empty()        ? "" : parts[0];
	std::string              params = parts.size() > 1     ? parts[1] : "";

	if (!cmd.empty() && cmd[0] == '/')
		cmd.erase(0, 1);
	cmd = toupper(cmd);

	for (size_t i = 0; i < _commandListSize; i++)
	{
		if (cmd == _commandList[i].command)
		{
			(this->*_commandList[i].handler)(params, fd);
			return;
		}
	}
	_sendResponse(fd, ERR_CMDNOTFOUND(_getHostname(), cmd));
}

// ── Lookups ───────────────────────────────────────────────────────────────────

Client *Server::_getClient(const int fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
		if (_clients[i]->getFd() == fd)
			return _clients[i];
	return NULL;
}

Client *Server::_getClient(const std::string nickname)
{
	for (size_t i = 0; i < _clients.size(); i++)
		if (_clients[i]->getNname() == nickname)
			return _clients[i];
	return NULL;
}

void Server::_addChannel(Channel *channel)
{
	_channels.push_back(channel);
}

Channel *Server::_getChannel(const std::string &name)
{
	for (size_t i = 0; i < _channels.size(); i++)
		if (_channels[i]->getChName() == name)
			return _channels[i];
	return NULL;
}

void Server::_removeEmptyChannels()
{
	std::vector<Channel *>::iterator it = _channels.begin();
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

// ── Client lifecycle ──────────────────────────────────────────────────────────

void Server::_removeClientFd(const int fd)
{
	for (size_t i = 0; i < _fdsPoll.size(); i++)
	{
		if (_fdsPoll[i].fd == fd)
		{
			_fdsPoll.erase(_fdsPoll.begin() + i);
			break;
		}
	}
	close(fd);
}

void Server::_removeClientFromChannels(const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;

	// Broadcast QUIT to every channel the client is in
	std::string quitMsg = client->getPrefix() + " QUIT :Connection closed\r\n";
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (_channels[i]->hasClient(client))
		{
			_broadcastToChannel(_channels[i]->getChName(), quitMsg, fd);
			_channels[i]->removeChannelClient(client);
		}
	}
}

void Server::_removeClientFromServer(const int fd)
{
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if ((*it)->getFd() == fd)
		{
			delete *it;
			_clients.erase(it);
			return;
		}
	}
}

void Server::_sendWelcome(Client *client)
{
	if (!client)
		return;

	std::string nick = client->getNname().empty()    ? "*" : client->getNname();
	std::string user = client->getUname().empty()    ? "*" : client->getUname();
	std::string host = client->getIpAddr().empty()   ? "localhost" : client->getIpAddr();
	std::string real = client->getRealName().empty() ? "*" : client->getRealName();
	int         fd   = client->getFd();

	_sendResponse(fd, RPL_WELCOME(_getHostname(), nick, user, host, real));
	_sendResponse(fd, RPL_YOURHOST(_getHostname(), nick));
	_sendResponse(fd, RPL_CREATED(_getHostname(), nick));
	_sendResponse(fd, RPL_MYINFO(_getHostname(), nick));
	_sendResponse(fd, RPL_MOTDSTART(_getHostname(), nick));
	_sendResponse(fd, RPL_MOTD(_getHostname(), nick, "Welcome to ft_irc!"));
	_sendResponse(fd, RPL_ENDOFMOTD(_getHostname(), nick));
}

// ── Broadcast ─────────────────────────────────────────────────────────────────

void Server::_broadcastToAll(const std::string &msg, int exclude_fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (!_clients[i] || _clients[i]->getFd() == exclude_fd)
			continue;
		_sendResponse(_clients[i]->getFd(), msg);
	}
}

void Server::_broadcastToChannel(const std::string &channelName, const std::string &msg, int exclude_fd)
{
	Channel *channel = _getChannel(channelName);
	if (!channel)
		return;

	std::vector<Client *> members = channel->getChClients();
	for (size_t i = 0; i < members.size(); i++)
	{
		if (!members[i] || members[i]->getFd() == exclude_fd)
			continue;
		_sendResponse(members[i]->getFd(), msg);
	}
}

// ── Fd cleanup ────────────────────────────────────────────────────────────────

void Server::_closeFds()
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		std::cout << "Closing client fd " << _clients[i]->getFd() << std::endl;
		close(_clients[i]->getFd());
	}
	if (this->_fdPpalSocket != -1)
	{
		std::cout << RED << "Server socket closed" << RESET << std::endl;
		close(this->_fdPpalSocket);
	}
}