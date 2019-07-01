#include "../libcommon/helper.h"

#include <cstdarg>
#include <cstdlib>

void panic(const char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    fputs("Error: ", stderr);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    fputs("\n", stderr);

    exit(1);
}

void parseArgs(int argc, char** argv, cmd_args& args){

	switch(argc){
	case 1:

	}

    if (argc > 1)
    {
        args.filename = argv[1];
        args.prob_bits = (argc>2) ? std::stoi(argv[2]):0;
    }else{
    	throw std::runtime_error("syntax main.exe <filename> [<probability_bits>]");
    }
}
