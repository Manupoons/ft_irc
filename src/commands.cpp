/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 10:50:29 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/11 13:08:28 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::_handlerClientCap(const std::string &buffer, const int fd)
{
	(void)buffer;
	(void)fd;
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
	Client *client = _getClient(fd);
	if (!client)
		return;

	if (!client->getIsAuthenticated())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NONICKNAMEGIVEN(_getHostname(), "*"));
		return;
	}
	std::string nick = buffer.substr(0, buffer.find(' '));
	if (!nick.empty() && nick[0] == ':')
		nick.erase(0, 1);

	if (!_isValidNickname(nick))
	{
		_sendResponse(fd, ERR_ERRORNICKNAME(_getHostname(), "*", nick));
		return;
	}
	if (_isNicknameInUse(fd, nick))
	{
		_sendResponse(fd, ERR_NICKNAMEINUSE(_getHostname(), nick));
		return;
	}
	
	if (client->getIsLogged())
	{
		std::string oldPrefix = client->getPrefix();
		client->setNickname(nick);
		_broadcastToAll(oldPrefix + "NICK :" + nick + "\r\n", -1);
		return;
	}

	client->setNickname(nick);
	if(_clientIsReadyToLogin(fd))
	{
		client->setIsLogged(true);
		_sendWelcome(client);
	}
}

void Server::_handlerClientUsername(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;
	
	if (!client->getIsAuthenticated())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (client->getIsLogged())
	{
		_sendResponse(fd, ERR_ALREADYREGISTERED(_getHostname(), client->getNickname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), "*", "USER"));
		return;
	}
	
	std::istringstream iss(buffer);
	std::string username, mode, unused, realname;
	iss >> username >> mode >> unused;
	std::getline(iss >> std::ws, realname);
	if (!realname.empty() && realname[0] == ':')
		realname.erase(0, 1);
	if (username.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), "*", "USER"));
		return;
	}
	
	client->setUsername(username);
	client->setRealname(realname.empty() ? username : realname);
	if(_clientIsReadyToLogin(fd))
	{
		client->setIsLogged(true);
		_sendWelcome(client);
	}
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