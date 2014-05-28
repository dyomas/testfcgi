#include <stdlib.h>
#include <iostream>

#include "libev_wrapper.h"
#include "logger.h"


using namespace std;

LibevWrapper::LibevWrapper()
  : m_prepare(NULL)
  , mLoop(ev_default_loop ())
{
}

LibevWrapper::~LibevWrapper()
{
  // deinitialize loop
  ev_default_destroy();
}

void *LibevWrapper::init_timer(void *pobject, cbm_timer_t pmethod, const ev_tstamp delay)
{
  return m_init_timer(new timer_handler_props(this, pobject, pmethod), delay, .0);
}

void *LibevWrapper::init_timer_periodic(void *pobject, cbm_timer_t pmethod, const ev_tstamp interval)
{
  return m_init_timer(new timer_handler_props(this, pobject, pmethod), interval, interval);
}

void LibevWrapper::stop_timer(void *ptr)
{
  ev_timer *w = reinterpret_cast<ev_timer *>(ptr);
  const timer_handler_props *phnd = reinterpret_cast<const timer_handler_props *>(w->data);

  ev_timer_stop(mLoop, w);
  LOG_DEBUG("Delete timer with delay " << w->repeat << " (" << hex << reinterpret_cast<size_t>(w) << dec << ")");

  delete phnd;
  delete w;
}

void LibevWrapper::pause_timer(void* timer)
{
  ev_timer_stop(mLoop, reinterpret_cast<ev_timer *>(timer));
}

void LibevWrapper::restart_timer(void* timer)
{
  ev_timer_again(mLoop, reinterpret_cast<ev_timer *>(timer));
}

void *LibevWrapper::init_io_reader(void *pobject, cbm_io_t pmethod, const int fd)
{
  return m_init_io(new io_handler_props(this, pobject, pmethod), fd, EV_READ);
}

void *LibevWrapper::init_io_writer(void *pobject, cbm_io_t pmethod, const int fd)
{
  return m_init_io(new io_handler_props(this, pobject, pmethod), fd, EV_WRITE);
}

void LibevWrapper::stop_io_handler(void *ptr)
{
  ev_io *w = reinterpret_cast<ev_io *>(ptr);
  ev_io_stop(mLoop, w);

  const io_handler_props *phnd = reinterpret_cast<const io_handler_props *>(w->data);
  LOG_DEBUG("Delete " << (w->events & EV_READ ? "reader" : "writer") << " from fd " << w->fd << " (" << hex << reinterpret_cast<size_t>(w) << dec << ")");

  delete phnd;
  delete w;
}

void LibevWrapper::run()
{
  ev_loop (mLoop, 0);
}

void LibevWrapper::stop()
{
  ev_unloop (mLoop, EVUNLOOP_ONE);
}

LibevWrapper::timer_handler_props::timer_handler_props(LibevWrapper *_self, void *_pobject, cbm_timer_t _pmethod)
  : self(_self)
  , pobject(reinterpret_cast<LibevWrapper *>(_pobject))
  , pmethod(_pmethod)
{
}

LibevWrapper::io_handler_props::io_handler_props(LibevWrapper *_self, void *_pobject, cbm_io_t _pmethod)
  : self(_self)
  , pobject(reinterpret_cast<LibevWrapper *>(_pobject))
  , pmethod(_pmethod)
{
}

void *LibevWrapper::m_init_timer(timer_handler_props *phnd, const ev_tstamp delay, const ev_tstamp interval)
{
  ev_timer *w = new ev_timer();
  w->data = phnd;
  LOG_DEBUG("New" << (delay == interval ? " periodic" : "") << " timer with delay " << delay << " (" << hex << reinterpret_cast<size_t>(w) << dec << ")");

  ev_timer_init (w, m_timer_cb, delay, interval);
  ev_timer_start (mLoop, w);
  return w;
}

void *LibevWrapper::m_init_io(io_handler_props *phnd, const int fd, const int events)
{
  ev_io *w = new ev_io();
  w->data = phnd;
  LOG_DEBUG("New " << (events & EV_READ ? "reader" : "writer") << " on fd " << fd << " (" << hex << reinterpret_cast<size_t>(w) << dec << ")");

  ev_io_init (w, m_io_cb, fd, events);
  ev_io_start (mLoop, w);
  return w;
}

void LibevWrapper::m_timer_cb(struct ev_loop* mLoop, ev_timer *w, int /*revents*/)
{
  const timer_handler_props *phnd = reinterpret_cast<const timer_handler_props *>(w->data);
  bool ret_val = true;

  LOG_DEBUG("Time elapsed (" << hex << reinterpret_cast<size_t>(w) << dec << ")");
  LibevWrapper *self = phnd->self;
  ret_val = (phnd->pobject->*phnd->pmethod)();

  if (!ret_val)
  {
    ev_timer_stop(mLoop, w);
  }

  if (!ret_val || !w->repeat)
  {
    LOG_DEBUG("exit timer with delay " << w->repeat << " (" << hex << reinterpret_cast<size_t>(w) << dec << ")");
    delete phnd;
    delete w;
  }
}

void LibevWrapper::m_io_cb(struct ev_loop* mLoop, ev_io *w, int /*revents*/)
{
  const io_handler_props *phnd = reinterpret_cast<const io_handler_props *>(w->data);
  //NOTE LibevWrapper pointer copied because following method can call stop_io_handler and destruct io_handler_props object
  LibevWrapper *self = phnd->self;
  LOG_DEBUG((w->events & EV_READ ? "Read" : "Write") << " fd " << w->fd);
  (phnd->pobject->*phnd->pmethod)(w->fd);
}
