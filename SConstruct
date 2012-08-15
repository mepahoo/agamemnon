import os, sys, glob

#Parameters that influence buid
#
# debug=(0|1)  default=1

#save invocation parameters
debug = ARGUMENTS.get('debug', 1) #if we get debug=0 on the cmdline we make a non-debug build

#create an environment
env = Environment()

#a list to store include dirs
include_directories = []

#os/compiler dependent settings
if os.name == 'posix':

  include_directories.append('/usr/local/include/thrift')

  env.Append(CCFLAGS = Split('-pedantic -Wall -Wno-long-long -pipe -Wno-variadic-macros -fPIC -fno-stack-protector'))
  if int(debug):
    env.Append(CCFLAGS = Split('-g -ggdb -O2'))
  else:
    env.Append(CCFLAGS = Split('-O3'))

env.Append(CPPPATH = include_directories)

#build thrift cassandra interface
SConscript(['thriftcassandra/SConscript'], exports={'env':env}, build_dir='build/thriftcassandra', duplicate=0)
Import('thriftcassandra')

#build agamemnon interface
SConscript(['agamemnon/SConscript'], exports={'env':env}, build_dir='build/agamemnon', duplicate=0)
Import('agamemnon')

Default(thriftcassandra, agamemnon)