#include "Session.h"

Session::Session(asio::io_context& ioc)
    : _socket(ioc)
    , _is_close(false)
    , _is_writeAsync(false)
{}

Session::~Session()
{
    do_close();
}

tcp::socket& Session::Socket()
{
    return _socket;
}

void Session::start()
{
    do_read();
}

void Session::do_read()
{
    _readBuf.Normalize();
    _readBuf.EnsureFreeSpace();
    auto self(shared_from_this());
    Socket().async_read_some(asio::buffer(_readBuf.GetWritePointer(), _readBuf.GetRemainingSpace()),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec)
            {
                _readBuf.WriteCompleted(length); // 写指针后移，更新缓冲区
                if (length < sizeof(uint16_t))
                {
                    // 如果小于两个字节，说明还没有将完整的数据包头读取到，继续读取
                    do_read();
                }
                else
                {
                    while (_readBuf.GetActiveSize() > 0)
                    {
                        // 大于两个字节，说明已经读取到了数据包的长度，开始处理，先读取前两个字节的数据包长度
                        uint16_t len;
                        memcpy(&len, _readBuf.GetReadPointer(), sizeof(len));
                        len = asio::detail::socket_ops::network_to_host_short(len); // 将网络字节序转换为主机字节序
                        std::cout << len << "length : " << length << std::endl;

                        if (length < 2 + len)
                        {
                            break; // 说明读取小于总长度，继续读取
                        }
                        else
                        {
                            // 说明读取长度大于等于总长度，说明已经读取到完整的数据包，还可能多读取了
                            uint8_t* base = _readBuf.GetReadPointer();
                            _readBuf.ReadCompleted(len + 2);
                            self->Send(CommandType::Battle, base, len + 2);
                            //std::cout << "_readBuf ptr" << _readBuf.GetActiveSize() << std::endl;
                        }
                        if (len + 2 > _readBuf.GetRemainingSpace()) // 如果数据包长度大于缓冲区剩余空间，说明缓冲区不够用
                        {
                            _readBuf.Normalize();
                            _readBuf.EnsureFreeSpace();
                        }
                        // 如果数据包长度小于等于缓冲区剩余空间，说明缓冲区够用
                            // 考虑读取的数据包长度小于等于长度，说明此时没有多读数据
                        if (length > 2 + len) // 读取长度 说明多读了
                        {

                        }
                    }
                    do_read();
                    
                }
            }
            else
            {
                do_close();
                return;
            }
        }
    );
}

void Session::do_write()
{
    if (_is_writeAsync) return; // Prevent multiple async writes
    if (_writeQueue.empty()) return; // Ensure queue is not empty

    _is_writeAsync = true;
    auto self(shared_from_this());
    auto& buf = _writeQueue.front();

    if (!buf.GetActiveSize())
    {
        _is_writeAsync = false;
        return; // No data to write
    }

    Socket().async_write_some(asio::buffer(buf.GetReadPointer(), buf.GetActiveSize()),
        [this, self, &buf](boost::system::error_code ec, std::size_t length) {
            if (ec)
            {
                // Close connection on failure
                _is_writeAsync = false;
                do_close();
                return;
            }
            buf.ReadCompleted(length); // Advance buffer
            if (!buf.GetActiveSize())
            {
                _writeQueue.pop(); // Remove completed buffer
            }

            _is_writeAsync = false; // Ready for the next write
            if (!_writeQueue.empty())
            {
                do_write(); // Process next buffer
            }
        }
    );
}

void Session::do_close()
{
    if (!_is_close)
    {
        _is_close = true;
        std::cout << "Closing connection!" << std::endl;
        boost::system::error_code err;
        _socket.close(err);
    }
}

void Session::Send(const CommandType type, const uint8_t* msg, std::size_t len)
{
    MessageBuffer buf(len);
    buf.Write(msg, 2);
    buf.Write(msg + 2, len - 2);
    _writeQueue.push(std::move(buf));
    do_write();
}
