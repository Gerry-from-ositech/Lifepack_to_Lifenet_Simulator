//  Asynchronous Function Call Library

//  Copyright JaeWook Choi 2007. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef AFC_ASYNCHRONOUS_FUNCTION_CALL_HEADER
#define AFC_ASYNCHRONOUS_FUNCTION_CALL_HEADER

#include <memory>

#include <boost/function/function0.hpp>
#include <boost/function/function1.hpp>
#include <boost/function/function2.hpp>

#define BOOST_BIND_ENABLE_STDCALL
#include <boost/bind.hpp>

#include <boost/shared_ptr.hpp>

#include <boost/type_traits/remove_reference.hpp>

#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

#define AFC_ERROR_UNHANDLED_EXCEPTION       ((UINT)-1)

#define AFC_EXIT_CODE_NORMAL                ((DWORD)0x0afc0000)
#define AFC_EXIT_CODE_TERMINATION           ((DWORD)0x0afcffff)

//
// WIN32 Multithreading programming tips.
//

// Visual C++ Multithreading: Programming Tips
// http://msdn2.microsoft.com/en-us/library/h14y172e(VS.80).aspx#_core_windows_handle_maps

// MFC Library Reference TN003: Mapping of Windows Handles to Objects
// http://msdn2.microsoft.com/en-us/library/c251x6s1(VS.80).aspx

// Don¡¯t Use the Win32 API PostThreadMessage() to Post Messages to UI Threads
// http://www.devx.com/tips/Tip/14720

namespace afc {  namespace detail
{

/**
 * 
 */
template<typename R>
struct ignore_completion
{
  static void routine(R, UINT) { }
};

template<>
struct ignore_completion<void>
{
  static void routine(UINT) { }
};

/**
 *  Thread specific local storage.
 */
template<typename T>
class afc_tss
{
private:
  DWORD slot_number_;
  DWORD is_set_;

public:
  afc_tss() : slot_number_( ::TlsAlloc() ), is_set_( ::TlsAlloc() )
  {
    BOOST_ASSERT( TLS_OUT_OF_INDEXES != slot_number_ );
    BOOST_ASSERT( TLS_OUT_OF_INDEXES != is_set_ );
  }

  ~afc_tss()
  {
    ::TlsFree( is_set_ );
    ::TlsFree( slot_number_ );
  }

  void set_value(T * value)
  {
    ::TlsSetValue( is_set_ , reinterpret_cast<void *>( -1 ) );
    ::TlsSetValue( slot_number_, value );
  }

  T * get_value()
  {
    return 0 != ::TlsGetValue( is_set_ )
      ? static_cast<T *>( ::TlsGetValue( slot_number_ ) )
      : 0;  // Called from the main thread.
  }
};

/**
 * Thread routine type enumerates.
 */
enum thread_routine_type
{
  win32_thread_routine  = 1,
  crt_thread_routine    = 2,
  mfc_thread_routine    = 3
};

} } // namespace afc::detail

namespace afc
{

/**
 *  Completion routine object.
 */
template<typename R>
class on_completion_t
{
public:
  typedef R result_type;
  typedef boost::function2<void, R, UINT> completion_routine_type;
  
private:
  completion_routine_type completion_routine_;

public:
  explicit on_completion_t(void (*completion_fxn)(R, UINT, ULONG_PTR), ULONG_PTR completion_key = 0)
    : completion_routine_( boost::bind( completion_fxn, boost::arg<1>(), boost::arg<2>(), completion_key ) )
  {
  }

  explicit on_completion_t(void (CALLBACK *completion_fxn)(R, UINT, ULONG_PTR), ULONG_PTR completion_key = 0)
    : completion_routine_( boost::bind( completion_fxn, boost::arg<1>(), boost::arg<2>(), completion_key ) )
  {
  }

  template<typename U, typename T>
  on_completion_t(void (U::*completion_fxn)(R, UINT, ULONG_PTR), T * t, ULONG_PTR completion_key = 0)
    : completion_routine_( boost::bind( completion_fxn, t, boost::arg<1>(), boost::arg<2>(), completion_key ) )
  {
  }

