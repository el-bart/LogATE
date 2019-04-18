#pragma once
#include <But/Pattern/AbstractFactory.hpp>
#include <But/Exception.hpp>
#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{

class FilterFactory final
{
public:
  using Name = Node::Name;
  using Options = std::map<std::string, std::string>;
private:
  using Factory = But::Pattern::AbstractFactory<Node, Node::Name, Options>;
public:
  BUT_DEFINE_EXCEPTION(Error, But::Exception, "failed to build required filter");
  BUT_DEFINE_EXCEPTION(MissingOption, Error, "missing option");
  BUT_DEFINE_EXCEPTION(UnknownOption, Error, "unknown option");
  BUT_DEFINE_EXCEPTION(InvalidValue, Error, "invalid option value");

  using Type = Factory::Builder;

  FilterFactory();
  NodePtr build(Type type, Name name, Options options);

private:
  Factory factory_;
};

}
