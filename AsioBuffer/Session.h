#pragma once
#include "common.h"
#include "MessageBuffer.h"
enum CommandType : uint8_t
{
	Close = 0x00,
	Battle = 0x01
};

// 会话类管理这个socket
class Session : public std::enable_shared_from_this<Session>
{
public:
	explicit Session(asio::io_context& ioc);
	~Session();
	tcp::socket& Socket();
	void start();
	
private:
	void do_read();
	void do_write();
	void do_close();
	void Send(const CommandType type, const uint8_t* msg, std::size_t len);
	tcp::socket _socket;
	MessageBuffer _readBuf;
	std::queue<MessageBuffer> _writeQueue;
	bool _is_close;
	bool _is_writeAsync; // 是否在异步发送
};