  explicit on_completion_t(completion_routine_type const & completion_routine)
    : completion_routine_( completion_routine )
  {
  }

  void operator ()(result_type result, UINT error_code) const
  {
    completion_routine_( result, error_code );
  }
};  // class on_completion_t

/**
 *  Completion routine object. void return specialization.
 */
template<>
class on_completion_t<void>
{
public:
  typedef boost::function1<void, UINT> completion_routine_type;

private:
  completion_routine_type completion_routine_;

public:
  explicit on_completion_t(void (CALLBACK *completion_fxn)(UINT, ULONG_PTR), ULONG_PTR completion_key = 0)
    : completion_routine_( boost::bind( completion_fxn, boost::arg<1>(), completion_key ) )
  {
  }

  explicit on_completion_t(void (*completion_fxn)(UINT, ULONG_PTR), ULONG_PTR completion_key = 0)
    : completion_routine_( boost::bind( completion_fxn, boost::arg<1>(), completion_key ) )
  {
  }

  template<typename U, typename T>
  on_completion_t(void (U::*completion_fxn)(UINT, ULONG_PTR), T * t, ULONG_PTR completion_key = 0)
    : completion_routine_( boost::bind( completion_fxn, t, boost::arg<1>(), completion_key ) )
  {
  }

  explicit on_completion_t(completion_routine_type const & completion_routine)
    : completion_routine_( completion_routine )
  {
  }

  void operator ()(UINT error_code) const
  {
    completion_routine_( error_code );
  }
};  // class on_completion_t<void>

/**
 * Helper function templates for the completion routine.
 */
template<typename R> inline
on_completion_t<R>
on_completion(void (*completion_fxn)(R, UINT, ULONG_PTR), ULONG_PTR completion_key = 0)
{
  return on_completion_t<R>( completion_fxn, completion_key );
}

template<typename R> inline
on_completion_t<R>
on_completion(void (CALLBACK *completion_fxn)(R, UINT, ULONG_PTR), ULONG_PTR completion_key = 0)
{
  return on_completion_t<R>( completion_fxn, completion_key );
}

template<typename R, typename U, typename T> inline
on_completion_t<R>
on_completion(void (U::*completion_fxn)(R, UINT, ULONG_PTR), T * t, ULONG_PTR completion_key = 0)
{
  return on_completion_t<R>( completion_fxn, t, completion_key );
}

template<typename R> inline
on_completion_t<R>
on_completion(typename on_completion_t<R>::completion_routine_type const & completion_routine)
{
  return on_completion_t<R>( completion_routine );
}

inline
on_completion_t<void>
on_completion(void (*completion_fxn)(UINT, ULONG_PTR), ULONG_PTR completion_key = 0)
{
  return on_completion_t<void>( completion_fxn, completion_key );
}

inline
on_completion_t<void>
on_completion(void (CALLBACK *completion_fxn)(UINT, ULONG_PTR), ULONG_PTR completion_key = 0)
{
  return on_completion_t<void>( completion_fxn, completion_key );
}

template<typename U, typename T> inline
on_completion_t<void>
on_completion(void (U::*completion_fxn)(UINT, ULONG_PTR), T * t, ULONG_PTR completion_key = 0)
{
  return on_completion_t<void>( completion_fxn, t, completion_key );
}

/**
 *  Helper templates to return the default completion routine.
 */
template<typename R> inline
on_completion_t<R> no_completion()
{
  return on_completion_t<R>( &detail::ignore_completion<R>::routine );
}

/**
 *  WIN32 thread traits.
 */
template<LPSECURITY_ATTRIBUTES ThreadAttributes = NULL, int Priority = THREAD_PRIORITY_NORMAL, size_t StackSize = 0>
struct win32_thread
{
  static HANDLE create_thread(LPTHREAD_START_ROUTINE fnThreadProc, LPVOID lpParameter, LPDWORD pdwThreadid)
  {
    HANDLE thread_handle = ::CreateThread( ThreadAttributes, StackSize, fnThreadProc
      , lpParameter, 0, pdwThreadid );

    ::SetThreadPriority( thread_handle, Priority );

    return thread_handle;
  }

