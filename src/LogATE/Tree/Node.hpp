#pragma once
#include "LogATE/SequenceNumber.hpp"
#include "LogATE/Log.hpp"
#include <string>
#include <mutex>

namespace LogATE::Tree
{

class Node
{
public:
  enum class CompareElement
  {
      Key,
      Value
  };
  struct Name final { std::string value_; };

  virtual ~Node() = default;

  Node(Node const&) = delete;
  Node const& operator=(Node const&) = delete;
  Node(Node&&) = delete;
  Node const& operator=(Node&&) = delete;

  virtual Name name() const = 0;
  virtual Path processedField() const = 0;

  Log first() const;
  Log last() const;

  SequenceNumber insert(Log log) const;

  // pruneUpTo(SeqN)
  std::vector<Log> range(SequenceNumber begin, SequenceNumber end) const;
  std::vector<Log> from(SequenceNumber first, size_t count) const;
  std::vector<Log> to(SequenceNumber last, size_t count) const;
  std::vector<But::NotNullRaw<Node>> children() const;

protected:
  Node(Name filterName, Path path);

private:
  virtual bool matchesFilter(Log const& log) = 0;

  mutable std::mutex mutex_;
  std::vector<Log> logs_;
  std::vector<But::NotNullUnique<Node>> children_;
};

}
