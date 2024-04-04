#ifndef APP_PTR_H
#define APP_PTR_H

// std
#include <atomic>
#include <utility>

namespace app355 {

// UNIQUE POINTER
template<typename T>
class unique_ptr
{
public:
    unique_ptr() = default;
    unique_ptr(T* inData);
    ~unique_ptr();

    void reset();
    T* release();
    T* get();

    unique_ptr(unique_ptr&& ioMove);
    unique_ptr& operator=(unique_ptr&& ioMove);
    unique_ptr(unique_ptr& inCopy) = delete;
    unique_ptr& operator=(unique_ptr& inCopy) = delete;

    T* operator->();
    T& operator*();
    operator bool();

private:
    T* mData{};
}; // class unique_ptr<T>

#pragma region unique_ptr

template<typename T>
inline unique_ptr<T>::unique_ptr(T* inData) : 
    //Beware: ControlBlock default initializes mStrongCount to 1
    mData{inData}
{
    // Do Nothing
}

template<typename T>
inline unique_ptr<T>::~unique_ptr()
{
    reset();
}

template<typename T>
inline void unique_ptr<T>::reset()
{
    // delete is smart enough to do nothing when passed nullptr
    delete mData;
    mData = nullptr;
}

template<typename T>
inline T* unique_ptr<T>::release()
{
    T* data = mData;
    mData = nullptr;
    return data;
}

template<typename T>
inline unique_ptr<T>::unique_ptr(unique_ptr&& ioMove)
{
    reset();
    mData = ioMove.release();
}

template<typename T>
inline unique_ptr<T>& unique_ptr<T>::operator=(unique_ptr&& ioMove)
{
    // Moving to self should be a NOP
    if (this != &ioMove)
    {
        reset();
        mData = ioMove.release();
    }
    return *this;
}

template<typename T>
inline T* unique_ptr<T>::operator->()
{
    return get();
}

template<typename T>
inline T& unique_ptr<T>::operator*()
{
    return *get();
}

template<class T> 
inline unique_ptr<T>::operator bool()
{
    return (mData != nullptr);
}

template<typename T>
inline T* unique_ptr<T>::get()
{
    return mData;
}
#pragma endregion // unique_ptr


// TRICKY: Variatic template arguments 
// Template that takes any number and type of arguments
template<typename T, typename ...Args>
unique_ptr<T> make_unique(Args... inArgs)
{
    T* data = new T{std::forward<Args>(inArgs)...};
    unique_ptr<T> unique{data};
    return unique;
}

//
/////////////////////////////////////////////////////////////////////////////////
//

// SHARED POINTER
template<typename T>
class shared_ptr
{
public:
    shared_ptr() = default;

    shared_ptr(T* inData) :
        mData{inData},
        mControl{new ControlBlock{inData}}
    {
        // Note: ControlBlock::Ctor{} sets strong count to 1
    }

    ~shared_ptr()
    {
        reset();
    }

    void reset()
    {
        // Cannot check the count if mControl no longer exists
        if (mControl != nullptr)
        {
            auto [strong, weak] = mControl->DecrementStrong();
            if (strong <= 0)
            {
                if (mData != nullptr)
                {
                    delete mData;
                }

                if (weak <= 0)
                {
                    delete mControl;
                }
            }
        }
        mData = nullptr;
        mControl = nullptr;
    }

    // RO5 Methods
    // Move Ctor
    shared_ptr(shared_ptr&& ioMove) : 
        mData{ioMove.mData},
        mControl{ioMove.mControl}
    {
        ioMove.mData = nullptr;
        ioMove.mControl = nullptr;
    }

    // Move Operand
    shared_ptr& operator=(shared_ptr&& ioMove)
    {
        // Moving to self should be a NOP
        if (this != &ioMove)
        {
            std::swap(mData, ioMove.mData);
            std::swap(mControl, ioMove.mControl);
        }
        return *this;
    }

    // Copy Ctor
    shared_ptr(const shared_ptr& inCopy)
    {
        mData = inCopy.mData;
        mControl = inCopy.mControl;
        mControl->IncrementStrong();
    }

    // Copy Operand
    shared_ptr& operator=(const shared_ptr& inCopy)
    {
        // Copying to self should be a NOP
        if (this != &inCopy)
        {
            mData = inCopy.mData;
            mControl = inCopy.mControl;
            mControl->IncrementStrong();
        }
        return *this;
    }

    T* operator->()
    {
        return get();
    }

    T& operator*()
    {
        return *get();
    }

    operator bool() const
    {
        return (mData != nullptr);
    }

