/*
 * Copyright 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jlc/cmdline.hpp>
#include <jlc/command.hpp>

int
main(int argc, char ** argv)
{
  jlm::JlcCommandLineParser commandLineParser;
  auto & commandLineOptions = commandLineParser.ParseCommandLineArguments(argc, argv);

  auto pgraph = generate_commands(commandLineOptions);
  pgraph->Run();

  return 0;
}
