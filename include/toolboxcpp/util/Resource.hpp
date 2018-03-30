#pragma once
#include <functional>   // std::hash
#include <type_traits>

namespace toolboxcpp
{
namespace util
{
    /** Generic wrapper type for unmanaged resources
     *  @tparam Handle  Resource handle type, can be anything.
     *  @tparam Deleter Type of deleter functor. Should be compatible with `void (T)` call contract
     *  @tparam Tag     Marker type which allows to have multiple distinct wrappers
     *                  with same handle type and same deleter. Useful in cases
     *                  where C interface provides a bunch of distinct handles via `typedef void*`
     *                  which are deleted the same way, but used in different ways
     *
     *  @todo           Optional function for T which provides custom empty value
     */
    template<typename Handle, typename Deleter, typename Tag = Deleter>
    class Resource
    {
    public:
        /** Default constructor, inits internal handle with zero value
         */
        Resource()
            : Resource(Resource::zero(), Deleter{})
        {}
        /** Captures provided handle value
         *  @param  handle  Raw handle value
         */
        explicit Resource(Handle handle)
            : Resource(handle, Deleter{})
        {}
        /** Captures handle value and initializes deleter with provided value
         *  @param  handle  Raw handle value
         *  @param  deleter Initialization value for deleter, can be any value from which deleter can be constructed
         */
        template<typename Del>
        explicit Resource(Handle handle, Del&& deleter);
        /** Move constructor
         *  @param  other   Other resource wrapper
         */
        Resource(Resource&& other)
            : Resource()
        {
            swap(other);
        }
        /** Move assignment
         *  @param  other   Assign from this value
         *  @return         Reference to this instance
         */
        Resource& operator=(Resource&& other)
        {
            if (this != &other)
            {
                // First, destroy and clean current instance by moving it into temporary in nested scope
                { Resource(std::move(*this)); }
                // Next, move othar into this
                swap(other);
            }
            return *this;
        }

        Resource(Resource const&)            = delete;
        Resource& operator=(Resource const&) = delete;
        /** Destructor, actual destruction is handled by nested Storage type
         */
        ~Resource()                          = default;
        /** Checks if stored handle is "zero"
         *  @return true if stored handle is "zero", false otherwise
         */
        bool empty() const noexcept
        {
            return get() == Resource::zero();
        }
        /** Equality comparison, in case Handle type isn't ordered
         *  @param  other   Right-side comparison argument
         *  @return         True if stored handles compare equal, false otherwise
         */
        bool equals(Resource const& other) const noexcept
        {
            return get() == other.get();
        }
        /** Compare this instance with other resource wrapper
         *  @param  other   Right-side instance
         *  @retval -1      If this' handle is "less" than other's
         *  @retval  0      If both handles are equal
         *  @retval  1      If this' handle is "greater" than other's
         */
        int compare(Resource const& other) const noexcept
        {
            auto left = get(), right = other.get();
            if (left < right) { return -1; }
            if (left > right) { return  1; }
            return 0;
        }

        const Handle get() const noexcept;
              Handle get()       noexcept;

        operator const Handle () const noexcept  { return get(); }
        operator       Handle ()       noexcept  { return get(); }

        Resource& reset(Handle handle)
        {
            return *this = Resource(handle);
        }

        template<typename Del>
        Resource& reset(Handle handle, Del&& deleter)
        {
            return *this = Resource(handle, std::forward<Del>(deleter));
        }

        Handle detach() noexcept;

        void swap(Resource& that) noexcept;

    private:
        /** Provides abstracted "zero" value for handle type
         *  Usually just a default value of that type
         *  @return Zero handle
         */
        static Handle zero() noexcept;

        struct Storage;     ///< Keep impl details and possible specializations hidden
        Storage _storage;
    };

    template<typename H, typename D, typename T>
    bool operator== (Resource<H, D, T> const& left, Resource<H, D, T> const& right) noexcept
    {
        return left.equals(right);
    }

    template<typename H, typename D, typename T>
    bool operator!= (Resource<H, D, T> const& left, Resource<H, D, T> const& right) noexcept
    {
        return !(left == right);
    }

    template<typename H, typename D, typename T>
    bool operator< (Resource<H, D, T> const& left, Resource<H, D, T> const& right) noexcept
    {
        return left.compare(right) < 0;
    }

    template<typename H, typename D, typename T>
    bool operator<= (Resource<H, D, T> const& left, Resource<H, D, T> const& right) noexcept
    {
        return left.compare(right) <= 0;
    }

    template<typename H, typename D, typename T>
    bool operator> (Resource<H, D, T> const& left, Resource<H, D, T> const& right) noexcept
    {
        return left.compare(right) > 0;
    }

