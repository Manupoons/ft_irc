/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarpio-cyepes <rcarpio-cyepes@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 10:31:44 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/30 04:33:07 by rcarpio-cye      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"

Client::Client(int fd) : 
	_fd(fd),
	_ipAddr(""),
	_nickname(""),
	_username(""),
	_realname(""),
	_buffer(""),
	_isAuthenticated(false),
	_isLogged(false)
{}

Client::~Client() {}

//----Getters----

int Client::getFd() const { return _fd; }

const std::string& Client::getIpAddr() const { return _ipAddr; }

const std::string& Client::getNickname() const { return _nickname; }

const std::string& Client::getUsername() const { return _username; }

const std::string& Client::getRealname() const { return _realname; }

const std::string& Client::getBuffer() const { return _buffer; }

bool Client::getIsAuthenticated() const { return _isAuthenticated; }


//----Setters----

void Client::setIpAddr(const std::string &ip) { _ipAddr = ip; }

void Client::setNickname(const std::string &nickname) { _nickname = nickname; }

void Client::setUsername(const std::string &username) { _username = username; }

void Client::setRealname(const std::string &realname) { _realname = realname; }

void Client::setIsAuthenticated(bool val) { _isAuthenticated = val; }

void Client::setIsLogged(bool val) { _isLogged = val; }

//----Buffer----

void Client::appendToBuffer(const std::string &data) { _buffer.append(data); }

void Client::clearBuffer() { _buffer.clear(); }

std::string Client::getPrefix() const
{
	std::string nick = _nickname.empty() ? "*" : _nickname;
	std::string user = _username.empty() ? "*" : _username;
	std::string host = _ipAddr.empty() ? "localhost" : _ipAddr;
	return ":" + nick + "!" + user + "@" + host;
}

void Client::sendMessage(const std::string &message)
{
	if (send(_fd, message.c_str(), message.size(), 0) == -1)
		std::cerr << "Error sending message to " << _nickname << std::endl;
}

bool Client::getIsLogged() const
{
    return _isLogged; 
}