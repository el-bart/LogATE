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
    ("silent-tags", po::value<std::string>(), "JSON array of paths to tags that names shall not be displayed")
    ("priority-tags", po::value<std::string>(), "JSON array of path to tags, that shall be displayed as first in line (prioirty by order in array)")
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


void setPort(Config& cfg, po::variables_map const& vm)
{
  if( vm.count("port") == 0 )
    return;
  cfg.port_.value_ = vm["port"].as<uint16_t>();
}


void setParsingMode(Config& cfg, po::variables_map const& vm)
{
  auto mode = 0x00;
  if( vm.count("parse-by-json") )
    mode |= 0x01;
  if( vm.count("parse-by-line") )
    mode |= 0x10;

  switch(mode)
  {
    case 0x00: return;
    case 0x01: cfg.jsonParsingMode_ = LogATE::Net::TcpServer::JsonParsingMode::ParseToEndOfJson; return;
    case 0x10: cfg.jsonParsingMode_ = LogATE::Net::TcpServer::JsonParsingMode::HardBreakOnNewLine; return;
    case 0x11: BUT_THROW(InvalidConfig, "parsing by JSON and parsing by line cannot be set at the same time");
  }
  BUT_ASSERT(!"code never reaches here");
}


void setKeyPath(Config& cfg, po::variables_map const& vm)
{
  if( not vm.count("key-path") )
    return;
  cfg.keyPath_ = LogATE::Tree::Path::parse( vm["key-path"].as<std::string>() );
}


auto toVectorOfString(std::string const& jsonIn)
{
  const auto json = nlohmann::json::parse(jsonIn);
  std::vector<std::string> out;
  for(auto& e: json)
    out.push_back( e.get<std::string>() );
  return out;
}

void setSilentTags(Config& cfg, po::variables_map const& vm)
{
  if( not vm.count("silent-tags") )
    return;
  cfg.silentTags_ = LogATE::Printers::OrderedPrettyPrint::SilentTags{ toVectorOfString( vm["silent-tags"].as<std::string>() ) };
}


void setPriorityTags(Config& cfg, po::variables_map const& vm)
{
  if( not vm.count("priority-tags") )
    return;
  cfg.priorityTags_ = LogATE::Printers::OrderedPrettyPrint::PriorityTags{ toVectorOfString( vm["priority-tags"].as<std::string>() ) };
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
    Config cfg;
    setPort(cfg, vm);
    setParsingMode(cfg, vm);
    setKeyPath(cfg, vm);
    setSilentTags(cfg, vm);
    setPriorityTags(cfg, vm);
    return cfg;
  }
  catch(std::exception const& ex)
  {
    BUT_THROW(InvalidConfig, "error processing command line parameters:\n" << ex.what() << "\n" << desc);
  }
}

}
