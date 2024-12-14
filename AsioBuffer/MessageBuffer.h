#pragma once
#include "common.h"
#include <cstring>
// 利用循环缓冲区进行数据的读写
/*
原理：
    当客户的发送io请求时，数据先会写入到服务器的缓冲区中，即_wpos指针会从0向后移动，表示有数据发送过来了，可以进行读取了，
    此时使用_rpos指针来表示读取的位置，当读取数据时，_rpos指针会向后移动，表示数据已经被读取了，
    当_wpos指针和_rpos指针相等时，表示缓冲区中没有数据了，此时需要等待客户端发送数据过来。
*/
class MessageBuffer
{
    typedef std::vector<uint8_t>::size_type size_type;

public:
    MessageBuffer() : _wpos(0), _rpos(0), _storage()
    {
        _storage.resize(1024); // 初始化分配4kb的空间
    }

    explicit MessageBuffer(std::size_t initialSize) : _wpos(0), _rpos(0), _storage()
    {
        _storage.resize(initialSize);
    }

    MessageBuffer(MessageBuffer const& right) : _wpos(right._wpos), _rpos(right._rpos), _storage(right._storage)
    {
    }

    MessageBuffer(MessageBuffer&& right) : _wpos(right._wpos), _rpos(right._rpos), _storage(right.Move()) { }

    void Reset() // 重置缓冲区
    {
        _wpos = 0;
        _rpos = 0;
    }

    void Resize(size_type bytes) // 调整缓冲区大小
    {
        _storage.resize(bytes);
    }

    uint8_t* GetBasePointer() { return _storage.data(); } // 获取缓冲区基地址
    uint8_t* GetReadPointer() { return GetBasePointer() + _rpos; } // 获取读取指针
    uint8_t* GetWritePointer() { return GetBasePointer() + _wpos; } // 获取写入指针
    // 从网络输出缓冲区中读取一部分数据
    void ReadCompleted(size_type bytes) { _rpos += bytes; } // 读取完成后更新读取指针
    // 写入数据到消息缓存区中
    void WriteCompleted(size_type bytes) { _wpos += bytes; } // 写入完成后更新写入指针
    size_type GetActiveSize() const { return _wpos - _rpos; } // 获取当前缓冲区中未发送的数据的大小
    size_type GetRemainingSpace() const { return _storage.size() - _wpos; } // 返回缓冲区中剩余空间的大小
    size_type GetBufferSize() const { return _storage.size(); } // 返回缓冲区大小

    // 将未读数据移动至缓冲区开头
    void Normalize()
    {
        if (_rpos)  // 如果 _rpos 非零（即存在读取过的数据）
        {
            if (_rpos != _wpos) // 如果 _rpos 和 _wpos 不相等（即缓冲区中还有未读取的数据）
            {
                /*
                    比如当前读指针是2，写指针是8，那么缓冲区还有6个字节未读取，那么将这个6个字节的数据移动到缓冲区的前面，
                    即将缓冲区的前2个字节的数据丢弃，然后更新读指针和写指针。
                    即memove("基址指针"，"读指针"，"未读取的数据大小")
                */
                memmove(GetBasePointer(), GetReadPointer(), GetActiveSize()); // 将未读取的数据移动到缓冲区的前面
            }
            _wpos -= _rpos; // 0 至 大小
            _rpos = 0; // 重置 _rpos 为 0
        }
    }

    // 确保缓冲区有足够的空闲空间，在调用 EnsureFreeSpace() 之前，请先调用 Normalize() 方法。
    // 需要提前使用Normalize()方法，因为Normalize()方法会将缓冲区的前部分只包含未读的数据，这样就可以确保缓冲区有足够的空闲空间。
    void EnsureFreeSpace()
    {
        // 如果缓冲区已满，则调整缓冲区大小
        if (GetRemainingSpace() == 0)
            _storage.resize(_storage.size() * 3 / 2);
    }

    void Write(void const* data, std::size_t size) // 当收到新数据时，调用 Write 方法将数据写入缓冲区。
    {
        if (size)
        {
            memcpy(GetWritePointer(), data, size);
            WriteCompleted(size); // 更新写入指针
        }
    }

    std::vector<uint8_t>&& Move() // 将缓冲区所有权转移给调用者
    {
        _wpos = 0;
        _rpos = 0;
        return std::move(_storage);
    }

    MessageBuffer& operator=(MessageBuffer const& right)
    {
        if (this != &right)
        {
            _wpos = right._wpos;
            _rpos = right._rpos;
            _storage = right._storage;
        }

        return *this;
    }

    MessageBuffer& operator=(MessageBuffer&& right)
    {
        if (this != &right)
        {
            _wpos = right._wpos;
            _rpos = right._rpos;
            _storage = right.Move();
        }

        return *this;
    }

    /************************************************\
    * 注意这里wpos是从内核态复制到用户态时，wpos需要改变
    * 当需要写入数据到缓存区时，即发送数据，需要提供rpos
    * 的位置来进行写入数据
    * 从内核态读取到的数据应该让wpos向后移动，此时wpos-
    * rpos才是读取到的数据 即GetActiveSize()
    \*************************************************/
private:
    size_type _wpos; // 表示当前写入数据的偏移量
    size_type _rpos; // 表示当前读取数据的偏移量
    std::vector<uint8_t> _storage; // 即动态分配的缓冲区，用来存储消息数据
};
