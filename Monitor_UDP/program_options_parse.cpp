#include <boost/program_options.hpp>
#include <iostream>
#include "program_options_parse.h"
#include <boost/system/system_error.hpp>

namespace po = boost::program_options;

bool program_options_parse::po_parse(const int argc, char* argv[], std::string& config_path_)
{
	po::options_description general_options("General options");
	std::string config_path;
	general_options.add_options()
		("help,h", "Show help")
		("config_path,c", po::value<std::string>(&config_path), "Enter path to the configuration file and the full file name");

	po::variables_map vm;

	try 
	{
		po::store(po::parse_command_line(argc, argv, general_options), vm);
	}
	catch(std::exception ec)
	{
		std::cerr << "Program options error: " << ec.what() << std::endl;
		return false;
	}

	po::notify(vm);

	if(vm.count("help"))
	{
		std::cout << general_options << std::endl;
		return false;
	}
	
	if (vm.count("config_path"))
	{
		if(config_path!="")
		{
			config_path_ = config_path;
			return true;
		}
		return false;
	}

	std::cout << "You have not entered the program options. Enter -h or --help to view the list of parameters" << std::endl;
	return false;
}