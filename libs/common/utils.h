#include <type_traits>
#include <cassert>
#include <cstdlib>
#include <iostream>


template <typename T> 
typename std::underlying_type<T>::type constexpr ToUnderlying(const T& val) noexcept
{
    return static_cast<typename std::underlying_type<T>::type>(val);
}

#pragma warning (push)
#pragma warning (disable:4702)
namespace singleton {
/// Singleton implementation.

namespace defaultPolicies {
    /// Default policies.

    /// \class CreateUsingNew
    /// \brief Creates a new instance of \a Type using the \a new operator.
    /// \tparam Type Type to create an instance from. Must be default-constructible.
    template<typename Type>
    class CreateUsingNew {
    public:
        /// Creates a new instance and returns a pointer to it.
        /// \return Returns the newly created instance.
        static inline Type* Create()          { return new Type; }

        /// Destroys the passed instance.
        /// \param obj The instance to destroy.
        static inline void Destroy(Type* obj) { delete obj;      }
    };

    /// \class DefaultLifetime
    /// \brief Implements object lifetime per C++ rules: Last created - first destroyed.
    /// \tparam Type Type of the object to track liftime of. Not used here.
    template<typename Type>
    class DefaultLifetime {
    public:
        /// Handler if dead reference is detected in the singleton.
        /// \throws Throws a \a std::logic_error.
        static inline void OnDeadReference()                  { throw std::logic_error("Dead reference!"); }

        /// Schedules destruction of the singleton.
        /// This policy uses \a std::exit to implement object lifetime according to C++ rules.
        /// \param fun Function that destroys the singleton.
        static inline void ScheduleDestruction(void (*fun)()) { std::atexit(fun);                          }
    };

    /// \class SingleThreaded
    /// \brief Policy class only allowing single threaded application. Minimalistic here.
    /// \tparam Type Type of the instance that can be accessed in parallel.
    template<typename Type>
    class SingleThreaded {
        /// \struct DummyLock
        /// \brief Dummy lock instance, does not lock.
        struct DummyLock {};
    public:
        typedef DummyLock Lock;     ///< Lock guard type.
        typedef Type VolatileType;  ///< Volatile type. Not volatile here to allow better optimizations.
    };

} // defaultPolicies


/// \class Singleton
/// \brief Class implementing the \a Singleton design pattern for a passed type.
///
/// This class implements the \a Singleton design pattern which guarantees that there is at most
/// one instance of the passed instance type kept alive.
///
/// This class accepts three different policies:
///
///   - CreationPolicy:
///     - Policy for creating and destroying objects.
///     - Interface:
///       - InstanceT* Create()
///       - void Destroy(InstanceT* obj)
///     - Defaults to: singleton::defaultPolicies::CreateUsingNew
///   - LifetimePolicy:
///     - Policy for managing the lifetime of objects.
///     - Interface:
///       - void OnDeadReference()
///       - void ScheduleDestruction(void (*)())
///     - Defaults to: singleton::defaultPolicies::DefaultLifetime
///   - ThreadingModel:
///     - Policy for parallel access protection.
///     - Needed typedefs:
///       - Lock
///       - VolatileType
///     - Defaults to: singleton::defaultPolicies::SingleThreaded
///
/// Example usage:
///
///   class A {};
///
///   A& ins = Singleton<A>::Instance();
///   A& oth = Singleton<A>::Instance();
///   assert(std::addressof(ins) == std::addressof(oth));
///
/// \tparam InstanceT      Type to contain a single instance of.
/// \tparam CreationPolicy Policy responsible for creating and destroying instances of \a InstanceT.
/// \tparam LifetimePolicy Policy responsible for tracking the lifetime of the instance.
/// \tparam ThreadingModel Policy responsible for protecting the instance from parallel access.
///
/// \author Andrei Alexandrescu (in "Modern C++ Design"), documented by Thomas Lang.
/// \date   2018-05-22
template<
    typename InstanceT,
    template <typename> class CreationPolicy = defaultPolicies::CreateUsingNew,
    template <typename> class LifetimePolicy = defaultPolicies::DefaultLifetime,
    template <typename> class ThreadingModel = defaultPolicies::SingleThreaded
>
class Singleton {
public:
    /// Gets the instance held by the singleton. Initializes it on first call.
    /// \return Returns the kept instance.
    static InstanceT& Instance();

private:
    /// Destroys the Singleton.
    static void DestroySingleton();

    // Prevents the user from creating, copying, assigning or destroying the instance.
    Singleton()                            = delete;
    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;
    ~Singleton()                           = delete;

private:
    typedef typename ThreadingModel<InstanceT>::VolatileType InstanceType; ///< Instance type, maybe volatile.
    static InstanceType* instance;                                         ///< Static, sole instance.
    static bool destroyed;                                                 ///< Indicator if the Singleton was destroyed.
};

/// Static member initialization.

template<
    typename InstanceT,
    template <typename> class CreationPolicy,
    template <typename> class LifetimePolicy,
    template <typename> class ThreadingModel
>
typename Singleton<InstanceT, CreationPolicy, LifetimePolicy, ThreadingModel>::InstanceType*
Singleton<InstanceT, CreationPolicy, LifetimePolicy, ThreadingModel>::instance = nullptr;

template<
    typename InstanceT,
    template <typename> class CreationPolicy,
    template <typename> class LifetimePolicy,
    template <typename> class ThreadingModel
>
bool Singleton<InstanceT, CreationPolicy, LifetimePolicy, ThreadingModel>::destroyed = false;

////////////////////////////////////////////////////////////////////////////////

template<
    typename InstanceT,
    template <typename> class CreationPolicy,
    template <typename> class LifetimePolicy,
    template <typename> class ThreadingModel
>
InstanceT& Singleton<InstanceT, CreationPolicy, LifetimePolicy, ThreadingModel>::Instance() {
    if (!instance) {
        typename ThreadingModel<InstanceT>::Lock guard;
        (void) guard; // Suppresses the "unused variable" warning.

        // The famous double-lock-checking model:
        // Check in a possibly multi-threaded environment if it is initialized at all.
        // If not, lock the access to it.
        // Then, check again. Because it is locked by now and if it was not created yet,
        // no other thread can create it in parallel now. We can safely create it then.
        if (!instance) { //-V547
            if (destroyed) {
                // If the instance is not present and we know that it was already destroyed,
                // something bad happened. Let the LifetimePolicy decide what to do now.
                LifetimePolicy<InstanceT>::OnDeadReference();
                destroyed = false;
            }

            // Now, let the CreationPolicy create the instance and schedule its destruction
            // according to the Lifetime policy.
            instance = CreationPolicy<InstanceT>::Create();
            LifetimePolicy<InstanceT>::ScheduleDestruction(&DestroySingleton);
        }
    }

    return *instance;
}

template<
    typename InstanceT,
    template <typename> class CreationPolicy,
    template <typename> class LifetimePolicy,
    template <typename> class ThreadingModel
>
void Singleton<InstanceT, CreationPolicy, LifetimePolicy, ThreadingModel>::DestroySingleton() {
    assert(!destroyed && "Singleton was already destroyed!");
    // Destroying is simple: Destroy the instance and clean up.
    CreationPolicy<InstanceT>::Destroy(instance);
    instance  = nullptr;
    destroyed = true;
}

} // singleton
#pragma warning (pop)
