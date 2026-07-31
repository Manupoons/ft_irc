/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 10:50:29 by mamaratr          #+#    #+#             */
/*   Updated: 2026/07/30 14:03:43 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

void Server::_handlerClientCap(const std::string &buffer, const int fd)
{
	(void)buffer;
	(void)fd;
}
void Server::_handlerClientJoin(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;
	if (!client->getIsLogged())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), client->getNickname(), "JOIN"));
		return;
	}
	
	std::istringstream iss(buffer);
	std::string channelList, passwordList;
	iss >> channelList >> passwordList;
	std::vector<std::string> channels, passwords;
	std::string token;
	std::istringstream chStream(channelList);
	while (std::getline(chStream, token, ','))
		channels.push_back(token);
	std::istringstream passwordStream(passwordList);
	while (std::getline(passwordStream, token, ','))
		passwords.push_back(token);

	for (size_t i = 0; i < channels.size(); i++)
	{
		std::string chName = channels[i];
		std::string password = (i < passwords.size() ? passwords[i] : "");

		if (chName.empty() || chName[0] != '#')
		{
			_sendResponse(fd, ERR_NOSUCHCHANNEL(_getHostname(), client->getNickname(), chName));
			continue;
		}

		Channel *channel = _getChannel(chName);
		if (channel)
		{
			if (channel->hasClient(client))
				continue;
			if (channel->getInviteOnly() && !channel->isInvited(client))
			{
				_sendResponse(fd, ERR_INVITEONLYCHAN(_getHostname(), client->getNickname(), chName));
				continue;
			}
			if (!channel->getPassword().empty() && channel->getPassword() != password)
			{
				_sendResponse(fd, ERR_BADCHANNELKEY(_getHostname(), client->getNickname(), chName));
				continue;
			}
			if (channel->getMaxUsers() > 0 && channel->clientCount() >= channel->getMaxUsers())
			{
				_sendResponse(fd, ERR_CHANNELISFULL(_getHostname(), client->getNickname(), chName));
				continue;
			}
		}
		else
		{
			channel = new Channel(chName);
			_addChannel(channel);
		}
		channel->addClient(client);
		if (channel->clientCount() == 1)
			channel->addOperator(client);
		channel->removeInvited(client);
		std::string joinMsg = client->getPrefix() + " JOIN " + chName + "\r\n";
		_broadcastToChannel(chName, joinMsg, -1);
		if (!channel->getTopic().empty())
			_sendResponse(fd, RPL_TOPIC(_getHostname(), client->getNickname(), chName, channel->getTopic()));
		else
			_sendResponse(fd, RPL_NOTOPIC(_getHostname(), client->getNickname(), chName));
		_sendResponse(fd, RPL_NAMREPLY(_getHostname(), client->getNickname(), chName, channel->getNickList()));
		_sendResponse(fd, RPL_ENDOFNAMES(_getHostname(), client->getNickname(), chName));
	}
}

void Server::_handlerClientQuit(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;

	std::string reason = buffer;
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	if (reason.empty())
		reason = "Client quit";
	std::string msg = client->getPrefix() + " QUIT :" + reason + "\r\n";

	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->hasClient(client))
			_broadcastToChannel(_channels[i]->getName(), msg, fd);
	}
	_removeClientFromServer(fd);
}

void Server::_handlerClientPart(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;
	if (!client->getIsLogged())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), client->getNickname(), "PART"));
		return;
	}
	std::istringstream iss(buffer);
	std::string channelList, reason;
	iss >> channelList;
	std::getline(iss >> std::ws, reason);
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	if (reason.empty())
		reason = "Leaving";
	std::vector<std::string> channels;
	std::string token;
	std::istringstream chStream(channelList);
	while ( std::getline(chStream, token, ','))
		channels.push_back(token);
	for (size_t i = 0; i < channels.size(); i++)
	{
		std::string chName = channels[i];
		Channel *channel = _getChannel(chName);
		
		if (!channel)
		{
			_sendResponse(fd, ERR_NOSUCHCHANNEL(_getHostname(), client->getNickname(), chName));
			continue;
		}
		if (!channel->hasClient(client))
		{
			_sendResponse(fd, ERR_NOTONCHANNEL(_getHostname(), client->getNickname(), chName));
			continue;
		}
		std::string partMsg = client->getPrefix() + " PART " + chName + " :" + reason + "\r\n";
		_broadcastToChannel(chName, partMsg, -1);
		channel->removeClient(client);
	}
	_removeEmptyChannels();
}

void Server::_handlerClientMode(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;
	if (!client->getIsLogged())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), client->getNickname(), "MODE"));
		return;
	}

	std::istringstream iss(buffer);
	std::string target, modeStr;
	iss >> target >> modeStr;

	if (modeStr.empty())
	{
		Channel *channel = _getChannel(target);
		if (!channel)
		{
			_sendResponse(fd, ERR_NOSUCHCHANNEL(_getHostname(), client->getNickname(), target));
			return;
		}
		std::string modes = channel->getModeString();
		if (modes.empty())
			modes = "+";
		_sendResponse(fd, RPL_CHANNELMODEIS(_getHostname(), client->getNickname(), target, modes));
		return;
	}

	if (target.empty() || target[0] != '#')
	{
		_sendResponse(fd, ERR_UMODEUNKNOWNFLAG(_getHostname(), client->getNickname()));
		return;
	}

	Channel *channel = _getChannel(target);
	if (!channel)
	{
		_sendResponse(fd, ERR_NOSUCHCHANNEL(_getHostname(), client->getNickname(), target));
		return;
	}
	if (!channel->hasClient(client))
	{
		_sendResponse(fd, ERR_NOTONCHANNEL(_getHostname(), client->getNickname(), target));
		return;
	}
	if (!channel->isOperator(client))
	{
		_sendResponse(fd, ERR_CHANOPRIVSNEEDED(_getHostname(), client->getNickname(), target));
		return;
	}

	std::vector<std::string> params;
	std::string p;
	while (iss >> p)
		params.push_back(p);
	size_t paramIdx = 0;

	bool adding = true;
	std::string modeMsg;
	std::string appliedParams;
	char lastSign = 0;

	for (size_t i = 0; i < modeStr.size(); i++)
	{
		char c = modeStr[i];
		if (c == '+')
		{
			adding = true;
			continue;
		}
		if (c == '-')
		{
			adding = false;
			continue;
		}

		bool applied = false;
		std::string paramUsed;

		switch (c)
		{
			case 'i':
				channel->setInviteOnly(adding);
				applied = true;
				break;

			case 't':
				channel->setTopicRestricted(adding);
				applied = true;
				break;

			case 'k':
				if (adding)
				{
					if (paramIdx >= params.size())
						break;
					channel->setPassword(params[paramIdx++]);
					paramUsed = channel->getPassword();
					applied = true;
				}
				else
				{
					channel->setPassword("");
					applied = true;
				}
				break;

			case 'l':
				if (adding)
				{
					if (paramIdx >= params.size())
						break;
					int lim = std::atoi(params[paramIdx].c_str());
					if (lim > 0)
					{
						channel->setMaxUsers(lim);
						paramUsed = params[paramIdx];
						applied = true;
					}
					paramIdx++;
				}
				else
				{
					channel->setMaxUsers(0);
					applied = true;
				}
				break;

			case 'o':
			{
				if (paramIdx >= params.size())
					break;
				std::string opNick = params[paramIdx++];
				Client *opTarget = _getClient(opNick);
				if (!opTarget || !channel->hasClient(opTarget))
				{
					_sendResponse(fd, ERR_USERNOTINCHANNEL(_getHostname(), client->getNickname(), opNick, target));
					break;
				}
				if (adding)
					channel->addOperator(opTarget);
				else
					channel->removeOperator(opTarget);
				paramUsed = opNick;
				applied = true;
				break;
			}

			default:
				_sendResponse(fd, ERR_UNKNOWNMODE(_getHostname(), client->getNickname(), c, target));
				break;
		}

		if (applied)
		{
			char sign = adding ? '+' : '-';
			if (sign != lastSign)
			{
				modeMsg += sign;
				lastSign = sign;
			}
			modeMsg += c;
			if (!paramUsed.empty())
				appliedParams += " " + paramUsed;
		}
	}

	if (!modeMsg.empty())
	{
		std::string fullMsg = client->getPrefix() + " MODE " + target + " " + modeMsg + appliedParams + "\r\n";
		_broadcastToChannel(target, fullMsg, -1);
	}
}

void Server::_handlerClientKick(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;
	if (!client->getIsLogged())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), client->getNickname(), "KICK"));
		return;
	}
	
	std::istringstream iss(buffer);
	std::string chName, targetNick, reason;
	iss >> chName >> targetNick;
	std::getline(iss >> std::ws, reason);
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	if (reason.empty())
		reason = "Kicked";
	if (chName.empty() || targetNick.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), client->getNickname(), "KICK"));
		return;
	}

	Channel *channel = _getChannel(chName);
	if (!channel)
	{
		_sendResponse(fd, ERR_NOSUCHCHANNEL(_getHostname(), client->getNickname(), chName));
		return;
	}
	if (!channel->hasClient(client))
	{
		_sendResponse(fd, ERR_NOTONCHANNEL(_getHostname(), client->getNickname(), chName));
		return;
	}
	if (!channel->isOperator(client))
	{
		_sendResponse(fd, ERR_CHANOPRIVSNEEDED(_getHostname(), client->getNickname(), chName));
		return;
	}
	
	Client *target = _getClient(targetNick);
	if (!target || !channel->hasClient(target))
	{
		_sendResponse(fd, ERR_USERNOTINCHANNEL(_getHostname(), client->getNickname(), targetNick, chName));
		return;
	}
	
	std::string kickMsg = client->getPrefix() + " KICK " + chName + " " + targetNick + " :" + reason + "\r\n";
	_broadcastToChannel(chName, kickMsg, -1);
	channel->removeClient(target);
	_removeEmptyChannels();
}

void Server::_handlerClientTopic(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;
	if (!client->getIsLogged())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), client->getNickname(), "TOPIC"));
		return;
	}
	
	std::istringstream iss(buffer);
	std::string chName, newTopic;
	iss >> chName;
	bool settingTopic = (bool)std::getline(iss >> std::ws, newTopic);

	Channel *channel = _getChannel(chName);
	if (!channel)
	{
		_sendResponse(fd, ERR_NOSUCHCHANNEL(_getHostname(), client->getNickname(), chName));
		return;
	}
	if (!channel->hasClient(client))
	{
		_sendResponse(fd, ERR_NOTONCHANNEL(_getHostname(), client->getNickname(), chName));
		return;
	}
	
	if (!settingTopic || newTopic.empty())
	{
		if (channel->getTopic().empty())
			_sendResponse(fd, RPL_NOTOPIC(_getHostname(), client->getNickname(), chName));
		else
			_sendResponse(fd, RPL_TOPIC(_getHostname(), client->getNickname(), chName, channel->getTopic()));
		return;
	}
	
	if (channel->getTopicRestricted() && !channel->isOperator(client))
	{
		_sendResponse(fd, ERR_CHANOPRIVSNEEDED(_getHostname(), client->getNickname(), chName));
		return;
	}
	
	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic.erase(0, 1);
	
	channel->setTopic(newTopic);
	std::string topicMsg = client->getPrefix() + " TOPIC " + chName + " :" + newTopic + "\r\n";
	_broadcastToChannel(chName, topicMsg, -1);
}

void Server::_handlerClientInvite(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;
	if (!client->getIsLogged())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), client->getNickname(), "INVITE"));
		return;
	}

	std::istringstream iss(buffer);
	std::string targetNick, chName;
	iss >> targetNick >> chName;
	if (targetNick.empty() || chName.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), client->getNickname(), "INVITE"));
		return;
	}
	
	Channel *channel = _getChannel(chName);
	if (!channel)
	{
		_sendResponse(fd, ERR_NOSUCHCHANNEL(_getHostname(), client->getNickname(), chName));
		return;
	}
	if (!channel->hasClient(client))
	{
		_sendResponse(fd, ERR_NOTONCHANNEL(_getHostname(), client->getNickname(), chName));
		return;
	}
	if (channel->getInviteOnly() && !channel->isOperator(client))
	{
		_sendResponse(fd, ERR_CHANOPRIVSNEEDED(_getHostname(), client->getNickname(), chName));
		return;
	}
	
	Client *target = _getClient(targetNick);
	if (!target)
	{
		_sendResponse(fd, ERR_NOSUCHNICK(_getHostname(), client->getNickname(), targetNick));
		return;
	}
	if (channel->hasClient(target))
	{
		_sendResponse(fd, ERR_USERONCHANNEL(_getHostname(), client->getNickname(), targetNick, chName));
		return;
	}

	channel->addInvitedUser(target);
	_sendResponse(fd, RPL_INVITING(_getHostname(), client->getNickname(), targetNick, chName));
	_sendResponse(target->getFd(), client->getPrefix() + " INVITE " + targetNick + " " + chName + "\r\n");
}

void Server::_handlerClientPrivmsg(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client)
		return;
	if (!client->getIsLogged())
	{
		_sendResponse(fd, ERR_NOTREGISTERED(_getHostname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NORECIPIENT(_getHostname(), client->getNickname()));
		return;
	}
	
	std::istringstream iss(buffer);
	std::string target, text;
	iss >> target;
	std::getline(iss >> std::ws, text);
	if (!text.empty() && text[0] == ':')
		text.erase(0, 1);
	if (target.empty())
	{
		_sendResponse(fd, ERR_NORECIPIENT(_getHostname(), client->getNickname()));
		return;
	}
	if (text.empty())
	{
		_sendResponse(fd, ERR_NOTEXTTOSEND(_getHostname(), client->getNickname()));
		return;
	}
	
	std::string outMsg = client->getPrefix() + " PRIVMSG " + target + " :" + text + "\r\n";
	
	if (target[0] == '#')
	{
		Channel *channel = _getChannel(target);
		if (!channel)
		{
			_sendResponse(fd, ERR_NOSUCHCHANNEL(_getHostname(), client->getNickname(), target));
			return;
		}
		if (!channel->hasClient(client))
		{
			_sendResponse(fd, ERR_NOTONCHANNEL(_getHostname(), client->getNickname(), target));
			return;
		}
		_broadcastToChannel(target, outMsg, fd);
	}
	else
	{
		Client *dest = _getClient(target);
		if (!dest)
		{
			_sendResponse(fd, ERR_NOSUCHNICK(_getHostname(), client->getNickname(), target));
			return;
		}
		_sendResponse(dest->getFd(), outMsg);
	}
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
		_broadcastToAll(oldPrefix + " NICK :" + nick + "\r\n", -1);
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
	Client *client = _getClient(fd);
	if (!client)
		return;
	if (client->getIsLogged())
	{
		_sendResponse(fd, ERR_ALREADYREGISTERED(_getHostname(), client->getNickname()));
		return;
	}
	if (buffer.empty())
	{
		_sendResponse(fd, ERR_NEEDMOREPARAMS(_getHostname(), "*", "PASS"));
		return;
	}
	
	std::string pass = buffer;
	if (!pass.empty() && pass[0] == ':')
		pass.erase(0, 1);
	if (pass != this->_password)
	{
		_sendResponse(fd, ERR_PASSWDMISMATCH(_getHostname()));
		_removeClientFromServer(fd);
		return;
	}
	client->setIsAuthenticated(true);
}

void Server::_handlerClientWho(const std::string &buffer, const int fd)
{
	Client *client = _getClient(fd);
	if (!client || !client->getIsLogged())
		return;
	
	std::string mask = buffer;
	if (mask.empty())
		mask = "*";
	if (!mask.empty() && mask[0] == ':')
		mask.erase(0, 1);
	if (mask[0] == '#')
	{
		Channel *channel = _getChannel(mask);
		if (channel)
		{
			std::vector<Client *> members = channel->getMembers();
			for (size_t i = 0; i < members.size(); i++)
			{
				std::string flags = channel->isOperator(members[i]) ? "H@" : "H";
				_sendResponse(fd, RPL_WHOREPLY(
					_getHostname(),
					client->getNickname(),
					mask,
					members[i]->getUsername(),
					members[i]->getIpAddr(),
					_getHostname(),
					members[i]->getNickname(),
					flags,
					members[i]->getRealname()
				));
			}
		}
	}
	else
	{
		for (size_t i = 0; i < _clients.size(); i++)
		{
			if (mask == "*" || _clients[i]->getNickname() == mask)
			{
				_sendResponse(fd, RPL_WHOREPLY(
					_getHostname(),
					client->getNickname(),
					"*",
					_clients[i]->getUsername(),
					_clients[i]->getIpAddr(),
					_getHostname(),
					_clients[i]->getNickname(),
					"H",
					_clients[i]->getRealname()
				));
			}
		}
	}
	_sendResponse(fd, RPL_ENDOFWHO(_getHostname(), client->getNickname(), mask));
}

void Server::_handlerClientParrot(const std::string &buffer, const int fd)
{
	(void)buffer;
	Client *client = _getClient(fd);
	if (!client)
		return;
	_sendResponse(fd, RPL_PARROT(_getHostname(), client->getNickname()));
}
