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
  struct Name final
  {
    std::string value_;
  };

  virtual Name name() const = 0;
  virtual Path processedField() const = 0;

  SequenceNumber first() const;
  SequenceNumber last() const;

  SequenceNumber insert(Log log) const;

  // pruneUpTo(SeqN)
  std::vector<Log> range(SequenceNumber begin, SequenceNumber end) const;
  std::vector<Log> range(SequenceNumber begin, size_t count) const;
  std::vector<But::NotNullShared<Node>> children() const;

protected:


private:
  mutable std::mutex mutex_;
  std::vector<Log> logs_;
  std::vector<But::NotNullShared<Node>> children_;
};

}