    T* get()
    {
        return mData;
    }

private:
    class ControlBlock
    {
    public:
        ControlBlock(T* inData) :  
                mData{inData}
        {
            // Do nothing
        }

        // RO5 Methods added
        ~ControlBlock() = default;

        T* get()
        {
            return mData;
        }

        std::pair<std::size_t, std::size_t> IncrementStrong()
        {
            std::lock_guard<std::mutex> lock{mMutex};
            auto refCounts = std::make_pair(++mStrongCount, mWeakCount);
            return refCounts;
        }

        std::pair<std::size_t, std::size_t> DecrementStrong()
        {
            std::lock_guard<std::mutex> lock{mMutex};
            auto refCounts = std::make_pair(--mStrongCount, mWeakCount);
            return refCounts;
        }

        std::pair<std::size_t, std::size_t> IncrementWeak()
        {
            std::lock_guard<std::mutex> lock{mMutex};
            auto refCounts = std::make_pair(mStrongCount, ++mWeakCount);
            return refCounts;
        }

        std::pair<std::size_t, std::size_t> DecrementWeak()
        {
            std::lock_guard<std::mutex> lock{mMutex};
            auto refCounts = std::make_pair(mStrongCount, --mWeakCount);
            return refCounts;
        }

    private:
        std::size_t mStrongCount{1}; // Default constructs with initial refcount of 1
        std::size_t mWeakCount{0};
        std::mutex mMutex{};
        T* mData{}; // Used by weak pointers to obtain a new shared pointer
    };              // class ControlBlock

private:
    template <typename T>
    friend class weak_ptr;

    shared_ptr(ControlBlock* inBlock)
    {
        mData = inBlock->get();
        mControl = inBlock;
        mControl->IncrementStrong();
    }

private:
    T* mData{};
    ControlBlock* mControl{};
}; // class shared_ptr<T>

// TRICKY: Variatic template arguments 
// Template that takes any number and type of arguments
template<typename T, typename ...Args>
shared_ptr<T> make_unique(Args... inArgs)
{
    T* data = new T{std::forward<Args>(inArgs)...};
    shared_ptr<T> unique{data};
    return unique;
}

// WEAK POINTER
template <typename T>
class weak_ptr
{
public:
    weak_ptr() : 
        mControl{nullptr}
    {

    }

    weak_ptr(shared_ptr<T>& inPtr)
    {
        if (inPtr)
        {
            mControl = inPtr.mControl;
            mControl->IncrementWeak();
        }
        // Note: ControlBlock::Ctor{} sets strong count to 1
    }

    ~weak_ptr()
    {
        reset();
    };

    void reset()
    {
        if (mControl != nullptr)
        {
            auto [strong, weak] = mControl->DecrementWeak();
            if (strong <= 0)
            {
                // delete data not available for weak_ptr
                if (weak <= 0)
                {
                    delete mControl;
                }
            }
        }
        mControl = nullptr;
    }

    // RO5 Methods
    // Shared Assign
    weak_ptr& operator=(shared_ptr<T>& inShared)
    {
        // Copying to self should be a NOP
        if (this != &inShared)
        {
            mControl = inShared.mControl;
            mControl->IncrementWeak();
        }
        return *this;
    }

    // Move Ctor
    weak_ptr (weak_ptr&& ioMove)
    {
        std::swap(mControl, ioMove.mControl);
    }

    // Move Operand
    weak_ptr& operator=(weak_ptr&& ioMove)
    {
        // Moving to self should be a NOP
        if (this != &ioMove)
        {
            std::swap(mControl, ioMove.mControl);
        }
        return *this;
    }

    // Copy Ctor
    weak_ptr (const weak_ptr& inCopy)
    {
        mControl = inCopy.mControl;
        mControl->IncrementStrong();
    }

    // Copy Operand
    weak_ptr& operator=(const weak_ptr& inCopy)
    {
        // Copying to self should be a NOP
        if (this != &inCopy)
        {
            mData = inCopy.mData;
            mControl = inCopy.mControl;
            mControl->IncrementStrong();
        }
        return *this;
    }

    shared_ptr<T> lock()
    {
        shared_ptr<T> lockedPtr{mControl};
        return lockedPtr;
    }

    operator bool() const
    {
        return (mControl != nullptr);
    }

    T* operator->()
    {
        return get();
    }

    T& operator*()
    {
        return *get();
    }

    T* get()
    {
        return mControl.get();
    }

private:
    // TRICKY: "Nested types" from a template class must be prefixed with the keyword 'typename'
    typename shared_ptr<T>::ControlBlock* mControl{};
}; // class shared_ptr<T>

} // namespace app355
#endif // APP_PTR_H