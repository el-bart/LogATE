#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Tree/Logs.hpp"
#include "LogATE/Tree/Type.hpp"
#include "LogATE/Tree/Name.hpp"
#include "LogATE/Tree/Path.hpp"
#include "LogATE/Utils/WorkerThreads.hpp"
#include <But/NotNull.hpp>
#include <But/Exception.hpp>
#include <string>
#include <vector>

namespace LogATE::Tree
{

class Node;
using NodePtr = But::NotNullUnique<Node>;
using NodeShPtr = But::NotNullShared<Node>;
using NodeWeakPtr = std::weak_ptr<Node>;


class Node
{
public:
  BUT_DEFINE_EXCEPTION(Error, But::Exception, "log node error");

  using Type = Type;
  using Name = Name;
  using TrimFields = std::vector<Path>;
  using Children = std::vector<NodeShPtr>;

  virtual ~Node() = default;

  Node(Node const&) = delete;
  Node const& operator=(Node const&) = delete;
  Node(Node&&) = delete;
  Node const& operator=(Node&&) = delete;

  virtual bool insert(AnnotatedLog const& log) = 0;

  virtual Children children() const = 0;
  NodeShPtr add(NodePtr node);
  virtual std::shared_ptr<Node> remove(NodeShPtr node) = 0;

  auto const& name() const { return name_; }
  auto const& type() const { return type_; }

  TrimFields trimFields() const;

  But::NotNullShared<const Logs> logs() const { return logs_; }
  auto clogs() const { return logs(); }

  void pruneUpTo(Log::Key const& key);

protected:
  Node(Utils::WorkerThreadsShPtr workers, Type type, Name name, TrimFields trimFields):
      workers_{ std::move(workers) },
      type_{ std::move(type) },
      name_{ std::move(name) },
      trimFields_{ std::move(trimFields) }
  { }

  auto logs() { return logs_; }
  void trimAdditionalFields(TrimFields const& other);

  Utils::WorkerThreadsShPtr workers_;

private:
  virtual NodeShPtr addImpl(NodePtr node) = 0;

  const Type type_;
  const Name name_;

  But::NotNullShared<Logs> logs_{ But::makeSharedNN<Logs>() };

  mutable std::mutex trimFieldsMutex_;
  TrimFields trimFields_;
};

}
