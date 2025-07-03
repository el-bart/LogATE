#include "CursATE/extractConfig.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <But/assert.hpp>

namespace po = boost::program_options;


namespace CursATE
{

namespace
{
auto prepareOptions()
{
  po::options_description desc{"CursATE options"};
  desc.add_options()
    ("help", "a hort desclimer on what to do when you spot a bear in the woods")
    ("port", po::value<uint16_t>(), "port service should listen on for JSON logs (default 4242)")
    ("key-path", po::value<std::string>(), "path to JSON value to be used as a key for sorting logs")
    ("parse-by-json", "parses JSON until its end; does not need EOLs; may break on invalid JSON in the middle (default: off)")
    ("parse-by-line", "assume one JSON per line; will not work for pretty-printed JSONs; will recover from syntax errors at EOLs (defualt: on)")
    ("silent-tags", po::value<std::string>(), "JSON array of tag names that shall be displayed as just a value")
    ("priority-tags", po::value<std::string>(), "JSON array of tag names, that shall be displayed as first in line (prioirty by order in array)")
    ("trim-fields", po::value<std::string>(), "JSON array of paths to tags, that shall not be displayed by default (visible only in detailed view)")
    ;
  return desc;
}

auto parseConfig(int argc, char** argv, po::options_description const& desc)
{
  try
  {
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    return vm;
  }
  catch(std::exception const& ex)
  {
    BUT_THROW(InvalidConfig, "error parsing input options:\n" << ex.what() << "\n" << desc);
  }
}


uint16_t getPort(po::variables_map const& vm)
{
  if( not vm.count("port") )
    return 4242;
  return vm["port"].as<uint16_t>();
}


auto getParsingMode(po::variables_map const& vm)
{
  auto mode = 0x00;
  if( vm.count("parse-by-json") )
    mode |= 0x01;
  if( vm.count("parse-by-line") )
    mode |= 0x10;

  switch(mode)
  {
    case 0x00: return LogATE::Net::TcpServer::JsonParsingMode::HardBreakOnNewLine; // default
    case 0x01: return LogATE::Net::TcpServer::JsonParsingMode::ParseToEndOfJson;
    case 0x10: return LogATE::Net::TcpServer::JsonParsingMode::HardBreakOnNewLine;
    case 0x11: BUT_THROW(InvalidConfig, "parsing by JSON and parsing by line cannot be set at the same time");
  }
  BUT_ASSERT(!"code never reaches here");
  throw std::logic_error{"getParsingMode(): code never reaches here"};
}


auto getKeyExtractor(po::variables_map const& vm)
{
  using LogATE::Tree::KeyExtractor;
  auto sourceFormat = KeyExtractor::SourceFormat::ISO8601_ns; // default
  // TODO: make SourceFormat a cmd-line param
  if( not vm.count("key-path") )
    BUT_THROW(InvalidConfig, "key path has not been specified");
  auto path = LogATE::Tree::Path::parse( vm["key-path"].as<std::string>() );
  return But::makeSharedNN<KeyExtractor>( std::move(path), sourceFormat );
}


auto toVectorOfString(std::string const& jsonIn)
{
  const auto json = nlohmann::json::parse(jsonIn);
  std::vector<std::string> out;
  for(auto& e: json)
    out.push_back( e.get<std::string>() );
  return out;
}



LogATE::Printers::OrderedPrettyPrint::SilentTags getSilentTags(po::variables_map const& vm)
{
  if( not vm.count("silent-tags") )
    return {};
  return LogATE::Printers::OrderedPrettyPrint::SilentTags{ toVectorOfString( vm["silent-tags"].as<std::string>() ) };
}


LogATE::Printers::OrderedPrettyPrint::PriorityTags getPriorityTags(po::variables_map const& vm)
{
  if( not vm.count("priority-tags") )
    return {};
  return LogATE::Printers::OrderedPrettyPrint::PriorityTags{ toVectorOfString( vm["priority-tags"].as<std::string>() ) };
}


auto toVectorOfPaths(std::string const& jsonIn)
{
  const auto json = nlohmann::json::parse(jsonIn);
  std::vector<LogATE::Tree::Path> out;
  for(auto& e: json)
    out.push_back( LogATE::Tree::Path::parse( e.get<std::string>() ) );
  return out;
}


LogATE::Tree::Node::TrimFields getTrimFields(po::variables_map const& vm)
{
  if( not vm.count("trim-fields") )
    return {};
  return LogATE::Tree::Node::TrimFields{ toVectorOfPaths( vm["trim-fields"].as<std::string>() ) };
}
}

But::Optional<Config> extractConfig(int argc, char** argv)
{
  const auto desc = prepareOptions();
  const auto vm = parseConfig(argc, argv, desc);

  if( vm.count("help") )
  {
    std::cout << desc << std::endl;
    return {};
  }

  try
  {
    Config cfg{
      .port_ = getPort(vm),
      .jsonParsingMode_ = getParsingMode(vm),
      .keyExtractor_ = getKeyExtractor(vm),
      .silentTags_ = getSilentTags(vm),
      .priorityTags_ = getPriorityTags(vm),
      .trimFields_ = getTrimFields(vm)
    };
    return cfg;
  }
  catch(std::exception const& ex)
  {
    BUT_THROW(InvalidConfig, "error processing command line parameters:\n" << ex.what() << "\n" << desc);
  }
}

}
