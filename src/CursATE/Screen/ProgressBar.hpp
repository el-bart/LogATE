#pragma once
#include <But/Exception.hpp>
#include <But/NotNull.hpp>
#include <atomic>

namespace CursATE::Screen
{

class ProgressBar final
{
public:
  struct Monitor final
  {
    explicit Monitor(uint64_t totalSize): totalSize_{totalSize} { }

    const uint64_t totalSize_;
    std::atomic<uint64_t> processed_{0};
    std::atomic<bool> abort_{false};
    std::atomic<bool> done_{false};
  };
  using MonitorShPtr = But::NotNullShared<Monitor>;

  BUT_DEFINE_EXCEPTION(WindowTooSmall, But::Exception, "window too small");
  BUT_DEFINE_EXCEPTION(InvalidInput, But::Exception, "invalid input");

  explicit ProgressBar(MonitorShPtr monitor):
    monitor_{ std::move(monitor) }
  {
    if( monitor_->totalSize_ == 0 )
      BUT_THROW(InvalidInput, "total size cannot be 0");
  }

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
