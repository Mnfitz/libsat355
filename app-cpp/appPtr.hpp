#ifndef APP_PTR_H
#define APP_PTR_H

// std
#include <atomic>
#include <utility>

namespace app355 {

namespace detail {

/// @brief ControlBlock is responsible for managing the lifetime 
    /// of a shared object, and holds 2 counts: mStrongCount and mWeakCount. 
    /// mStrongCount relates to the number of shared_ptrs with ownership 
    /// of the data, where mWeakCount refers to the weak_ptrs.
    template<typename T>
    class ControlBlock
    {
    public:
        /// @brief Constructs ControlBlock with template data type T
        /// @param inData data to be managed by controlblock
        ControlBlock(T* inData) :  
            mData{inData}
        {
            // Do nothing
        }

        // RO5 Methods added
        /// @brief dtor is default
        ~ControlBlock() = default;

        // Copying should not be done, because we use a std::mutex
        /// @brief Copies not allowed for ControlBlock
        ControlBlock(const ControlBlock& inCopy) = delete;
        /// @brief Copies not allowed for ControlBlock
        ControlBlock& operator=(const ControlBlock& inCopy) = delete;

        /// @brief Increments the strong refcount
        /// @return a pair of <mStrongCount, mWeakCount>
        std::pair<std::size_t, std::size_t> IncrementStrong()
        {
            std::lock_guard<std::mutex> lock{mMutex};
            const bool hasDataBeenDeleted = (mStrongCount <= 0);
            if (!hasDataBeenDeleted)
            {
                ++mStrongCount;
            }
            return {mStrongCount, mWeakCount};
        }

        /// @brief Decrements the strong refcount
        /// @return a pair of <mStrongCount, mWeakCount>
        std::pair<std::size_t, std::size_t> DecrementStrong()
        {
            std::lock_guard<std::mutex> lock{mMutex};
            // StrongCount is not allowed to change once it reaches 0 and must be deleted
            assert((mStrongCount > 0) && "Cannot decrement below zero");
            --mStrongCount;
            return {mStrongCount, mWeakCount};
        }

        /// @brief Increments the weak refcount
        /// @return a pair of <mStrongCount, mWeakCount>
        std::pair<std::size_t, std::size_t> IncrementWeak()
        {
            std::lock_guard<std::mutex> lock{mMutex};
            ++mWeakCount;
            return {mStrongCount, mWeakCount};
        }

        /// @brief Decrements the weak refcount
        /// @return a pair of <mStrongCount, mWeakCount>
        std::pair<std::size_t, std::size_t> DecrementWeak()
        {
            std::lock_guard<std::mutex> lock{mMutex};
            // StrongCount is not allowed to change once it reaches 0 and must be deleted
            assert((mWeakCount > 0) && "Cannot decrement below zero");
            --mWeakCount;
            return {mStrongCount, mWeakCount};
        }

        /// @brief Gets the data ControlBlock is managing
        /// @return mData
        T* get()
        {
            return mData;
        }

    private:
        std::size_t mStrongCount{1}; // Default constructs with initial strong refcount of 1
        std::size_t mWeakCount{0};
        std::mutex mMutex{};
        T* mData{nullptr}; // Used by weak pointers to obtain a new shared pointer
    }; // class ControlBlock

} // namespace app355::detail

#pragma region unique_ptr

/// @brief std::unique_ptr is a smart pointer that owns and manages a single object through 
/// a pointer and disposes of that object when the unique_ptr goes out of scope.
template<typename T>
class unique_ptr
{
public:
    /// @brief Default ctor creates a pointer with its mData as nullptr
    unique_ptr() = default;
    /// @brief Alt ctor creates a pointer with its mData assigned
    /// @param inData The data which will be stored in the unique_ptr
    unique_ptr(T* inData);
    /// @brief dtor resets the pointer
    ~unique_ptr();

    /// @brief Deletes and nulls out stored data
    void reset();
    /// @brief Surrenders control of the pointer then resets it
    /// @return Returns the stored data that was once held within unique_ptr
    T* release();
    /// @brief Returns the stored data without releasing control
    /// @return stored data
    T* get();

    /// @brief Initializes this with the released data from the input pointer
    /// @param ioMove unique_ptr which shall have its data released and transferred
    unique_ptr(unique_ptr&& ioMove);
    /// @brief Assigns this to have the released data from the input pointer
    /// @param ioMove unique_ptr which shall have its data released and transferred
    /// @return this
    unique_ptr& operator=(unique_ptr&& ioMove);
    /// @brief Copy ctor is not available for unique_ptr, as duplicates are not allowed
    unique_ptr(unique_ptr& inCopy) = delete;
    /// @brief Copy assign is not available for unique_ptr, as duplicates are not allowed
    unique_ptr& operator=(unique_ptr& inCopy) = delete;