  static HANDLE create_thread(unsigned (__stdcall *)(LPVOID), void *, unsigned *)
  {
    BOOST_ASSERT(false);
    return 0;
  }

  static HANDLE create_thread(UINT (*)(LPVOID), LPVOID, LPDWORD)
  {
    BOOST_ASSERT(false);
    return 0;
  }

  BOOST_STATIC_CONSTANT( detail::thread_routine_type, routine_type = detail::win32_thread_routine );

};  // struct win32_thread

#if !defined(_ATL_MIN_CRT) && defined(_MT)

/**
 *  CRT thread traits.
 */
template<LPVOID security = NULL, int Priority = THREAD_PRIORITY_NORMAL, size_t StackSize = 0>
struct crt_thread
{
  static HANDLE create_thread(LPTHREAD_START_ROUTINE, LPVOID, LPDWORD)
  {
    BOOST_ASSERT(false);
    return 0;
  }

  static HANDLE create_thread(unsigned (__stdcall *fnThreadProc)(LPVOID), void * argument, unsigned * thrdaddr)
  {
    typedef unsigned int (__stdcall * start_address)(LPVOID);
    HANDLE thread_handle = (HANDLE)_beginthreadex( security, StackSize, fnThreadProc
      , argument, 0, thrdaddr );

    ::SetThreadPriority( thread_handle, Priority );

    return thread_handle;
  }

  static HANDLE create_thread(UINT (*)(LPVOID), LPVOID, LPDWORD)
  {
    BOOST_ASSERT(false);
    return 0;
  }

  BOOST_STATIC_CONSTANT( detail::thread_routine_type, routine_type = detail::crt_thread_routine );

};  // struct crt_thread

#endif  // #if !defined(_ATL_MIN_CRT) && defined(_MT)

#if defined(_MFC_VER) && defined(_MT)

/**
 *  MFC thread traits.
 *
 *  Reference: A Visual C++ Threads FAQ by Doug Harrison ( http://members.cox.net/doug_web/threads.htm )
 */
template<LPSECURITY_ATTRIBUTES ThreadAttributes = NULL, int Priority = THREAD_PRIORITY_NORMAL, size_t StackSize = 0>
struct mfc_thread
{
  static HANDLE create_thread(LPTHREAD_START_ROUTINE, LPVOID, LPDWORD)
  {
    BOOST_ASSERT(false);
    return 0;
  }

  static HANDLE create_thread(unsigned (__stdcall *)(LPVOID), void *, unsigned *)
  {
    BOOST_ASSERT(false);
    return 0;
  }

  static HANDLE create_thread(UINT (*fnThreadProc)(LPVOID), LPVOID lpParameter, LPDWORD pdwThreadid)
  {
    CWinThread * pThread = AfxBeginThread( fnThreadProc, lpParameter, Priority, StackSize, CREATE_SUSPENDED
      , ThreadAttributes );
    BOOST_ASSERT( pThread );

    if( pdwThreadid )
      *pdwThreadid = pThread->m_nThreadID;

    HANDLE worker_thread = NULL;
    ::DuplicateHandle( ::GetCurrentProcess(), pThread->m_hThread, ::GetCurrentProcess()
      , &worker_thread, 0, FALSE, DUPLICATE_SAME_ACCESS );

    pThread->ResumeThread();

    return worker_thread;
  }

