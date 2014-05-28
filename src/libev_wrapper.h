#ifndef __libev_wrapper_h__
#define __libev_wrapper_h__

#include <ev.h>
#include <list>

class LibevWrapper
{
  LibevWrapper(const LibevWrapper&);
  LibevWrapper& operator=(const LibevWrapper&);

public:
    typedef bool (LibevWrapper::*cbm_timer_t)();
    typedef void (LibevWrapper::*cbm_io_t)(const int);

    LibevWrapper();
    ~LibevWrapper();

    /*NOTE
    Остановка таймера сопровождается освобождением памяти.
    Таймер может остановить себя сам, вернув false, а может быть принудительно остановлен вызовом stop_timer.
    Двойной останов неминуемо ведет к ошибке Segmentation fault
    */
    void *init_timer(void */*pobject*/, cbm_timer_t /*pmethod*/, const ev_tstamp);
    void *init_timer_periodic(void */*pobject*/, cbm_timer_t /*pmethod*/, const ev_tstamp);
    void stop_timer(void *);
    void pause_timer(void* timer);
    void restart_timer(void* timer);

    void *init_io_reader(void */*pobject*/, cbm_io_t /*pmethod*/, const int /*fd*/);
    void *init_io_writer(void */*pobject*/, cbm_io_t /*pmethod*/, const int /*fd*/);
    void stop_io_handler(void *);

    void run();
    void stop();
  private:
    struct timer_handler_props
    {
      LibevWrapper *self;
      LibevWrapper *pobject;
      cbm_timer_t pmethod;
      timer_handler_props(LibevWrapper */*_self*/, void */*_pobject*/, cbm_timer_t /*_pmethod*/);
    };
    struct io_handler_props
    {
      LibevWrapper *self;
      LibevWrapper *pobject;
      cbm_io_t pmethod;
      io_handler_props(LibevWrapper */*_self*/, void */*_pobject*/, cbm_io_t /*_pmethod*/);
    };

    void *m_init_timer(timer_handler_props */*phnd*/, const ev_tstamp /*delay*/, const ev_tstamp /*interval*/);
    void *m_init_io(io_handler_props */*phnd*/, const int /*fd*/, const int /*events*/);

    static void m_timer_cb(EV_P_ ev_timer */*w*/, int /*revents*/);
    static void m_io_cb(EV_P_ ev_io */*w*/, int /*revents*/);

    ev_prepare *m_prepare;
    struct ev_loop* mLoop;
};

#endif //__libev_wrapper_h__

