/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 10:42:46 by mamaratr          #+#    #+#             */
/*   Updated: 2026/05/27 11:29:39 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Server
{
	private:
		int _port;
		std::string _password;
		std::vector<Client> _clients;
		std::vector<Channel> _channels;

	public:
		Server();
		Server(const Server &copy);
		Server &operator=(const Server &assign);
		~Server();

		void init(const int port, const std::string &password);
};

#endif