  BOOST_STATIC_CONSTANT( detail::thread_routine_type, routine_type = detail::mfc_thread_routine );

};  // struct mfc_thread

#endif  // #if defined(_MFC_VER) && defined(_MT)

#if !defined(_ATL_MIN_CRT) && defined(_MT)
typedef crt_thread<NULL, THREAD_PRIORITY_NORMAL, 0>   default_thread;
#else
typedef win32_thread<NULL, THREAD_PRIORITY_NORMAL, 0> default_thread;
#endif

/**
 *  Thread specific information for the spawned thread.
 */
class thread_specific
{
private:
  struct thread_specific_info_
  {
    HANDLE abort_event;
    HANDLE caller_thread;
  } tsi_;

  static detail::afc_tss<thread_specific_info_> & tsi()
  {
    // Scoped static initialization is protected thread-safe through synchronization event
    // (afc_param_type.sync_event).
    //
    // @see afc_thread::afc_thread_proc_win32
    //
    // Reference: C++ scoped static initialization is not thread-safe, on purpose!
    // ( http://blogs.msdn.com/oldnewthing/archive/2004/03/08/85901.aspx )
    static detail::afc_tss<thread_specific_info_> static_thread_specific_info;
    return static_thread_specific_info;
  }

public:
  thread_specific()
  {
    tsi().set_value( &tsi_ );
  }

  void set(HANDLE abort_event, HANDLE caller_thread)
  {
    thread_specific_info_ * tsi_ptr = tsi().get_value();
    if( tsi_ptr )
    {
      tsi_ptr->abort_event = abort_event;
      tsi_ptr->caller_thread = caller_thread;
    }
  }

  static bool check_abort()
  {
    thread_specific_info_ * tsi_ptr = tsi().get_value();
    return tsi_ptr
      ? WAIT_OBJECT_0 == ::WaitForSingleObject( tsi_ptr->abort_event, 0 )
      : false;
  }

  static HANDLE get_caller_thread_handle()
  {
    thread_specific_info_ * tsi_ptr = tsi().get_value();
    return tsi_ptr
      ? tsi_ptr->caller_thread
      : ::GetCurrentThread(); // Called from the main thread.
  }
};  // class thread_specific

/**
 * Default exception handler.
 */
template<typename R>
struct no_throw_exception
{
  typedef R result_type;

  template<typename TFxn> inline
  result_type operator ()(TFxn fxn, UINT & error_code) const
  {
    return fxn();
  }
};

template<>
struct no_throw_exception<void>
{
  typedef void result_type;

  template<typename TFxn> inline
  result_type operator ()(TFxn fxn, UINT & error_code) const
  {
    fxn();
  }
};

} // namespace afc

namespace afc { namespace detail
{

#if defined(DEBUG) || defined(_DEBUG)
struct debug_helper
{
  static unsigned thread_counter()
  {
    static unsigned static_count = 0;
    return static_count++;
  }
};
#endif  // #if defined(DEBUG) || defined(_DEBUG)

/**
 *  Thread parameter packing and thread procedure.
 */
template<typename R>
struct afc_thread
{
  typedef R                   result_type;
  typedef on_completion_t<R>  on_completion_type;

  typedef struct tag_afc_param
  {
    on_completion_type  on_comp;
    boost::function1<R, UINT &> user_fxn;
    HANDLE sync_event;
    boost::shared_ptr<void> shared_abort_event;
    boost::shared_ptr<void> shared_caller_thread;

    tag_afc_param(boost::function1<R, UINT &> const & user_fxn_in, on_completion_type const & on_comp_in, HANDLE sync_event_in
      , boost::shared_ptr<void> shared_abort_event_in, boost::shared_ptr<void> shared_caller_thread_in)
      : user_fxn( user_fxn_in ), on_comp( on_comp_in ), sync_event( sync_event_in )
      , shared_abort_event( shared_abort_event_in ), shared_caller_thread( shared_caller_thread_in )
    {
    }
  } afc_param_type;

