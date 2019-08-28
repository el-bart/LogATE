#pragma once
#include <future>
#include <functional>
#include <But/assert.hpp>

namespace LogATE::Utils::detail
{

struct TaskBase
{
  virtual ~TaskBase() = default;
  virtual void run() = 0;
};


template<typename Ret>
struct Task final: public TaskBase
{
  Task(std::promise<Ret> promise, std::function<Ret()> fun):
    promise_{ std::move(promise) },
    fun_{ std::move(fun) }
  { }

  void run() override
  {
    try
    {
      BUT_ASSERT(fun_);
      promise_.set_value( fun_() );
    }
    catch(...)
    {
      promise_.set_exception( std::current_exception() );
    }
  }

  std::promise<Ret> promise_;
  std::function<Ret()> fun_;
};


template<>
struct Task<void> final: public TaskBase
{
  Task(std::promise<void> promise, std::function<void()> fun):
    promise_{ std::move(promise) },
    fun_{ std::move(fun) }
  { }

  void run() override
  {
    try
    {
      BUT_ASSERT(fun_);
      fun_();
      promise_.set_value();
    }
    catch(...)
    {
      promise_.set_exception( std::current_exception() );
    }
  }

  std::promise<void> promise_;
  std::function<void()> fun_;
};


template<typename Ret>
auto makeTask(std::promise<Ret>&& promise, std::function<Ret()>&& fun)
{
  auto ptr = new Task<Ret>{ std::move(promise), std::move(fun) };
  return std::unique_ptr<TaskBase>{ptr};
}

}
