#include <type_traits>
#include <utility>
#include <cstddef>

namespace naive_smart_pointers {
    template <typename T>
    struct deleter_pointer_wrapper {
        void (*fp)(T*);

        deleter_pointer_wrapper(void (*fp_)(T*)) : fp{fp_} { }
        void operator()(T* t) const {
            fp(t);
        }
    };

    template <typename T>
    struct default_deleter {
        void operator()(T* t) const {
            delete t;
        }
    };

    template <typename T>
    struct default_deleter<T[]> {
        void operator()(T* t) const {
            delete[] t;
        }
    };

    template <typename T>
    struct is_deleter_function_candidate : 
        std::false_type {};

    template <typename T>
    struct is_deleter_function_candidate<void (*)(T*)> : 
        std::true_type {};

    template <typename T>
    constexpr auto is_deleter_function_candidate_v = 
        is_deleter_function_candidate<T>::value;

    template <typename T, typename D>
    using deleter_base_t = std::conditional_t<is_deleter_function_candidate_v<D>,
        deleter_pointer_wrapper<T>, D>;

    template <typename T, typename D>
    class unique_ptr_base : deleter_base_t<T, D> {
    protected:
        using deleter_type = deleter_base_t<T, D>;
        T* p{nullptr};
        void swap(T& other) noexcept {
            using std::swap;
            swap(static_cast<deleter_type&>(this), static_cast<deleter_type&>(other));
            swap(p, other.p);
        }

        bool operator==(const unique_ptr_base& other) {
            return p == other.p;
        }
        // take note: operator!=(const unique_ptr_base&) inferred from operator==() since c++20
        // operator!= -> !(operator==)

    public:
        unique_ptr_base() = default;
        unique_ptr_base(T* p_) : p{p_} {}
        unique_ptr_base(T* p_, void (*fp_)(T*)) : 
            deleter_type{fp_}, p{p_} {}
        ~unique_ptr_base() {
            if (p) {
                (*static_cast<deleter_type*>(this))(p);
            }
        }

        unique_ptr_base(const unique_ptr_base&) = delete;
        unique_ptr_base& operator=(const unique_ptr_base&) = delete;

        unique_ptr_base(unique_ptr_base&& other) noexcept :
            deleter_type{std::move(static_cast<deleter_type&>(other))},
            p{std::exchange(other.p, nullptr)} {}

        unique_ptr_base& operator=(unique_ptr_base&& other) noexcept {
            unique_ptr_base{std::move(other)}.swap(*this);
            return *this;
        }

        bool empty() const noexcept {
            return p;
        }

        // take note: this is a conversion function. the type in the name is the return
        // type, thus writing the return type is redundant
        // same goes to operator X(); operator int(); operator std::string()
        explicit operator bool() const noexcept {
            return !empty();
        }

        T* get() noexcept {
            return p;
        }

        // Do we need to add a const getter()?

    };

    // general unique_ptr template
    template<typename T, typename D = default_deleter<T>>
    class unique_ptr : public unique_ptr_base<T, D> {
        public:
        // take note: exposing the parent's ctor functions
        // without this, we will not be getting the compiler generated
        // free functions
        using unique_ptr_base<T, D>::unique_ptr_base;

        void swap(unique_ptr& other) noexcept {
            unique_ptr_base<T, D>::swap(other);
        }

        bool operator==(const unique_ptr& other) const noexcept {
            // return unique_ptr_base<T, D>::operator==(other);
            return this->p == other.p;
        }

        // take note: removing this to allow compiler generated move assignment.
        // generated automatically 
        // unique_ptr& operator=(unique_ptr&& other) noexcept {
            // unique_ptr_base<T, D>::operator=(std::move(other));
            // return this;
        // }

        T& operator*() noexcept {
            return *(this->p);
        }

        const T& operator*() const noexcept {
            return *(this->p);
        }

        T* operator->() noexcept {
            return this->p;
        }

        const T* operator->() const noexcept {
            return this->p;
        }
    };

    // unique_ptr specialization for arrays
    // couldnt assign default deleter D = default_deleter<T>> like above 
    // seems like partial specialization shouldnt have default template arg
    template <typename T, typename D>
    class unique_ptr<T[], D> : public unique_ptr_base<T, D> {
    public:
        using unique_ptr_base<T, D>::unique_ptr_base;

        void swap(unique_ptr& other) noexcept {
            unique_ptr_base<T, D>::swap(other);
        }

        bool operator==(const unique_ptr& other) const noexcept {
            return this->p == other.p;
        }

        T& operator[](std::size_t n) noexcept {
            return (this->p)[n];
        }

        const T& operator[](std::size_t n) const noexcept {
            return (this->p)[n];
        }
    };

}