  static DWORD WINAPI afc_thread_proc_win32(LPVOID parameter)
  {
    std::auto_ptr<afc_param_type> afc_param_ptr( static_cast<afc_param_type *>( parameter ) );

    thread_specific tss;
    tss.set( afc_param_ptr->shared_abort_event.get(), afc_param_ptr->shared_caller_thread.get() );

    :SetEvent( afc_param_ptr->sync_event );

    try
    {
      UINT error_code = 0;
      result_type result = afc_param_ptr->user_fxn( error_code );
      afc_param_ptr->on_comp( result, error_code );
    }
    catch(...)
    {
      afc_param_ptr->on_comp( BOOST_DEDUCED_TYPENAME boost::remove_reference<result_type>::type(), AFC_ERROR_UNHANDLED_EXCEPTION );
    }

#if defined(DEBUG) || defined(_DEBUG)
    return AFC_EXIT_CODE_NORMAL + debug_helper::thread_counter();
#else
    return AFC_EXIT_CODE_NORMAL;
#endif
  }

  static unsigned __stdcall afc_thread_proc_crt(LPVOID parameter)
  {
    return static_cast<unsigned>( afc_thread_proc_win32( parameter ) );
  }

  static UINT afc_thread_proc_mfc(LPVOID parameter)
  {
    return static_cast<UINT>( afc_thread_proc_win32( parameter ) );
  }
};  // struct afc_thread

/**
 *  Thread parameter packing and thread procedure. void return specialization.
 */
template<>
struct afc_thread<void>
{
  typedef void                  result_type;
  typedef on_completion_t<void> on_completion_type;

  typedef struct tag_afc_param
  {
    on_completion_type  on_comp;
    boost::function1<void, UINT &> user_fxn;
    HANDLE sync_event;
    boost::shared_ptr<void> shared_abort_event;
    boost::shared_ptr<void> shared_caller_thread;

    tag_afc_param(boost::function1<void, UINT &> const & user_fxn_in, on_completion_type const & on_comp_in, HANDLE sync_event_in
      , boost::shared_ptr<void> shared_abort_event_in, boost::shared_ptr<void> shared_caller_thread_in)
      : user_fxn( user_fxn_in ), on_comp( on_comp_in ), sync_event( sync_event_in )
      , shared_abort_event( shared_abort_event_in ), shared_caller_thread( shared_caller_thread_in )
    {
    }
  } afc_param_type;

  static DWORD WINAPI afc_thread_proc_win32(LPVOID parameter)
  {
    std::auto_ptr<afc_param_type> afc_param_ptr( static_cast<afc_param_type *>( parameter ) );

    thread_specific tss;
    tss.set( afc_param_ptr->shared_abort_event.get(), afc_param_ptr->shared_caller_thread.get() );

    ::SetEvent( afc_param_ptr->sync_event );

    UINT error_code = 0;
    try
    {
      afc_param_ptr->user_fxn( error_code );
    }
    catch(...)
    {
      error_code = AFC_ERROR_UNHANDLED_EXCEPTION;
    }
    afc_param_ptr->on_comp( error_code );

#if defined(DEBUG) || defined(_DEBUG)
    return AFC_EXIT_CODE_NORMAL + debug_helper::thread_counter();
#else
    return AFC_EXIT_CODE_NORMAL;
#endif
  }

  static unsigned __stdcall afc_thread_proc_crt(LPVOID parameter)
  {
    return static_cast<unsigned>( afc_thread_proc_win32( parameter ) );
  }

  static UINT afc_thread_proc_mfc(LPVOID parameter)
  {
    return static_cast<UINT>( afc_thread_proc_win32( parameter ) );
  }
};  // struct afc_thread<void>

/**
 *  AFC proxy base class to access and control the spawned worker thread.
 */
class afc_proxy_base
{
protected:
  DWORD worker_thread_id_;
  boost::shared_ptr<void> shared_worker_thread_;
  boost::shared_ptr<void> shared_abort_event_;

public:
  afc_proxy_base()
    : worker_thread_id_( 0 ), shared_worker_thread_()
    , shared_abort_event_( ::CreateEvent( NULL, TRUE, FALSE, NULL ), &::CloseHandle )
  {
  }

  HANDLE get_thread_handle() const
  {
    return shared_worker_thread_.get();
  }

