#include "CursATE/extractConfig.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <But/assert.hpp>

namespace po = boost::program_options;
using KeyFormat = LogATE::Tree::KeyExtractor::SourceFormat;


namespace CursATE
{

namespace
{
auto possibleKeyFormats()
{
  return std::map<std::string, KeyFormat>{
    {"raw",        KeyFormat::Raw},
    {"iso8601-ns", KeyFormat::ISO8601_ns},
    {"unix",       KeyFormat::UNIX},
    {"unix-ms",    KeyFormat::UNIX_ms},
    {"unix-us",    KeyFormat::UNIX_us},
    {"unix-ns",    KeyFormat::UNIX_ns},
  };
}

auto toKeyFormat(std::string const& name)
{
  auto const pkf = possibleKeyFormats();
  auto const it = pkf.find(name);
  if( it == pkf.end() )
    BUT_THROW(InvalidConfig, "unkown key format: " << name);
  return it->second;
}

auto defaultKeyFormat() { return KeyFormat::ISO8601_ns; }

auto allKeyValues()
{
  std::ostringstream os;
  auto spacer = "";
  for(auto& e: possibleKeyFormats())
  {
    os << spacer << e.first;
    if( e.second == defaultKeyFormat() )
      os << " [default]";
    spacer = ", ";
  }
  return os.str();
}

auto prepareOptions()
{
  static const std::string keyFormatDescription = "format of the key: " + allKeyValues();
  po::options_description desc{"CursATE options"};
  desc.add_options()
    ("help", "a hort desclimer on what to do when you spot a bear in the woods")
    ("port", po::value<uint16_t>(), "port service should listen on for JSON logs (default 4242)")
    ("key-path", po::value<std::string>(), "path to JSON value to be used as a key for sorting logs")
    ("key-format", po::value<std::string>(), keyFormatDescription.c_str())
    ("parse-by-json", "parses JSON until its end; does not need EOLs; may break on invalid JSON in the middle (default: off)")
    ("parse-by-line", "assume one JSON per line; will not work for pretty-printed JSONs; will recover from syntax errors at EOLs (defualt: on)")
    ("silent-tags", po::value<std::string>(), "JSON array of tag names that shall be displayed as just a value")
    ("priority-tags", po::value<std::string>(), "JSON array of tag names, that shall be displayed as first in line (prioirty by order in array)")
    ("trim-fields", po::value<std::string>(), "JSON array of paths to tags, that shall not be displayed by default (visible only in detailed view)")
    ("dont-trim-key", "disables auto-trimming field defined as a key")
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
  auto const sourceFormat = vm.count("key-format") ? toKeyFormat( vm["key-format"].as<std::string>() ) : defaultKeyFormat();
  if( not vm.count("key-path") )
    BUT_THROW(InvalidConfig, "key path has not been specified");
  auto path = LogATE::Tree::Path::parse( vm["key-path"].as<std::string>() );
  return But::makeSharedNN<LogATE::Tree::KeyExtractor>( std::move(path), sourceFormat );
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

auto getOppConfig(po::variables_map const& vm)
{
  return LogATE::Printers::OrderedPrettyPrint::Config{
      .silentTags_ = getSilentTags(vm),
      .priorityTags_ = getPriorityTags(vm),
      .paddedFields_ = {} // TODO
  };
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
      .oppConfig_ = getOppConfig(vm),
      .trimFields_ = getTrimFields(vm)
    };
    if( not vm.count("dont-trim-key") )
        cfg.trimFields_.push_back( cfg.keyExtractor_->path() ); // processed key is anyway always on display
    return cfg;
  }
  catch(std::exception const& ex)
  {
    BUT_THROW(InvalidConfig, "error processing command line parameters:\n" << ex.what() << "\n" << desc);
  }
}

}
