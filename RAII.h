#ifndef _WEBRTC_BASE_RAII_H_
#define _WEBRTC_BASE_RAII_H_

#include <gio/gio.h>
#include <glib.h>

#define DISABLE_COPY_CONSTRUCTION(X)                                           \
    X& operator=(const X&) = delete;                                           \
    X(const X&) = delete;

#define DISABLE_MOVE_CONSTRUCTION(X)                                           \
    X& operator=(X&&) = delete;                                                \
    X(X&&) = delete;

template<typename T>
class GUniqPtr
{
  public:
    GUniqPtr(T* obj = nullptr)
      : m_obj(obj)
    {}
    GUniqPtr(GUniqPtr&& other)
      : m_obj(other.m_obj)
    {
        other.m_obj = nullptr;
    }

    GUniqPtr& operator=(GUniqPtr&& other)
    {
        if (this != &other) {
            other.m_obj = m_obj;
            m_obj = nullptr;
        }

        return *this;
    }

    ~GUniqPtr()
    {
        if (m_obj != nullptr) {
            g_free(m_obj);
        }
    }

    T* get() const { return m_obj; }
    T& operator*() const noexcept { return *m_obj; }
    T* operator->() const noexcept { return m_obj; }

    T* release()
    {
        T* obj = m_obj;
        m_obj = nullptr;

        return obj;
    }

    void reset(T* obj = nullptr)
    {
        if (m_obj != nullptr) {
            g_free(m_obj);
        }
        m_obj = obj;
    }

  private:
    DISABLE_COPY_CONSTRUCTION(GUniqPtr);

    T* m_obj;
};

template<typename T,
         typename T_ARG,
         T_ARG* (*REF_FUNC)(T_ARG*),
         void (*UNREF_FUNC)(T_ARG*)>
class GRefCountedObject
{
  public:
    GRefCountedObject(T* obj = nullptr)
      : m_obj(obj)
    {}

    GRefCountedObject(const GRefCountedObject& other)
    {
        if (other.m_obj != nullptr) {
            m_obj = static_cast<T*>(REF_FUNC(static_cast<T_ARG*>(other.m_obj)));
        } else {
            m_obj = nullptr;
        }
    }

    GRefCountedObject(GRefCountedObject&& other)
      : m_obj(other.m_obj)
    {
        other.m_obj = nullptr;
    }

    GRefCountedObject& operator=(const GRefCountedObject& other)
    {
        if (this != &other) {
            if (other.m_obj != nullptr) {
                m_obj =
                  static_cast<T*>(REF_FUNC(static_cast<T_ARG*>(other.m_obj)));
            } else {
                m_obj = nullptr;
            }
        }

        return *this;
    }

    GRefCountedObject& operator=(GRefCountedObject&& other)
    {
        if (this != &other) {
            m_obj = other.m_obj;
            other.m_obj = nullptr;
        }

        return *this;
    }

    ~GRefCountedObject()
    {
        if (m_obj != nullptr) {
            UNREF_FUNC(m_obj);
        }
    }

    T* get() const { return m_obj; }
    T& operator*() const noexcept { return *m_obj; }
    T* operator->() const noexcept { return m_obj; }

    T* release()
    {
        T* obj = m_obj;
        m_obj = nullptr;

        return obj;
    }

    void reset(T* obj = nullptr)
    {
        if (m_obj != nullptr) {
            UNREF_FUNC(m_obj);
        }
        m_obj = obj;
    }

  private:
    T* m_obj;
};

using GObjectPtr =
  GRefCountedObject<GObject, void, g_object_ref, g_object_unref>;

using GBytesPtr = GRefCountedObject<GBytes, GBytes, g_bytes_ref, g_bytes_unref>;

using GKeyFilePtr =
  GRefCountedObject<GKeyFile, GKeyFile, g_key_file_ref, g_key_file_unref>;

using GIOChannelPtr = GRefCountedObject<GIOChannel,
                                        GIOChannel,
                                        g_io_channel_ref,
                                        g_io_channel_unref>;

using GSocketClientPtr =
  GRefCountedObject<GSocketClient, void, g_object_ref, g_object_unref>;

using GSocketServicePtr =
  GRefCountedObject<GSocketService, void, g_object_ref, g_object_unref>;

using GSocketConnectionPtr =
  GRefCountedObject<GSocketConnection, void, g_object_ref, g_object_unref>;

class Mutex
{
  public:
    Mutex() { g_mutex_init(&m_mutex); }
    ~Mutex() { g_mutex_clear(&m_mutex); }

    void lock() { g_mutex_lock(&m_mutex); }
    void unlock() { g_mutex_unlock(&m_mutex); }

  private:
    DISABLE_COPY_CONSTRUCTION(Mutex);
    DISABLE_MOVE_CONSTRUCTION(Mutex);

    GMutex m_mutex;
};

class LockGuard
{
  public:
    LockGuard(Mutex& mutex)
      : m_mutex(&mutex)
    {
        m_mutex->lock();
    }
    ~LockGuard() { m_mutex->unlock(); }

    LockGuard(LockGuard&& other)
      : m_mutex(other.m_mutex)
    {
        other.m_mutex = nullptr;
    }
    LockGuard& operator=(LockGuard&& other)
    {
        if (this != &other) {
            m_mutex = other.m_mutex;
            other.m_mutex = nullptr;
        }

        return *this;
    }

  private:
    DISABLE_COPY_CONSTRUCTION(LockGuard);

    Mutex* m_mutex;
};

#endif /* _WEBRTC_BASE_RAII_H_ */
