#pragma once
#include "LogATE/SequenceNumber.hpp"
#include "LogATE/Log.hpp"
#include "LogATE/Tree/Logs.hpp"
#include "LogATE/Tree/Path.hpp"
#include <But/Exception.hpp>
#include <But/Mpl/FreeOperators.hpp>
#include <string>
#include <vector>

namespace LogATE::Tree
{

class Node;
using NodePtr = But::NotNullUnique<Node>;
using NodeShPtr = But::NotNullShared<Node>;


class Node
{
public:
  BUT_DEFINE_EXCEPTION(Error, But::Exception, "log node error");

  struct Type final { std::string value_; };
  struct Name final { std::string value_; };
  using TrimFields = std::vector<Path>;
  using Children = std::vector<NodeShPtr>;

  virtual ~Node() = default;

  Node(Node const&) = delete;
  Node const& operator=(Node const&) = delete;
  Node(Node&&) = delete;
  Node const& operator=(Node&&) = delete;

  virtual void insert(Log const& log) = 0;

  virtual Children children() const = 0;
  virtual void add(NodePtr node) = 0;

  auto const& name() const { return name_; }
  auto const& type() const { return type_; }
  auto const& trimFields() const { return trimFields_; }

  Logs& logs()             { return logs_; }
  Logs const& logs() const { return logs_; }

  void pruneUpTo(const SequenceNumber sn);

protected:
  Node(Type type, Name name, TrimFields trimFields):
      type_{ std::move(type) },
      name_{ std::move(name) },
      trimFields_{ std::move(trimFields) }
  { }

  Logs logs_;

private:
  const Type type_;
  const Name name_;
  const TrimFields trimFields_;
};


BUT_MPL_FREE_OPERATORS_COMPARE(Node::Name, .value_)
BUT_MPL_FREE_OPERATORS_COMPARE(Node::Type, .value_)

}
