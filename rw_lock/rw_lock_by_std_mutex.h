#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

namespace zone
{
    class read_write_mutex
    {
    public:
        read_write_mutex() = default;
        ~read_write_mutex() = default;

        read_write_mutex(const read_write_mutex &) = delete;
        read_write_mutex & operator=(const read_write_mutex &) = delete;

        read_write_mutex(const read_write_mutex &&) = delete;
        read_write_mutex & operator=(const read_write_mutex &&) = delete;

        void lock_read() {
            std::unique_lock<std::mutex> lock( m_mutex );
            m_cond_read.wait(lock, [this]()-> bool {
                    return m_write_count == 0;
                });
            ++m_read_count;
        }

        void unlock_read() {
            std::unique_lock<std::mutex> lock( m_mutex );
            if (--m_read_count == 0 && m_write_count > 0) {
                m_cond_write.notify_one();
            }
        }

        void lock_write() {
            std::cout << "(lock_write) thread_id: " << std::this_thread::get_id() << std::endl;
            std::unique_lock<std::mutex> lock( m_mutex );
            std::cout << "(lock_write1) thread_id: " << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            ++m_write_count;
            m_cond_write.wait(lock, [this]()-> bool {
                 std::cout << "(predicate) thread_id: " << std::this_thread::get_id() << std::endl;
                    return m_read_count == 0 && !m_writing;
                });
            m_writing = true;
        }

        void unlock_write() {
            std::cout << "(unlock_write) thread_id: " << std::this_thread::get_id() << std::endl;
            std::unique_lock<std::mutex> lock( m_mutex );
            std::cout << "(unlock_write1) thread_id: " << std::this_thread::get_id() << std::endl;
            if (--m_write_count == 0) {
                m_cond_read.notify_all();
            } else {
                m_cond_write.notify_one();
            }
            m_writing = false;
        }

    private:
        volatile size_t m_read_count = 0;
        volatile size_t m_write_count = 0;
        volatile bool m_writing = false;
        std::mutex m_mutex;
        std::condition_variable m_cond_read;
        std::condition_variable m_cond_write;
    };



    template<typename _ReadWriteLock>
    class unique_read_lock
    {
    public:
        explicit unique_read_lock(_ReadWriteLock & rwLock)
            : m_ptr_rw_lock(&rwLock) {
            m_ptr_rw_lock->lock_read();
        }

        ~unique_read_lock() {
            if (m_ptr_rw_lock) {
                m_ptr_rw_lock->unlock_read();
            }
        }

        unique_read_lock() = delete;
        unique_read_lock(const unique_read_lock &) = delete;
        unique_read_lock & operator = (const unique_read_lock &) = delete;
        unique_read_lock(const unique_read_lock &&) = delete;
        unique_read_lock & operator = (const unique_read_lock &&) = delete;

    private:
        _ReadWriteLock * m_ptr_rw_lock = nullptr;
    };



    template<typename _ReadWriteLock>
    class unique_write_lock
    {
    public:
        explicit unique_write_lock(_ReadWriteLock & rwLock)
            : m_ptr_rw_lock(&rwLock) {
            m_ptr_rw_lock->lock_write();
        }

        ~unique_write_lock() {
            if (m_ptr_rw_lock) {
                m_ptr_rw_lock->unlock_write();
            }
        }

        unique_write_lock() = delete;
        unique_write_lock(const unique_write_lock &) = delete;
        unique_write_lock & operator = (const unique_write_lock &) = delete;
        unique_write_lock(const unique_write_lock &&) = delete;
        unique_write_lock & operator = (const unique_write_lock &&) = delete;

    private:
        _ReadWriteLock * m_ptr_rw_lock = nullptr;
    };
}