    /// @brief Gets the pointer to the stored data
    /// @return pointer to unique_ptr's data
    T* operator->();
    /// @brief Gets the address of the stored data pointer
    /// @return reference to unique_ptr's data object
    T& operator*();
    /// @brief Determines whether data is stored in the unique_ptr.
    /// Returns false if mData is nullptr
    operator bool();

private:
    T* mData{};
}; // class unique_ptr<T>

// Inlines

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

// TRICKY mnfitz 07apr2024: Variatic template arguments 
// Template that takes any number and type of arguments
/// @brief Method for creating and allocating a unique_ptr with the given arguments
template<typename T, typename ...Args>
unique_ptr<T> make_unique(Args... inArgs)
{
    T* data = new T{std::forward<Args>(inArgs)...};
    unique_ptr<T> unique{data};
    return unique;
}

#pragma endregion // unique_ptr

//
/////////////////////////////////////////////////////////////////////////////////
//

/// @brief std::shared_ptr is a smart pointer that manages a 
/// single object through a pointer and a /// control block,
/// which disposes of that object when the strong reference count goes to 0.
template<typename T>
class shared_ptr
{
public:
    /// @brief Default ctor returns a shared_ptr with null mData and mControl
    shared_ptr() = default;

    /// @brief Alt ctor returns a shared_ptr with initialized mData
    /// and mControl, with its mControl starting with a strong refcount of 1
    /// @param inData Template type, allowing any sort of data to be stored
    shared_ptr(T* inData) :
        mData{inData},
        mControl{new detail::ControlBlock<T>{inData}}
    {
        // Note: ControlBlock::Ctor{} sets strong count to 1
    }

    /// @brief dtor will reset the pointer
    ~shared_ptr()
    {
        reset();
    }

    /// @brief reset() erases the contents of the pointer and decrements its strong refcount by 1
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
    /// @brief Construct this with data moved from inputted shared_ptr
    /// @param ioMove shared_ptr which will have its data swapped with this, nullifying it 
    shared_ptr(shared_ptr&& ioMove) noexcept
    {
        if (ioMove)
        {
            // swap allows for noexcept move ctor
            std::swap(mData, ioMove.mData);
            std::swap(mControl, ioMove.mControl);
        }
    }

    // Move Operand
    /// @brief Assign this with data moved from inputted shared_ptr
    /// @param ioMove shared_ptr which will have its data swapped with this
    /// @return this
    shared_ptr& operator=(shared_ptr&& ioMove) noexcept
    {
        // Moving to self should be a NOP
        if (this != &ioMove)
        {
            // Reset/delete the old value which was moved
            //reset(); // noexcept requires us to leave as a turd
            if (ioMove)
            {
                // swap allows for noexcept move assign
                std::swap(mData, ioMove.mData);
                std::swap(mControl, ioMove.mControl);
            }
        }
        return *this;
    }

    // Copy Ctor
    /// @brief Construct this with data copied from inputted shared_ptr
    /// @param ioMove shared_ptr which will have its data copied
    shared_ptr(const shared_ptr& inCopy)
    {
        if (inCopy)
        {
            inCopy.mControl->IncrementStrong();
            mData = inCopy.mData;
            mControl = inCopy.mControl;
        }
    }

    // Copy Operand
    /// @brief Assign this with data copied from inputted shared_ptr
    /// @param ioMove shared_ptr which will have its data copied
    /// @return this
    shared_ptr& operator=(const shared_ptr& inCopy)
    {
        // Copying to self should be a NOP
        if (this != &inCopy)
        {
            reset();

            if (inCopy)
            {
                inCopy.mControl->IncrementStrong();
                mData = inCopy.mData;
                mControl = inCopy.mControl;
            }
        }
        return *this;
    }

    /// @brief Gets the pointer to the stored data
    /// @return pointer to shared_ptr's data
    T* operator->()
    {
        return get();
    }

    /// @brief Gets the address of the stored data pointer
    /// @return reference to shared_ptr's data object
    T& operator*()
    {
        return *get();
    }

    /// @brief Checks if unique_ptr has data, returns false if mData equals nullptr
    operator bool() const
    {
        return (mData != nullptr);
    }

    /// @brief Gets the stored data
    /// @return pointer to shared_ptr's data
    T* get()
    {
        return mData;
    }

private:
    template <typename T>
    friend class weak_ptr;

