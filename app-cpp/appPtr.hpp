#ifndef APP_PTR_H
#define APP_PTR_H

// std
#include <atomic>

namespace app355
{

    // UNIQUE POINTER
    template <typename T>
    class unique_ptr
    {
    public:
        unique_ptr() = default;

        unique_ptr(T *inData) : mData{inData}
        {
            // Do Nothing
        }

        ~unique_ptr()
        {
            reset();
        }

        void reset()
        {
            // delete is smart enough to do nothing when passed nullptr
            delete mData;
            mData = nullptr;
        }

        T *release()
        {
            T *data = mData;
            mData = nullptr;
            return data;
        }

        // RO5 Methods
        // Move Ctor
        unique_ptr(unique_ptr &&ioMove)
        {
            reset();
            mData = ioMove.release();
        }

        // Move Operand
        unique_ptr &operator=(unique_ptr &&ioMove)
        {
            // Moving to self should be a NOP
            if (this != &ioMove)
            {
                reset();
                mData = ioMove.release();
            }
            return *this;
        }

        // Unique has no Copy Ctor
        unique_ptr(unique_ptr &inCopy) = delete;

        // Unique has no Copy Operand
        unique_ptr &operator=(unique_ptr &inCopy) = delete;

        T *operator->()
        {
            return get();
        }

        T &operator*()
        {
            return *get();
        }

        operator bool() const
        {
            return (mData != nullptr);
        }

        T *get()
        {
            return mData;
        }

    private:
        T *mData{};
    }; // class unique_ptr<T>

    // SHARED POINTER
    template <typename T>
    class shared_ptr
    {
    public:
        shared_ptr() = default;

        shared_ptr(T *inData) : mData{inData},
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
            // delete is smart enough to do nothing when passed nullptr
            if (mControl->DecrementStrong().first <= 0)
            {
                delete mData;
                if (mControl->Get().second <= 0)
                {
                    delete mControl;
                }
            }
            mData = nullptr;
            mControl = nullptr;
        }

        // RO5 Methods
        // Move Ctor
        shared_ptr(shared_ptr<T>&& ioMove) : mData{ioMove.mData},
                                          mControl{ioMove.mControl}
        {
            ioMove.mData = nullptr;
            ioMove.mControl = nullptr;
        }

        // Move Operand
        shared_ptr &operator=(shared_ptr<T>&& ioMove)
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
        shared_ptr(const shared_ptr &inCopy)
        {
            mData = inCopy.mData;
            mControl = inCopy.mControl;
            mControl->IncrementStrong();
        }

        // Copy Operand
        shared_ptr &operator=(const shared_ptr &inCopy)
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

        T *operator->()
        {
            return get();
        }

        T &operator*()
        {
            return *get();
        }

        operator bool() const
        {
            return (mData != nullptr);
        }

        T *get()
        {
            return mData;
        }

    private:
        class ControlBlock
        {
        public:
            ControlBlock(T *inData) :  
                 mData{inData}
            {
                // Do nothing
            }

            ControlBlock(shared_ptr<T>* inPtr) :
                 mData{inPtr->mData}
            {
                // Do nothing
            }

            // RO5 Methods added
            ~ControlBlock() = default;

            T *get()
            {
                return mData;
            }

            std::pair<std::size_t, std::size_t> Get()
            {
                std::lock_guard<std::mutex> lock{mMutex};
                std::pair<std::size_t, std::size_t> refCounts{mStrongCount, mWeakCount};
                return refCounts;
            }

            std::pair<std::size_t, std::size_t> IncrementStrong()
            {
                std::lock_guard<std::mutex> lock{mMutex};
                std::pair<std::size_t, std::size_t> refCounts{++mStrongCount, mWeakCount};
                return refCounts;
            }

            std::pair<std::size_t, std::size_t> DecrementStrong()
            {
                std::lock_guard<std::mutex> lock{mMutex};
                std::pair<std::size_t, std::size_t> refCounts{--mStrongCount, mWeakCount};
                return refCounts;
            }

            std::pair<std::size_t, std::size_t> IncrementWeak()
            {
                std::lock_guard<std::mutex> lock{mMutex};
                std::pair<std::size_t, std::size_t> refCounts{mStrongCount, ++mWeakCount};
                return refCounts;
            }

            std::pair<std::size_t, std::size_t> DecrementWeak()
            {
                std::lock_guard<std::mutex> lock{mMutex};
                std::pair<std::size_t, std::size_t> refCounts{mStrongCount, --mWeakCount};
                return refCounts;
            }

        private:
            std::size_t mStrongCount{1};
            std::size_t mWeakCount{0};
            std::mutex mMutex{};
            T *mData{}; // Used by weak pointers to obtain a new shared pointer
        };              // class ControlBlock

    private:
        template <typename T>
        friend class weak_ptr;

        shared_ptr(ControlBlock& inBlock) : 
            mData{inBlock.mData},
            mControl{inBlock}
        {

        }

    private:
        T *mData{};
        ControlBlock *mControl{};
    }; // class shared_ptr<T>

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
            if ((mControl->DecrementWeak().second <= 0) && (mControl->Get().first <= 0))
            {
                delete mControl;
            }
            mControl = nullptr;
        }

        // RO5 Methods
        // Shared Assign
        weak_ptr &operator=(shared_ptr<T>& inShared)
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
        weak_ptr(weak_ptr<T>&& ioMove) : mControl{ioMove.mControl}
        {
            ioMove.mControl = nullptr;
        }

        // Move Operand
        weak_ptr &operator=(weak_ptr<T>&& ioMove)
        {
            // Moving to self should be a NOP
            if (this != &ioMove)
            {
                std::swap(mControl, ioMove.mControl);
            }
            return *this;
        }

        // Copy Ctor
        weak_ptr(const weak_ptr &inCopy)
        {
            mControl = inCopy.mControl;
            mControl->IncrementStrong();
        }

        // Copy Operand
        weak_ptr &operator=(const weak_ptr &inCopy)
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

            return shared_ptr { mControl }
        }

        operator bool() const
        {
            return (mControl != nullptr);
        }

        T *operator->()
        {
            return get();
        }

        T &operator*()
        {
            return *get();
        }

        T *get()
        {
            return mControl.get();
        }

    private:
        // TRICKY: "Nested types" from a template class must be prefixed with the keyword 'typename'
        typename shared_ptr<T>::ControlBlock *mControl{};
    }; // class shared_ptr<T>

} // namespace app355
#endif // APP_PTR_H