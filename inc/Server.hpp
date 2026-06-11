/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamaratr <mamaratr@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 10:42:46 by mamaratr          #+#    #+#             */
/*   Updated: 2026/06/11 11:13:53 by mamaratr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Replies.hpp"

class Server
{
	private:
		struct commandHandler
		{
			std::string cmd;
			void (Server::*handler)(const std::string &, const int);
		};

		static const int _commandListSize = 13;
		static const commandHandler _commandList[_commandListSize];
	
		int						_port;
		std::string				_password;
		int 					_serverSocket; 
		std::string				_hostname;
		std::vector<Client *>	_clients;
		std::vector<Channel *>	_channels;
		struct sockaddr_in		_confSocket;
		int						_replyCode;
		std::vector<struct pollfd>	_fds;
		static bool 			_signal;

		//¬¬¬¬ Validation ¬¬¬¬
		void _isValidPort(const std::string &port);
		bool _isClientInAnyChannel(const int fd);
		bool _clientIsReadyToLogin(const int fd);
		bool _isValidNickname(const std::string &nickname);
		bool _isNicknameInUse(const int fd, const std::string &nickname);

		//¬¬¬¬ Command Handlers ¬¬¬¬
		void _handlerClientCap(const std::string &buffer, const int fd);
		void _handlerClientJoin(const std::string &buffer, const int fd);
		void _handlerClientQuit(const std::string &buffer, const int fd);
		void _handlerClientPart(const std::string &buffer, const int fd);
		void _handlerClientMode(const std::string &buffer, const int fd);
		void _handlerClientKick(const std::string &buffer, const int fd);
		void _handlerClientTopic(const std::string &buffer, const int fd);
		void _handlerClientInvite(const std::string &buffer, const int fd);
		void _handlerClientPrivmsg(const std::string &buffer, const int fd);
		void _handlerClientNickname(const std::string &buffer, const int fd);
		void _handlerClientUsername(const std::string &buffer, const int fd);
		void _handlerClientPassword(const std::string &buffer, const int fd);
		void _handlerClientWho(const std::string &buffer, const int fd);

		//¬¬¬¬ Socket / Loop ¬¬¬¬
		static void _signalHandler(const int signum);
		void		_closeFds();
		void		_serverLoop();
		void		_setServerSocket();
		void		_addServerSignal();
		void		_acceptNewClient();
		void		_clearClient(const int fd);
		void		_receiveNewData(const int fd);
		void		_sendResponse(const int fd, const std::string &response);

		//¬¬¬¬ Parsing ¬¬¬¬
		void						_executeCommand(const std::string buffer, const int fd);
		std::string					_cleanseBuffer(const std::string &buffer, const std::string &chars_to_remove);
		std::vector<std::string>	_splitBuffer(const std::string &buffer, const std::string &delimiter);

		//¬¬¬¬ Lookups ¬¬¬¬
		Client*		_getClient(const int fd);
		Client*		_getClient(const std::string nickname);
		Channel*	_getChannel(const std::string &channel_name);
		void		_addChannel(Channel *channel);
		void		_removeEmptyChannels();

		//¬¬¬¬ Client cycle ¬¬¬¬
		void _removeClientFd(const int fd);
		void _removeClientFromChannel(const int fd);
		void _removeClientFromServer(const int fd);
		void _sendWelcome(Client *client);

		//¬¬¬¬ Broadcast ¬¬¬¬
		void _broadcastToAll(const std::string &msg, int exclude_fd = -1);
		void _broadcastToChannel(const std::string &channelName, const std::string &msg, int exclude_fd = -1);

		//¬¬¬¬ Hostname ¬¬¬¬
		std::string _getHostname();
		
	public:
		Server();
		~Server();

		void init(const std::string &port, const std::string &password);
};

std::string toupper(const std::string &str);

#endif