    shared_ptr(detail::ControlBlock<T>* inBlock)
    {
        if (inBlock != nullptr)
        {
            inBlock->IncrementStrong();
            mData = inBlock->get();
            mControl = inBlock;
        }
    }

private:
    T* mData{nullptr};
    detail::ControlBlock<T>* mControl{nullptr};
}; // class shared_ptr<T>

// TRICKY mnfitz 04072024: Variatic template arguments 
// Template that takes any number and type of arguments
/// @brief Method for creating and allocating a shared_ptr with the given arguments
template<typename T, typename ...Args>
shared_ptr<T> make_shared(Args... inArgs)
{
    T* data = new T{std::forward<Args>(inArgs)...};
    shared_ptr<T> unique{data};
    return unique;
}

//
/////////////////////////////////////////////////////////////////////////////////
//

/// @brief std::weak_ptr is a smart pointer similar to shared_ptr, that only has 
/// access to an object's control block. It does not increase the control block's 
/// reference count, but will still be deleted when the reference count reaches 0.
template <typename T>
class weak_ptr
{
public:
    /// @brief default ctor initializes mControl as nullptr
    weak_ptr() : 
        mControl{nullptr}
    {

    }
    /// @brief Alt ctor will make its mControl equal to inPtr's while incrementing its weakCount by 1
    /// @param inData Template type, allowing any sort of data to be stored
    weak_ptr(shared_ptr<T>& inPtr)
    {
        if (inPtr)
        {
            inPtr.mControl->IncrementWeak();
            mControl = inPtr.mControl;
        }
        // Note: ControlBlock::Ctor{} sets strong count to 1
    }

    /// @brief Rests weak_ptr
    ~weak_ptr()
    {
        reset();
    };

    /// @brief Sets mControl to nullptr, and checks if both strong  
    /// and weak refcounts are 0, meaning mControl should be deleted
    void reset()
    {
        if (mControl != nullptr)
        {
            auto [strong, weak] = mControl->DecrementWeak();
            if (weak <= 0 && strong <= 0)
            {
                // delete data not available for weak_ptr
                delete mControl;
            }
        }
        mControl = nullptr;
    }

    // Alt Assign Operator
    /// @brief Assigns this to have inShared's mControl and increments its weak refcount
    /// @param inShared shared_ptr which mControl is copied
    weak_ptr& operator=(shared_ptr<T>& inShared)
    {
        // Copying to self should be a NOP
        if (inShared.mControl != nullptr)
        {
            // bump weak refcount
            inShared.mControl->IncrementWeak();
            mControl = inShared.mControl;
        }
        return *this;
    }

    // RO5 Methods
    // Move Ctor
    /// @brief Constructs by swapping the mControl of this and ioMove; if it exists
    /// @param ioMove weak_ptr which will have its mControl swapped with null
    weak_ptr(weak_ptr&& ioMove) noexcept
    {
        if (ioMove)
        {
            // swap allows for noexcept move ctor
            std::swap(mControl, ioMove.mControl);
        }
    }

    // Move Operand
    /// @brief Assigns mControl to take ioMove's mControl; if it exists
    /// @param ioMove weak_ptr which will have its mControl be swapped with this
    weak_ptr& operator=(weak_ptr&& ioMove) noexcept
    {
         // Moving to self should be a NOP
        if (this != &ioMove)
        {
            if (ioMove)
            {
                // swap allows for noexcept move assign
                std::swap(mControl, ioMove.mControl);
            }
        }
        return *this;
    }

    // Copy Ctor
    /// @brief Constructs by copying inCopy's mControl and increments is weak refcount
    weak_ptr(const weak_ptr& inCopy)
    {
        if (inCopy)
        {
            inCopy.mControl->IncrementWeak();
            mControl = inCopy.mControl;
        }
    }

    // Copy Operand
    /// @brief Assigns mControl to copy inCopy's mCOntrol; if it exists
    weak_ptr& operator=(const weak_ptr& inCopy)
    {
        // Copying to self should be a NOP
        if (this != &inCopy)
        {
            reset(); // Decrement refcount of any previous ControlBlock

            if (inCopy)
            {
                inCopy->IncrementWeak();
                mControl = inCopy.mControl
            }
        }
        return *this;
    }

    /// @brief Create a shared_ptr from this with access to the pointer's stored data
    /// @return shared_ptr
    shared_ptr<T> lock()
    {
        shared_ptr<T> lockedPtr{mControl};
        return lockedPtr;
    }

    /// @brief Checks if weak_ptr has a ControlBlock, returns false if mControlBlock equals nullptr
    operator bool() const
    {
        return (mControl != nullptr);
    }

    /// @brief Gets the pointer to the stored ControlBlock
    /// @return pointer to shared_ptr's mControlBlock
    T* operator->()
    {
        return get();
    }

    /// @brief Gets the address of the stored ControlBlock pointer
    /// @return reference to shared_ptr's mControlBlock object
    T& operator*()
    {
        return *get();
    }

    /// @brief Gets the pointer to the stored ControlBlock
    /// @return pointer to shared_ptr's mControlBlock
    T* get()
    {
        return mControl.get();
    }

private:
    // TRICKY mnfitz 07apr2024: "Nested types" from a template class must be prefixed with the keyword 'typename'
    typename detail::ControlBlock<T>* mControl{nullptr};
}; // class shared_ptr<T>

} // namespace app355
#endif // APP_PTR_H