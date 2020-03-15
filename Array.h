#ifndef DATA_ARRAY_INCLUDED
#define DATA_ARRAY_INCLUDED

#include <string.h>
#include <vector>

namespace data
{
    /**
     * Struct CopyPolicy represents storage policy for the Array class.
     * The purpose of this policy is to create own copy of the array
     * and delete it when no longer needed.
     *
     * @author Ilia Yastrebov
     */
    template <class T>
    struct CopyPolicy
    {
        /**
         * Creates copy of the given array and returns its pointer. Works
         * *only* with PODs (plain old data) and cannot be used with more
         * complicated user-defined types
         *
         * @param ptr Source array
         * @param size Size of the source array
         * @return Pointer to the new array
         */
        static const T * copy(const T * ptr, size_t size)
        {
            T * copy = new T[size];
            memcpy(copy, ptr, size * sizeof(T));
            return copy;
        }

        /**
         * Destroys array.
         *
         * @param ptr Source array
         * @param size Size of the array
         */
        static void destroy(const T * ptr, size_t size)
        {
            if (NULL != ptr) {
                delete [] ptr;
                ptr = NULL;
            }
        }
    };

    /**
     * CopyPolicy specialization for array of char (string)
     *
     * @author Ilia Yastrebov
     */
    template <>
    struct CopyPolicy<char>
    {
        static const char * copy(const char * ptr, size_t size)
        {
            char * copy = new char[size + 1];
            strcpy(copy, ptr);
            return copy;
        }

        static void destroy(const char * ptr, size_t size)
        {
            if (NULL != ptr) {
                delete [] ptr;
                ptr = NULL;
            }
        }
    };

    /**
     * CopyPolicy specialization for array of strings
     *
     * @author Ilia Yastrebov
     */
    template <>
    struct CopyPolicy<char *>
    {
        static char * const * copy(char * const * ptr, size_t size)
        {
            char ** copy = new char * [size];
            for (size_t i = 0; i < size; ++i) {
                copy[i] = new char[strlen(ptr[i]) + 1];
                strcpy(copy[i], ptr[i]);
            }
            return static_cast<char * const *>(copy);
        }

        static void destroy(char * const * ptr, size_t size)
        {
            if (NULL != ptr) {
                for (size_t i = 0; i < size; ++i) {
                    delete[] ptr[i];
                }
                delete [] ptr;
                ptr = NULL;
            }
        }
    };

    /**
     * Struct MovePolicy represents storage policy for the Array class.
     * The purpose of this policy is to take ownership of the array
     * and delete it when no longer needed.
     *
     * @author Ilia Yastrebov
     */
    template <class T>
    struct MovePolicy
    {
        // Returns the same pointer - no copy!
        static const T * copy(const T * ptr, size_t size) { return ptr; }

        /**
         * Destroys array.
         *
         * @param ptr Source array
         * @param size Size of the array
         */
        static void destroy(const T * ptr, size_t size)
        {
            if (NULL != ptr) {
                delete [] ptr;
                ptr = NULL;
            }
        }
    };

    /**
     * MovePolicy specialization for array of char (string)
     *
     * @author Ilia Yastrebov
     */
    template <>
    struct MovePolicy<char>
    {
        // Returns the same pointer - no copy!
        static const char * copy(const char * ptr, size_t size) { return ptr; }

        static void destroy(const char * ptr, size_t size)
        {
            if (NULL != ptr) {
                delete [] ptr;
                ptr = NULL;
            }
        }
    };

    /**
     * MovePolicy specialization for array of strings
     *
     * @author Ilia Yastrebov
     */
    template <>
    struct MovePolicy<char *>
    {
        // Returns the same pointer - no copy!
        static char * const * copy(char * const * ptr, size_t size) { return ptr; }

        static void destroy(char * const * ptr, size_t size)
        {
            if (NULL != ptr) {
                for (size_t i = 0; i < size; ++i) {
                    delete[] ptr[i];
                }
                delete [] ptr;
                ptr = NULL;
            }
        }
    };

