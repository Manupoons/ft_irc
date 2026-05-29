/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 11:20:40 by mamaratr          #+#    #+#             */
/*   Updated: 2026/05/29 11:37:26 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "utils.hpp"

class Client
{
	private:
		int _socket;
		std::string _nickname;
		std::string _username;
		bool _isRegistered;
		std::string _buffer;

	public:
		Client();
		//Client(const Client &copy);
		//Client &operator=(const Client &assign);
		~Client();
};

#endif