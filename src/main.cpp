/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:55:38 by rcarpio-          #+#    #+#             */
/*   Updated: 2026/05/27 10:42:07 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
		return (std::cout << "Usage: ./ircserv <port> <password>" << std::endl, 1);
	if (std::atoi(argv[1]) < 1024 || std::atoi(argv[1]) > 65535)
		return (std::cout << "Error: Port number must be between 1024 and 65535" << std::endl, 1);
	// Else initialize server
}