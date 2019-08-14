#include "CursATE/extractConfig.hpp"
#include <boost/program_options.hpp>
#include <iostream>

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
    ("parse-json-by-json", "parses JSON until its end; does not need EOLs; may break on invalid JSON in the middle (default: off)")
    ("parse-json-by-line", "assume one JSON per line; will not work for pretty-printed JSONs; will recover from syntax errors at EOLs (defualt: on)")
    ("silent-tags", po::value<std::string>(), "JSON array of paths to tags that names shall not be displayed")
    ("peiority-tags", po::value<std::string>(), "JSON array of path to tags, that shall be displayed as first in line (prioirty by order in array)")
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
    BUT_THROW(InvalidConfig, "error parsing input options: " << ex.what() << "\n" << desc);
  }
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

  return Config{};
}

}
