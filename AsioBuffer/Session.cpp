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
                _readBuf.WriteCompleted(length);

                uint8_t* base = _readBuf.GetReadPointer(); // Get read pointer before advancing
                _readBuf.ReadCompleted(length);

                self->Send(CommandType::Battle, base, length);

                std::cout << "do_read: length: " << length << std::endl;
                do_read();
            }
            else
            {
                do_close();
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
    //buf.Write(reinterpret_cast<const char*>(&type), sizeof(type));
    buf.Write(msg, len); // Write message payload
    _writeQueue.push(std::move(buf));

    do_write();
}