    template<typename H, typename D, typename T>
    bool operator>= (Resource<H, D, T> const& left, Resource<H, D, T> const& right) noexcept
    {
        return left.compare(right) >= 0;
    }

namespace resource
{
    /** Factory function, creates resource wrapper from handle and deleter
     *  @tparam H       Handle type
     *  @tparam D       Deleter type
     *  @param  handle  Handle value to store in @ref Resource
     *  @param  deleter Deleter instance and type for @ref Resource
     *  @return         Resource instance, ready for use
     */
    template<typename H, typename D>
    Resource<H, typename std::decay<D>::type> make(H handle, D&& deleter)
    {
        return Resource<H, typename std::decay<D>::type>(handle, std::forward<D>(deleter));
    }
    /** Factory function, creates resource wrapper from handle, deleter and tag type
     *  @note Tag provides only marker type and is not actually stored anywhere
     *  @tparam H       Handle type
     *  @tparam D       Deleter type
     *  @tparam T       Tag type
     *  @param  handle  Handle value to store in @ref Resource
     *  @param  deleter Deleter instance and type for @ref Resource
     *  @param  tag     Tag value, to deduce its type
     *  @return         Resource instance, ready for use
     */
    template<typename H, typename D, typename T>
    Resource<H, typename std::decay<D>::type, T> make(H handle, D&& deleter, T tag)
    {
        return Resource<H, typename std::decay<D>::type, T>(handle, std::forward<D>(deleter));
    }
} // namespace resource
} // namespace util
} // namespace toolboxcpp

namespace std
{
    template<typename H, typename D, typename T>
    struct hash<util::Resource<H, D, T>>
    {
        size_t operator () (util::Resource<H, D, T> const& value) const
        {
            return _hasher(value.get());
        }
    private:
        hash<H> _hasher;
    };
} // namespace std

/** @defgroup util_resource_impl @ref util::Resource implementation details. Beware of dragons
 *  @{
 */
namespace toolboxcpp
{
namespace util
{
namespace resource
{
namespace impl
{
    template<typename H>
    constexpr H zeroHandle() noexcept
    {
        return {};
    }

    template<typename H, typename D, typename T = void> struct Storage;
    /** Implementation of Resource::Storage for stateless deleter
     */
    template<typename H, typename D>
    struct Storage<H, D, typename std::enable_if<std::is_empty<D>::value>::type>
    {
        template<typename Del>
        Storage(H handle, Del&&)
            : _handle(handle)
        { }

        H const& get() const noexcept { return _handle; }
        H&       get()       noexcept { return _handle; }

        void swap(Storage& other) noexcept
        {
            std::swap(_handle, other._handle);
        }

        ~Storage()
        {
            auto tmp = zeroHandle<H>();
            if (_handle != tmp)
            {
                std::swap(_handle, tmp);
                (D{})(tmp);
            }
        }

    private:
        H _handle;
    };

    /** Implementation of Resource::Storage for stateful deleter
     */
    template<typename H, typename D>
    struct Storage<H, D, typename std::enable_if<!std::is_empty<D>::value>::type>
    {
        template<typename Del>
        Storage(H handle, Del&& deleter)
            : _handle(handle)
            , _deleter(std::forward<Del>(deleter))
        { }

        H const& get() const noexcept { return _handle; }
        H&       get()       noexcept { return _handle; }

        void swap(Storage& other) noexcept
        {
            std::swap(_handle, other._handle);
            std::swap(_deleter, other._deleter);
        }

        ~Storage()
        {
            auto tmp = zeroHandle<H>();
            if (_handle != tmp)
            {
                std::swap(_handle, tmp);
                _deleter(tmp);
            }
        }
    private:
        H _handle;
        D _deleter;
    };
} // namespace impl
} // namespace resource
    template<typename H, typename D, typename T>
    struct Resource<H, D, T>::Storage: public resource::impl::Storage<H, D>
    {
        using resource::impl::Storage<H, D>::Storage;
    };

    template<typename H, typename D, typename T>
    H Resource<H, D, T>::zero() noexcept
    {
        return resource::impl::zeroHandle<H>();
    }

    template<typename H, typename D, typename T>
    template<typename Del>
    Resource<H, D, T>::Resource(H handle, Del&& del)
        : _storage(handle, std::forward<Del>(del))
    { }

    template<typename H, typename D, typename T>
    const H Resource<H, D, T>::get() const noexcept
    {
        return _storage.get();
    }

    template<typename H, typename D, typename T>
    H Resource<H, D, T>::get() noexcept
    {
        return _storage.get();
    }

    template<typename H, typename D, typename T>
    H Resource<H, D, T>::detach() noexcept
    {
        auto freeHandle = zero();
        std::swap(freeHandle, _storage.get());
        return freeHandle;
    }

    template<typename H, typename D, typename T>
    void Resource<H, D, T>::swap(Resource<H, D, T>& other) noexcept
    {
        _storage.swap(other._storage);
    }
} // namespace util
} // namespace toolboxcpp
/** @}
 */
