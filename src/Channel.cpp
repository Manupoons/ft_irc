/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 10:31:42 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/23 14:47:54 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Channel.hpp"
#include <algorithm>

Channel::Channel(const std::string &name) :
	_name(name),
	_topic(""),
	_password(""),
	_numberUsers(0),
	_inviteOnly(false),
	_topicRestricted(false),
	_passwordRequired(false),
	_maxUsers(0)
{
	std::cout << "Channel created" << std::endl;
}

Channel::~Channel()
{
	std::cout << "Channel destroyed" << std::endl;
}

void Channel::addClient(Client* client)
{
	if (client && !hasClient(client))
	{
		_members.push_back(client);
		_numberUsers++;
	}
}

void Channel::removeClient(Client* client)
{
	if (client)
	{
		std::vector<Client*>::iterator it = std::find(_members.begin(), _members.end(), client);
		if (it != _members.end())
		{
			_members.erase(it);
			_numberUsers--;
		}
	}
}

bool Channel::hasClient(Client* client) const
{
	return std::find(_members.begin(), _members.end(), client) != _members.end();
}

void Channel::broadcast(const std::string& message, Client* sender) const
{
	for (std::vector<Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (*it != sender)
		{
			(*it)->sendMessage(message);
			std::cout << "Broadcasting message to " << (*it)->getNickname() << ": " << message << std::endl;
		}
	}
}

bool Channel::isOperator(Client* client) const
{
	return std::find(_operators.begin(), _operators.end(), client) != _operators.end();
}

void Channel::addOperator(Client* client)
{
	if (client && !isOperator(client))
	{
		_operators.push_back(client);
	}
}

void Channel::removeOperator(Client* client)
{
	if (client)
	{
		std::vector<Client*>::iterator it = std::find(_operators.begin(), _operators.end(), client);
		if (it != _operators.end())
		{
			_operators.erase(it);
		}
	}
}

bool Channel::checkJoinRules(const std::string& password) const
{
	if (_inviteOnly)
	{
		// Check if the client is invited
		if (std::find(_invitedUsers.begin(), _invitedUsers.end(), nullptr) == _invitedUsers.end())
		{
			return false;
		}
	}
	if (_passwordRequired && password != _password)
	{
		return false;
	}
	if (_maxUsers > 0 && _numberUsers >= _maxUsers)
	{
		return false;
	}
	return true;
}

void Channel::setMode(char mode, bool add, const std::string& arg)
{
	// Implement mode setting logic based on the mode character
	switch (mode)
	{
	case 'i':
		_inviteOnly = add;
		break;
	case 't':
		_topicRestricted = add;
		break;
	case 'k':
		if (add)
			_password = arg;
		else
			_password.clear();
		_passwordRequired = !arg.empty();
		break;
	case 'l':
		_maxUsers = add ? std::stoi(arg) : 0;
		break;
	default:
		break;
	}
}

std::string Channel::getModeString() const
{
	std::string modes = "+";
	if (_inviteOnly)
		modes += "i";
	if (_topicRestricted)
		modes += "t";
	if (!_password.empty())
		modes += "k";
	if (_numberUsers > 0)
		modes += "l";
	if (modes == "+")
		return "";
	return modes;
}

std::string Channel::getNickList() const
{
	std::string list;
	for (size_t i = 0; i < _members.size(); i++)
	{
		if (i > 0)
			list += " ";
		if (isOperator(_members[i]))
			list += "@";
		list += _members[i]->getNickname();
	}
	return list;
}

void Channel::removeInvited(Client *client)
{
	if (!client)
		return;
	std::vector<Client *>::iterator it = std::find(_invitedUsers.begin(), _invitedUsers.end(), client);
	if (it != _invitedUsers.end())
		_invitedUsers.erase(it);
}

bool Channel::isInvited(Client* client) const
{
	return std::find(_invitedUsers.begin(), _invitedUsers.end(), client) != _invitedUsers.end();
}

void Channel::addInvitedUser(Client* client)
{
	if (client && !isInvited(client))
	{
		_invitedUsers.push_back(client);
	}
}

int Channel::clientCount() const
{
	return static_cast<int>(_members.size());
}