  DWORD get_thread_id() const
  {
    return worker_thread_id_;
  }

  BOOL abort() const
  {
    return ::SetEvent( shared_abort_event_.get() );
  }

  bool is_running() const
  {
    DWORD res = ::WaitForSingleObject( shared_worker_thread_.get(), 0 );
    BOOST_ASSERT( WAIT_FAILED != res );
    return ( WAIT_OBJECT_0 != res );
  }

  BOOL set_thread_priority(int priority) const
  {
    return ::SetThreadPriority( shared_worker_thread_.get(), priority );
  }

  int get_thread_priority() const
  {
    return ::GetThreadPriority( shared_worker_thread_.get() );
  }

  DWORD suspend() const
  {
    return ::SuspendThread( shared_worker_thread_.get() );
  }

  DWORD resume() const
  {
    return ::ResumeThread( shared_worker_thread_.get() );
  }

  BOOL terminate(DWORD exit_code = AFC_EXIT_CODE_TERMINATION) const
  {
    return ::TerminateThread( shared_worker_thread_.get(), exit_code );
  }

  DWORD wait(DWORD timeout)
  {
    return ::WaitForSingleObject( shared_worker_thread_.get(), timeout );
  }

  DWORD wait_with_message_loop(DWORD timeout)
  {
    DWORD res = 0, count_begin = ::GetTickCount();
    MSG msg = { 0, };
    HANDLE handles[] = { shared_worker_thread_.get() };

    while( true )
    {
      DWORD elapsed = ::GetTickCount() - count_begin;
      if( timeout < elapsed )
        elapsed = timeout;

      res = ::MsgWaitForMultipleObjects( 1, handles, FALSE, timeout - elapsed, QS_ALLINPUT );
      if( WAIT_OBJECT_0 + 1 == res )
      { // New messages have been arrived.

        while( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
          BOOL unicode = ::IsWindowUnicode( msg.hwnd );

          if( 0 >= ( unicode ? ::GetMessageW( &msg, NULL, 0, 0 ) : ::GetMessageA( &msg, NULL, 0, 0 ) ) )
          { // If it is a quit message or error, returns WAIT_TIMEOUT immediately.

            return WAIT_TIMEOUT;
          }

          // Otherwise, translate and dispatch the message.
          ::TranslateMessage( &msg );
          if( unicode )
            ::DispatchMessageW( &msg );
          else
            ::DispatchMessageA( &msg );

          if( WAIT_OBJECT_0 == ::WaitForSingleObject( shared_worker_thread_.get(), 0 ) )
          { // Worker thread is terminated.

            return WAIT_OBJECT_0;
          }
        }
      }
      else
      {
        return res;
      }
    } // while( true )
  }
};  // class afc_proxy_base

/**
 *  Parameterized AFC proxy class template.
 */
template<typename R, typename ThreadTraits>
class afc_proxy_t : public afc_proxy_base
{
public:
  typedef typename afc_thread<R>::result_type         result_type;
  typedef typename afc_thread<R>::on_completion_type  on_completion_type;
  typedef typename afc_thread<R>::afc_param_type      afc_param_type;
  typedef ThreadTraits                                thread_traits_type;
  typedef boost::function0<result_type>               user_function_type;

private:
  template<typename ExceptionHandler>
    bool launch(user_function_type const & user_fxn, ExceptionHandler const & exception_handler, on_completion_type const & on_comp)
  {
    HANDLE caller_thread = NULL;
    if( ::DuplicateHandle( ::GetCurrentProcess(), ::GetCurrentThread(), ::GetCurrentProcess()
      , &caller_thread, 0, FALSE, DUPLICATE_SAME_ACCESS) )
    {
      boost::shared_ptr<void> shared_caller_thread( caller_thread, &::CloseHandle );
      boost::shared_ptr<void> shared_sync_event( ::CreateEvent( NULL, TRUE, FALSE, NULL ), &CloseHandle );

      if( shared_abort_event_.get() && shared_sync_event.get() && shared_caller_thread.get() )
      {
        std::auto_ptr<afc_param_type> afc_param_ptr(
#if BOOST_WORKAROUND(BOOST_MSVC, <= 1300)
          new afc_param_type( boost::bind( boost::type<R>(), exception_handler, user_fxn, boost::arg<1>() )
          , on_comp, shared_sync_event.get(), shared_abort_event_, shared_caller_thread ) );
#else
          new afc_param_type( boost::bind( exception_handler, user_fxn, boost::arg<1>() )
          , on_comp, shared_sync_event.get(), shared_abort_event_, shared_caller_thread ) );
#endif
        if( afc_param_ptr.get() )
        { // Succeeded to allocate APC parameters on heap.

          unsigned thrdaddr = 0;

          // Determines the right thread routine type according to the thread
          // trait specified then creates a worker thread.
          switch( thread_traits_type::routine_type )
          {
          case detail::win32_thread_routine:
            shared_worker_thread_.reset( thread_traits_type::create_thread( &detail::afc_thread<R>::afc_thread_proc_win32
              , static_cast<LPVOID>( afc_param_ptr.get() ), &worker_thread_id_ ), &::CloseHandle );
            break;

          case detail::crt_thread_routine:
            shared_worker_thread_.reset( thread_traits_type::create_thread( &detail::afc_thread<R>::afc_thread_proc_crt
              , static_cast<LPVOID>( afc_param_ptr.get() ), &thrdaddr ), &::CloseHandle );
            worker_thread_id_ = thrdaddr;
            break;

          case detail::mfc_thread_routine:
            shared_worker_thread_.reset( thread_traits_type::create_thread( &detail::afc_thread<R>::afc_thread_proc_mfc
              , static_cast<LPVOID>( afc_param_ptr.get() ), &worker_thread_id_ ), &::CloseHandle );
            break;
          }

          if( shared_worker_thread_.get() )
          { // Succeeded to create the worker thread.

            // Spawned worker thread has the ownership of the heap allocated AFC parameters now.
            afc_param_ptr.release();

            // Waits till the spawned thread procedure is started and completes its thread
            // specific local storage initialization.
            DWORD res = ::WaitForSingleObject( shared_sync_event.get(), 10000 );
            BOOST_ASSERT( WAIT_OBJECT_0 == res );
            return ( WAIT_OBJECT_0 == res ) ? true : false;
          }
        }
      }
    }