    /**
     * Struct MoveStringPolicy represents storage policy for the Array class.
     * The purpose of this policy is to take ownership of the array and delete it when no longer needed,
     * but now for specific strings. This policy is used by binary deserialization for strings array
     *
     * @author Ilia Yastrebov
     */
    template <class T>
    struct MoveStringPolicy
    {
        // Returns the same pointer - no copy!
        static const T * copy(const T * ptr, size_t size) { return ptr; }

        /**
         * Destroys array.
         *
         * @param ptr Source array
         * @param size Size of the array
         */
        static void destroy(const T * ptr, size_t size)
        {
            if (NULL != ptr) {
                delete [] ptr;
                ptr = NULL;
            }
        }
    };

    /**
     * Struct ShallowCopyPolicy represents storage policy for the Array class.
     * The purpose of this policy is to deal with source pointer and prevent
     * any copying.
     *
     * @author Ilia Yastrebov
     */
    template <class T>
    struct ShallowCopyPolicy
    {
        // Returns the same pointer - no copy!
        static const T * copy(const T * ptr, size_t size) { return ptr; }

        // Does nothing - we should not delete source pointer!
        static void destroy(const T * ptr, size_t size) {};
    };

    /**
     * Class Array represents base class for arrays
     *
     * @author Ilia Yastrebov
     */
    class ArrayBase
    {
    public:
        typedef std::vector<size_t> Dims;

        /**
         * Helper method that calculates the actual size of array from its dimensions
         *
         * @param dims Array dimensions
         * @return Actual size of array from its dimensions
         */
        static size_t calculateSize(const Dims & dims)
        {
            size_t size = 1;
            for (Dims::const_iterator it = dims.begin(); it != dims.end(); ++it) {
                size *= (*it);
            }
            return size;
        }

        virtual ~ArrayBase() {}
    };

    /**
     * Class Array represents the array of elements allocated as a continous memory block.
     * This class is suitable to store multi-dimensional arrays.
     *
     * @author Ilia Yastrebov
     */
    template <class T>
    class Array : public ArrayBase
    {
    public:
        /**
         * Returns array pointer.
         *
         * @param dims Output parameter which indicates array dimensions
         * @return pointer to the array
         */
        const T * getArray(Dims & dims) const
        {
            dims = dims_;
            return ptr_;
        }

        /**
         * Gets number of elements in array
         *
         * @return size_t number of elements
         */
        size_t getSize() const
        {
            return size_;
        }

        /**
         * Dtor.
         */
        virtual ~Array() {}

        /**
         * Compare operator works for one dimension array
         *
         * @param other object to compare
         */
        bool operator==(Array<T>& other)
        {
            if(dims_ != other.dims_) {
                return false;
            }

            for (size_t i=0; i<size_; i++) {
                if(ptr_[i] != other.ptr_[i]) {
                    return false;
                }
            }

            return true;
        }

    protected:
        /**
         * Protected ctor.
         */
        Array(const T * ptr, const Dims & dims) :
            ptr_(ptr),
            dims_(dims),
            size_(calculateSize(dims))
        {
        }

        /** Pointer to array */
        const T * ptr_;

        /** Array dimensions */
        Dims dims_;

        /** Array length (allocated for all dimensions) */
        size_t size_;
    };

    /**
     * Class ArrayImpl represents array of elements with storage policy.
     * This class simplify storage of arrays with different storage
     * policies.
     *
     * @author Ilia Yastrebov
     */
    template < class T,
               template <class> class StoragePolicy = CopyPolicy >
    class ArrayImpl : public Array<T>
    {
    public:
        /**
         * Ctor.
         *
         * @param ptr Source array
         * @param dims Array dimensions
         */
        ArrayImpl(const T * ptr, const ArrayBase::Dims & dims) :
            Array<T>(StoragePolicy<T>::copy(ptr, ArrayBase::calculateSize(dims)), dims)
        {
        }

        /**
         * Dtor.
         */
        virtual ~ArrayImpl()
        {
            StoragePolicy<T>::destroy(Array<T>::ptr_, Array<T>::size_);
        }

    };

} // namespace data

#endif
