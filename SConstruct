import os, sys, glob

#Parameters that influence buid
#
# debug=(0|1)  default=1

#save invocation parameters
debug = ARGUMENTS.get('debug', 1) #if we get debug=0 on the cmdline we make a non-debug build

#create an environment
env = Environment()

conf = Configure(env)

##check for compiler and do sanity checks
if not conf.CheckCXX():
    print('!! Your compiler and/or environment is not correctly configured.')
    Exit(1)
##check for environment variables
if 'CXX' in os.environ:
    conf.env.Replace(CXX = os.environ['CXX'])
    print(">> Using compiler " + os.environ['CXX'])
if 'CXXFLAGS' in os.environ:
    conf.env.Append(CCFLAGS = os.environ['CXXFLAGS'])
    print(">> Appending custom build flags : " + os.environ['CXXFLAGS'])
if 'LDFLAGS' in os.environ:
    conf.env.Append(LINKFLAGS = os.environ['LDFLAGS'])
    print(">> Appending custom link flags : " + os.environ['LDFLAGS'])

##Check for boost##
if not conf.CheckLibWithHeader('boost_system', 'boost/bind.hpp', 'c++') :
    print 'Boost library must be installed!'
    Exit(1)

##check thrift##
if not conf.CheckLibWithHeader('thrift', 'thrift/Thrift.h', 'c++') :
    print 'thrift library must be installed!'
    Exit(1)

##check zlib##
if conf.CheckLibWithHeader('z', 'zlib.h', 'c++'):
    conf.env.Append(CPPDEFINES=['HAS_ZLIB_H'])
    conf.env.Append(HAVE_ZLIB=1)
else:
    conf.env.Append(HAVE_ZLIB=0)

##checks done##
env = conf.Finish()

extralibs = []

#os/compiler dependent settings
if os.name == 'posix':

  #this is needed because thrift libs itself dont include the 'thrift/' prefix
  thriftIncludeFile = env.FindFile('Thrift.h', ['/usr/include/thrift', '/usr/local/include/thrift'])
  if thriftIncludeFile==None:
    print "thift include dir not found in usual places"
    Exit(1)

  (thriftIncludeFileDir, thriftIncludeFileName) = os.path.split(thriftIncludeFile.get_abspath())
  env.Append(CPPPATH = [thriftIncludeFileDir])

  env.Append(CCFLAGS = Split('-pedantic -Wall -Wno-long-long -pipe -Wno-variadic-macros -fPIC'))
  if int(debug):
    env.Append(CCFLAGS = Split('-g -ggdb -O2'))
  else:
    env.Append(CCFLAGS = Split('-O3'))

  if env['HAVE_ZLIB']==1:
    extralibs += ['z']


#build thrift cassandra interface
SConscript(['thriftcassandra/SConscript'], exports={'env':env}, build_dir='build/thriftcassandra', duplicate=0)
Import('thriftcassandra')

#build agamemnon interface
SConscript(['agamemnon/SConscript'], exports={'env':env}, build_dir='build/agamemnon', duplicate=0)
Import('agamemnon')

Default(thriftcassandra, agamemnon)

#build examples
SConscript(['examples/SConscript'], exports=[{'env':env}, {'deplibs':agamemnon+thriftcassandra+extralibs}], build_dir='build/examples', duplicate=0)
Import('examples')
