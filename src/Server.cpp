/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarpio- <rcarpio-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 10:31:47 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/03 18:32:20 by rcarpio-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
	_port = std::stoi(port);
	_password = password;
	_hostname = "localhost";
	std::cout << "Server initialization" << std::endl;

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
	

	
}
