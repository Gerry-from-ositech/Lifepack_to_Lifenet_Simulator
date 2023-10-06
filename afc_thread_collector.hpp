//  Asynchronous Function Call Library - AFC Thread Collector.

//  Copyright JaeWook Choi 2007. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef AFC_THREAD_COLLECTOR_HEADER
#define AFC_THREAD_COLLECTOR_HEADER

#ifndef AFC_ASYNCHRONOUS_FUNCTION_CALL_HEADER
#error afc_thread_collector.hpp requires afc.hpp to be included first.
#endif

#pragma warning( disable: 4786 )  // warning C4786: '' : identifier was truncated to '255' characters in the debug information

#include <vector>
#include <map>
#include <algorithm>

#ifndef AFC_THREAD_COLLECTOR_WAIT_TIMEOUT
#define AFC_THREAD_COLLECTOR_WAIT_TIMEOUT   5000
#endif

namespace afc {  namespace detail
{

/**
 *  AFC thread collector class.
 *
 *  Maintains the list of AFC proxy which will be guaranteed to be terminated at exit
 *  either in normal way or in abnormal way by calling ::TerminateThread().
 *
 *  init() should be called in the main thread before creating a second thread which may
 *  use any of afc_thread_collector's public singleton access member functions, otherwise
 *  it is not thread-safe.
 */
class afc_thread_collector
{
private:
  typedef std::map<DWORD, afc_proxy_base> proxy_map_type;
  proxy_map_type proxy_map_;

  struct map_helper
  {
    typedef proxy_map_type::value_type map_value_type;

    static void abort_all(map_value_type const & val)
    {
      afc_proxy_base const & proxy = val.second;
      proxy.abort();
    }

    static void terminate_if_alive(map_value_type const & val)
    {
      afc_proxy_base const & proxy = val.second;
      if( proxy.is_running() )
        proxy.terminate( AFC_EXIT_CODE_TERMINATION );
    }

    static HANDLE back_inserter(map_value_type const & val)
    {
      afc_proxy_base const & proxy = val.second;
      return proxy.get_thread_handle();
    }
  };

  /**
   *  Proxy object to guard the access to the thread collector.
   */
  struct guard_proxy
  {
    boost::shared_ptr<void> shared_mutex_;
    afc_thread_collector & thread_collector_;

    guard_proxy(boost::shared_ptr<void> const & shared_mutex_in, afc_thread_collector & thread_collector_in)
      : shared_mutex_( shared_mutex_in ), thread_collector_( thread_collector_in )
    {
      ::WaitForSingleObject( shared_mutex_.get(), INFINITE );
    }

    ~guard_proxy()
    {
      ::ReleaseMutex( shared_mutex_.get() );
    }

    afc_thread_collector * operator ->() const
    {
      return &thread_collector_;
    }
  };

  // Guarded singleton pointer
  static guard_proxy guarded_instance()
  {
    return init();
  }

  bool wait_with_message_loop(DWORD timeout) const
  {
    DWORD res = 0, count_begin = ::GetTickCount();
    MSG msg = { 0, };

    DWORD count = proxy_map_.size();
    std::vector<HANDLE> handles( count );
    std::transform( proxy_map_.begin(), proxy_map_.end(), handles.begin(), &map_helper::back_inserter );

    while( true )
    {
      DWORD elapsed = ::GetTickCount() - count_begin;
      if( timeout < elapsed )
        elapsed = timeout;

      res = ::MsgWaitForMultipleObjects( count, &handles[0], FALSE, timeout - elapsed, QS_ALLINPUT );
      if( WAIT_OBJECT_0 + count == res )
      { // New messages have been arrived.

        while( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
          BOOL unicode = ::IsWindowUnicode( msg.hwnd );

          if( 0 >= ( unicode ? ::GetMessageW( &msg, NULL, 0, 0 ) : ::GetMessageA( &msg, NULL, 0, 0 ) ) )
          { // If it is a quit message or error, continues to check handles signaled immediately.

            break;
          }

          // Otherwise, translate and dispatch the message.
          ::TranslateMessage( &msg );
          if( unicode )
            ::DispatchMessageW( &msg );
          else
            ::DispatchMessageA( &msg );

          if( WAIT_OBJECT_0 == ::WaitForMultipleObjects( count, &handles[0], TRUE, 0 ) )
          { // All threads are terminated.

            return true;
          }
        }
      }
      else if( WAIT_TIMEOUT == res || WAIT_FAILED == res )
      { // Timeout or failed.

        BOOST_ASSERT( WAIT_FAILED != res );
        return false;
      }

      if( WAIT_OBJECT_0 == ::WaitForMultipleObjects( count, &handles[0], TRUE, 0 ) )
      { // All threads are terminated.

        return true;
      }
    } // while( true )

    return false;
  }

  void contract_(afc_proxy_base const & p)
  {
    proxy_map_type::iterator it_f = proxy_map_.find( p.get_thread_id() );
    if( proxy_map_.end() == it_f )
    {
      proxy_map_.insert( std::make_pair( p.get_thread_id(), p ) );
    }
  }

  void recede_(afc_proxy_base const & p)
  {
    proxy_map_type::iterator it_f = proxy_map_.find( p.get_thread_id() );
    if( proxy_map_.end() != it_f )
    {
      proxy_map_.erase( it_f );
    }
  }

  bool abort_and_wait_all_(DWORD timeout, bool force_termination) const
  {
    std::for_each( proxy_map_.begin(), proxy_map_.end(), &map_helper::abort_all );
    if( !wait_with_message_loop( timeout ) )
    {
      if( !force_termination )
        return false;

      std::for_each( proxy_map_.begin(), proxy_map_.end(), &map_helper::terminate_if_alive );
    }
    return true;
  }

  afc_thread_collector()
  {
  }

#if BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
public:
#endif
  ~afc_thread_collector()
  {
    abort_and_wait_all_( AFC_THREAD_COLLECTOR_WAIT_TIMEOUT, true );
  }

public:
  static guard_proxy init()
  {
    static boost::shared_ptr<void> static_mutex( ::CreateMutex( NULL, FALSE, NULL ), &::CloseHandle );
    BOOST_ASSERT( NULL != static_mutex.get() );

    static afc_thread_collector static_collector;

    return guard_proxy( static_mutex, static_collector );
  }

  static void contract(afc_proxy_base const & p)
  {
    guarded_instance()->contract_( p );
  }

  static void recede(afc_proxy_base const & p)
  {
    guarded_instance()->recede_( p );
  }

  static bool abort_and_wait_all(DWORD timeout = 1000, bool force_termination = false)
  {
    return guarded_instance()->abort_and_wait_all_( timeout, force_termination );
  }

}; // afc_thread_collector

} } // namespace afc::detail

namespace afc
{

  /**
   *  typedef for AFC thread collector.
   */
  typedef detail::afc_thread_collector thread_collector;

} // namespace afc

#endif  // #ifndef AFC_THREAD_COLLECTOR_HEADER
