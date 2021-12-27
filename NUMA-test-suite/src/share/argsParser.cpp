#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <cassert>
#include <vector>
#include <string>
#include <map>

// struct option {
//     const char *name;
//     int         has_arg;
//     int        *flag;
//     int         val;
// };

enum class OptionArgType {
  CHARS,
  INT,
  FLOAT
};


struct OptionStruct {
  const char* long_name;
  char short_name;
  const char* desc;
  int has_arg;
  void* store;
  OptionArgType type;
  OptionStruct() {}
  OptionStruct(const char* lname, char sname, const char* d, int arg, void* st, OptionArgType t):
    long_name(lname), short_name(sname), desc(d), has_arg(arg), store(st), type(t) {}
};


/* a getopt.h based argument parser */
class GetOpt {
private:
  int _argc;
  char** _argv;
  std::map<int, OptionStruct> _option_map;
  std::string _option_string;

  bool _parse_verbose;
public:
  
  GetOpt(int agrc, char** argv);
  void add_option(OptionStruct opt);
  void add_option(const char* lname, char sname, const char* d, int arg, void* st, OptionArgType t);
  void verbose_on() { _parse_verbose = true; }
  void verbose_off() { _parse_verbose = false; }
  option* option_array();
  void parse();
  void print_help();
};

GetOpt::GetOpt(int argc, char** argv) {
  _argc = argc;
  _argv = argv;
  _option_string = "";
  _parse_verbose = true;

  OptionStruct opt("help", 1, "print help message", no_argument, 0, OptionArgType::CHARS); // 1 is reserved as the value for "help" option
  add_option(opt);
}

void GetOpt::add_option(const char* lname, char sname, const char* d, int arg, void* st, OptionArgType t) {
  OptionStruct opt(lname, sname, d, arg, st, t);
  add_option(opt);
}

void GetOpt::add_option(OptionStruct opt_) {
  // option opt;
  // opt.name = opt_.long_name;
  // opt.has_arg = opt_.has_arg;
  // opt.flag = 0;
  // opt.val = (int)opt_.short_name;
  // _options.insert(_options.begin(), opt);
  _option_map[(int)opt_.short_name] = opt_;

  if (opt_.long_name != NULL) {
    _option_string += opt_.short_name;
    if (opt_.has_arg == required_argument) {
      _option_string += ':';
    }
    else if (opt_.has_arg == optional_argument) {
      _option_string += ':';
    }
  }
}

option* GetOpt::option_array() {
  int array_size = _option_map.size() + 1; // last item is (0, 0, 0, 0)
  option* options = new option[array_size];
  int i = 0;
  std::map<int, OptionStruct>::iterator it;
  for (it = _option_map.begin(); it != _option_map.end(); it++) {
    //int short_name = (int)it->first;
    OptionStruct opt_ = it->second;
    option opt;
    opt.name = opt_.long_name;
    opt.has_arg = opt_.has_arg;
    opt.flag = 0;
    opt.val = (int)opt_.short_name;

    options[i++] = opt;
  }

  option opt;
  opt.name = 0;
  opt.has_arg = 0;
  opt.flag = 0;
  opt.val = 0;
  options[i] = opt;
  
  return options;
}

void GetOpt::print_help() {
  std::string msg = "Options:\n";
  std::map<int, OptionStruct>::iterator it;
  for (it = _option_map.begin(); it != _option_map.end(); it++) {
    OptionStruct opt = it->second;
    char name[128];
    char line[1024];
    if (opt.short_name < 64) {
      sprintf(name, "--%s", opt.long_name);
    }
    else {
      sprintf(name, "--%s, -%c", opt.long_name, opt.short_name);
    }
    sprintf(line, "  %-16s %s\n", name, opt.desc);
    msg += line;
  }
  printf("%s", msg.c_str());
}

void GetOpt::parse() {
  if (_argc == 1) {
    print_help();
    exit(0);
  }
  
  int c;
  int option_index = 0;

  if (_parse_verbose) {
    printf("option string: %s\n", _option_string.c_str());
  }

  while (1) {
    c = getopt_long (_argc, _argv, _option_string.c_str(), //"t:i:r:b:c:v::h",
                     option_array(), &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    OptionStruct opt = _option_map[c];
    if (_parse_verbose) {
      printf ("option -%c with value '%s'\n", (char)c, optarg);
    }

    if (opt.store) {

      switch (opt.type) {
      case OptionArgType::CHARS: {
        strcpy((char*)opt.store, optarg);
        break;
      }
      case OptionArgType::INT: {
        *((int*)opt.store) = std::stoi(optarg);
        break;
      }
      case OptionArgType::FLOAT: {
        *((float*)opt.store) = std::stof(optarg);
        break;
      }
      }
    }
    
    
  }
  
}

int test(int argc, char** argv) {
  GetOpt parser(argc, argv);
  int id;
  float score;
  char name[100];


  // OptionStruct o1("id", 'i', "id of xxx", required_argument, &id, OptionArgType::INT);
  // parser.add_option(o1);
  parser.verbose_off();
  parser.add_option("id", 'i', "id of xxx", required_argument, &id, OptionArgType::INT);
  parser.add_option("name", 'n', "name of xxx", required_argument, name, OptionArgType::CHARS);
  parser.add_option("score", 's', "score of xxx", required_argument, &score, OptionArgType::FLOAT);
  parser.parse();
  
  printf("id: %d\n", id);
  printf("name: %s\n", name);
  printf("score: %.3f\n", score);
  return 0;
}
