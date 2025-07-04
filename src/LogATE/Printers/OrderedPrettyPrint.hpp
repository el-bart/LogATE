#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Utils/PrintableStringConverter.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <sstream>

namespace LogATE::Printers
{

class OrderedPrettyPrint final
{
public:
  struct SilentTags final
  {
    std::vector<std::string> tags_;
  };
  struct PriorityTags final
  {
    std::vector<std::string> tags_;
  };
  struct PaddedFields final
  {
    std::map<std::string, unsigned> padding_; // field name -> padding length
  };
  struct Config final
  {
    SilentTags silentTags_{};
    PriorityTags priorityTags_{};
    PaddedFields paddedFields_{};
  };

  OrderedPrettyPrint() = default;
  OrderedPrettyPrint(Config const& config);

  std::string operator()(LogATE::Log const& in) const;

private:
  void printNode(std::stringstream& ss, nlohmann::json const& value, unsigned padding) const;
  void printNode(std::stringstream& ss, std::string const& key, nlohmann::json const& value) const;
  void constructValue(std::stringstream& ss, nlohmann::json const& in, unsigned padding) const;
  void constructObject(std::stringstream& ss, nlohmann::json const& in, bool printBrace=true) const;
  void constructArray(std::stringstream& ss, nlohmann::json const& in) const;
  bool isSilent(std::string const& tag) const;
  unsigned paddingFor(std::string const& tag) const;

  const Utils::PrintableStringConverter printable_{};
  const std::unordered_set<std::string> silentTags_{};
  const PriorityTags priorityTags_{};
  const PaddedFields paddedFields_{};
};

}
