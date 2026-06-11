/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarpio-mamaratr <rcarpio-mamaratr@stud    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:55:38 by rcarpio-          #+#    #+#             */
/*   Updated: 2026/06/11 14:09:01 by rcarpio-mam      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include <iostream>

int main(int argc, char **argv)
{	// Else initialize server
	(void) argc;
	try
	{
		Server server;
		server.init(argv[1], argv[2]);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error initializing server: " << e.what() << std::endl;
		return 1;
	}	
}

static bool isValidPort(const std::string &port)
{
	for (size_t i = 0; i < port.length(); ++i)
	{
		if (!isdigit(port[i]))
			return false;
	}
	int portNum = std::atoi(port.c_str());
	return (portNum >= 1024 && portNum <= 65535);
}
