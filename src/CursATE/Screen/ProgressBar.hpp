#pragma once
#include <But/Exception.hpp>
#include <But/NotNull.hpp>
#include <atomic>

namespace CursATE::Screen
{

class ProgressBar final
{
public:
  struct Monitor
  {
    Monitor() = default;

    Monitor(Monitor&) = default;
    Monitor& operator=(Monitor&) = default;

    Monitor(Monitor&&) = default;
    Monitor& operator=(Monitor&&) = default;

    virtual ~Monitor() = default;

    virtual uint64_t totalSize() const = 0;
    virtual uint64_t processed() const = 0;
    virtual bool done() const = 0;
    virtual bool aborted() const = 0;
    virtual void abort() = 0;
  };
  using MonitorShPtr = But::NotNullShared<Monitor>;

  BUT_DEFINE_EXCEPTION(WindowTooSmall, But::Exception, "window too small");
  BUT_DEFINE_EXCEPTION(InvalidInput, But::Exception, "invalid input");

  explicit ProgressBar(MonitorShPtr monitor): monitor_{ std::move(monitor) } { }

  ProgressBar(ProgressBar const&) = delete;
  ProgressBar& operator=(ProgressBar const&) = delete;
  ProgressBar(ProgressBar&&) = delete;
  ProgressBar& operator=(ProgressBar&&) = delete;

  bool process();

private:
  void waitForKey();
  MonitorShPtr monitor_;
};

}
