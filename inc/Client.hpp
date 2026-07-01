/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 11:20:40 by mamaratr          #+#    #+#             */
/*   Updated: 2026/07/01 11:55:03 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "utils.hpp"

class Client
{
	private:
		int _fd;
		std::string _ipAddr;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _buffer;
		bool _isAuthenticated;
		bool _isLogged;

	public:
		Client(int fd);
		~Client();

		int					getFd() const;
		const std::string&	getIpAddr() const;
		const std::string&	getNickname() const;
		const std::string&	getUsername() const;
		const std::string&	getRealname() const;
		const std::string&	getBuffer() const;
		bool				getIsAuthenticated() const;
		bool				getIsLogged() const;
		
		void setIpAddr(const std::string &ip);
		void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setRealname(const std::string &realname);
		void setIsAuthenticated(bool val);
		void setIsLogged(bool val);
	
		void appendToBuffer(const std::string &data);
		void clearBuffer();

		std::string getPrefix() const;
};

#endif