    return false;
  }

public:
  template<typename ExceptionHandler>
  afc_proxy_t const & operator ()(user_function_type const& user_fxn, ExceptionHandler const & exception_handler
    , on_completion_type const & on_comp)
  {
    this->launch( user_fxn, exception_handler, on_comp );
    return *this;
  }
};  // class afc_proxy_t

} } // namespace afc::detail

namespace afc
{

/**
 *  typedef for AFC proxy.
 */
typedef detail::afc_proxy_base proxy;

/**
 *  Helper function templates to create the parameterized AFC proxy class template instance.
 *  Generic null-nary function object versions.
 */
template<typename ThreadTraits, typename R, typename F, typename ExceptionHandler>
detail::afc_proxy_t<R, ThreadTraits> inline
launch(F f, ExceptionHandler const & exception_handler, on_completion_t<R> const & on_comp)
{
  return detail::afc_proxy_t<R, ThreadTraits>()( f, exception_handler, on_comp );
}

template<typename ThreadTraits, typename R, typename F>
detail::afc_proxy_t<R, ThreadTraits> inline
launch(F f, on_completion_t<R> const & on_comp)
{
  return detail::afc_proxy_t<R, ThreadTraits>()( f, no_throw_exception<R>(), on_comp );
}

template<typename ThreadTraits, typename F>
detail::afc_proxy_t<typename F::result_type, ThreadTraits> inline
launch(F f)
{
  typedef typename F::result_type result_type;
  return detail::afc_proxy_t<result_type, ThreadTraits>()( f, no_throw_exception<result_type>(), no_completion<result_type>() );
}

/**
 *  Helper function templates to create the parameterized AFC proxy class template instance.
 *  Function signatures versions up to AFC_LAUNCHER_MAX_NUM_ARGS arguments.
 */
#define AFC_LAUNCHER_MAX_NUM_ARGS  8
#define AFC_LAUNCHER_PARAM(J,I,D)  BOOST_PP_CAT(T,I) BOOST_PP_CAT(a,I)

#define AFC_LAUNCHER_HELPER(z, n, unused) \
template<typename ThreadTraits, typename R, typename ExceptionHandler \
BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (*fxn)(BOOST_PP_ENUM_PARAMS(n, T)) BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY) \
           , ExceptionHandler const & exception_handler, on_completion_t<R> const & on_comp) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , exception_handler, on_comp ); \
} \
template<typename ThreadTraits, typename R, typename U, typename T, typename ExceptionHandler \
  BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (U::*fxn)(BOOST_PP_ENUM_PARAMS(n, T)), T t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY) \
  , ExceptionHandler const & exception_handler, on_completion_t<R> const & on_comp ) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn, t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , exception_handler, on_comp ); \
} \
template<typename ThreadTraits, typename R, typename U, typename T, typename ExceptionHandler \
  BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (U::*fxn)(BOOST_PP_ENUM_PARAMS(n, T)) const, T t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY) \
  , ExceptionHandler const & exception_handler, on_completion_t<R> const & on_comp ) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn, t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , exception_handler, on_comp ); \
} \
template<typename ThreadTraits, typename R \
BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (*fxn)(BOOST_PP_ENUM_PARAMS(n, T)) BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY) \
  , on_completion_t<R> const & on_comp) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , no_throw_exception<R>(), on_comp ); \
} \
template<typename ThreadTraits, typename R, typename U, typename T \
BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (U::*fxn)(BOOST_PP_ENUM_PARAMS(n, T)), T t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY) \
  , on_completion_t<R> const & on_comp) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn, t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , no_throw_exception<R>(), on_comp ); \
} \
template<typename ThreadTraits, typename R, typename U, typename T \
BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (U::*fxn)(BOOST_PP_ENUM_PARAMS(n, T)) const, T t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY) \
           , on_completion_t<R>  const & on_comp) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn, t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , no_throw_exception<R>(), on_comp ); \
} \
template<typename ThreadTraits, typename R \
BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (*fxn)(BOOST_PP_ENUM_PARAMS(n, T)) BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY)) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , no_throw_exception<R>(),  no_completion<R>() ); \
} \
template<typename ThreadTraits, typename R, typename U, typename T \
BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (U::*fxn)(BOOST_PP_ENUM_PARAMS(n, T)), T t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY)) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn, t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , no_throw_exception<R>(),  no_completion<R>() ); \
} \
template<typename ThreadTraits, typename R, typename U, typename T \
BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)> \
detail::afc_proxy_t<R, ThreadTraits> inline \
launch(R (U::*fxn)(BOOST_PP_ENUM_PARAMS(n, T)) const, T t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n,AFC_LAUNCHER_PARAM,BOOST_PP_EMPTY)) \
{ \
  return detail::afc_proxy_t<R, ThreadTraits>()( boost::bind( fxn, t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a) ) \
  , no_throw_exception<R>(), no_completion<R>() ); \
}

#define  BOOST_PP_LOCAL_MACRO(n)    AFC_LAUNCHER_HELPER(~, n, ~)
#define  BOOST_PP_LOCAL_LIMITS      (0, AFC_LAUNCHER_MAX_NUM_ARGS - 1)
#include BOOST_PP_LOCAL_ITERATE()

#undef AFC_LAUNCHER_HELPER
#undef AFC_LAUNCHER_PARAM
#undef AFC_LAUNCHER_MAX_NUM_ARGS

} // namespace afc

#endif  // #ifndef AFC_ASYNCHRONOUS_FUNCTION_CALL_HEADER
