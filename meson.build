project(
  'fastrpc',
  'cpp',
  version: '8.1.14~lubosmeson-11',
  default_options: [
    'cpp_std=c++14',
    'warning_level=2',
    'buildtype=debugoptimized',
    'wrap_mode=nofallback',
    'default_library=both'
  ]
)

install_files = not meson.is_subproject()

# the trailing versioning for the installation of the shared lib
# ignored for static library builds
shlib_version = '13.1.0'

dependecies = [
  dependency('libxml-2.0')
]

includes = include_directories(
  'src',
  '.'
)

headers = [
  'src/frpc.h',
  'src/frpcvalue.h',
  'src/frpcarray.h',
  'src/frpcstruct.h',
  'src/frpcbinary.h',
  'src/frpcdatetime.h',
  'src/frpcstring.h',
  'src/frpcbool.h',
  'src/frpcint.h',
  'src/frpcdouble.h',
  'src/frpcpool.h',
  'src/frpcerror.h',
  'src/frpckeyerror.h',
  'src/frpctypeerror.h',
  'src/frpcindexerror.h',
  'src/frpcstreamerror.h',
  'src/frpclenerror.h',
  'src/frpcserverproxy.h',
  'src/frpcdatabuilder.h',
  'src/frpcmarshaller.h',
  'src/frpcunmarshaller.h',
  'src/frpcwriter.h',
  'src/frpcprotocolerror.h',
  'src/frpchttperror.h',
  'src/frpcencodingerror.h',
  'src/frpchttpio.h',
  'src/frpchttp.h',
  'src/frpchttpclient.h',
  'src/frpctreebuilder.h',
  'src/frpctreefeeder.h',
  'src/frpcfault.h',
  'src/frpcmethod.h',
  'src/frpcdefaultmethod.h',
  'src/frpcmethodregistry.h',
  'src/frpcserver.h',
  'src/frpcresponseerror.h',
  'src/frpcheadmethod.h',
  'src/frpcsocket.h',
  'src/frpcsocketunix.h',
  'src/frpcsocketwin.h',
  'src/frpcplatform.h',
  'src/frpcconnector.h',
  'src/frpcconverters.h',
  'src/frpcnull.h',
  'src/frpcbinmarshaller.h',
  'src/frpcxmlmarshaller.h',
  'src/frpcinternals.h',
  'src/frpccompare.h',
  'src/frpcb64marshaller.h',
  'src/frpcjsonmarshaller.h',
  'src/frpcb64writer.h',
  'src/frpcconfig.h',
  'src/frpcbinunmarshaller-extras.h',
  'src/frpcbinaryref.h',
  'src/frpclogging.h',
  'src/frpcstring_view.h',
]

sources = [
  'src/frpcvalue.cc',
  'src/frpcarray.cc',
  'src/frpcstruct.cc',
  'src/frpcbinary.cc',
  'src/frpcdatetime.cc',
  'src/frpcstring.cc',
  'src/frpcbool.cc',
  'src/frpcint.cc',
  'src/frpcdouble.cc',
  'src/frpcpool.cc',
  'src/frpcerror.cc',
  'src/frpckeyerror.cc',
  'src/frpctypeerror.cc',
  'src/frpcindexerror.cc',
  'src/frpcstreamerror.cc',
  'src/frpclenerror.cc',
  'src/frpcserverproxy.cc',
  'src/frpcdatabuilder.cc',
  'src/frpcmarshaller.cc',
  'src/frpcunmarshaller.cc',
  'src/frpcwriter.cc',
  'src/frpcbinmarshaller.cc',
  'src/frpcxmlmarshaller.cc',
  'src/frpcbinunmarshaller.cc',
  'src/frpcxmlunmarshaller.cc',
  'src/frpcprotocolerror.cc',
  'src/frpchttperror.cc',
  'src/frpcencodingerror.cc',
  'src/frpchttpio.cc',
  'src/frpchttp.cc',
  'src/frpchttpclient.cc',
  'src/frpctreebuilder.cc',
  'src/frpctreefeeder.cc',
  'src/frpcfault.cc',
  'src/frpc.cc',
  'src/frpcmethodregistry.cc',
  'src/frpcserver.cc',
  'src/frpcresponseerror.cc',
  'src/frpcconnector.cc',
  'src/frpcnull.cc',
  'src/frpcurlunmarshaller.cc',
  'src/frpcjsonmarshaller.cc',
  'src/frpcb64unmarshaller.cc',
  'src/frpcbase64.cc',
  'src/frpcb64writer.cc',
  'src/frpcconfig.cc',
  'src/frpccompare.cc',
  'src/frpcstring_view.cc',
]

frpc_version_h = configuration_data()
frpc_version_h.set('FASTRPC_MAJOR', 8)
frpc_version_h.set('FASTRPC_MINOR', 0)
frpc_version_h = configure_file(
  input: 'src/frpcversion.h.in',
  output: 'frpcversion.h',
  configuration: frpc_version_h
)

git_revision = 'no revision'
git = find_program('git', required: false)
fs = import('fs')
if git.found() and fs.is_dir('.git')
    res = run_command('git', 'describe', '--all', '--dirty', '--long', '--abbrev=40', check: true)
    git_revision = res.stdout().strip()
endif

add_project_arguments(
  '-DPACKAGE_VERSION="' + meson.project_version() + '"',
  language: ['c', 'cpp']
)

add_project_arguments(
  '-DPACKAGE_NAME="' + meson.project_name() + '"',
  language: ['c', 'cpp']
)

add_project_arguments(
  '-DGIT_REVISION="' + git_revision + '"',
  language: ['c', 'cpp']
)

if get_option('optimization') not in ['1', '2', '3']
  add_project_arguments(
    '-DDEBUG',
    language: ['c', 'cpp']
  )
endif

if get_option('debug')
  add_project_arguments(
    '-ggdb',
    language: ['c', 'cpp']
  )
endif

cxx = meson.get_compiler('cpp')
add_project_arguments(
  cxx.get_supported_arguments(
    [
      '-Wconversion',
      '-Wno-sign-conversion',
      '-Wnon-virtual-dtor',
      '-Woverloaded-virtual',
      '-Wctor-dtor-privacy',
      '-fno-omit-frame-pointer',
      '-ftemplate-backtrace-limit=0',
      '-Wextra-semi',
      '-Wuseless-cast',
      '-Wzero-as-null-pointer-constant',
      '-Wundef',
      '-Wlogical-op',
      '-Wpointer-arith',
      '-Wcast-qual',
      '-Wformat=2',
      '-Wformat-truncation',
      '-Wdisabled-optimization',
      '-Wnull-dereference',
      '-Wduplicated-cond',
      '-fdiagnostics-color=always',
      '-Werror=format',
    ]
  ),
  language: 'cpp'
)

lib = library(
  'fastrpc',
  sources: [sources, headers, frpc_version_h],
  dependencies: dependecies,
  include_directories: includes,
  install: install_files,
  version: shlib_version
)

if install_files
  install_headers([headers, frpc_version_h])
endif

pkg = import('pkgconfig')
pkg.generate(
  lib,
  name: 'libfastrpc',
  description: 'The seznam fastrpc library'
)

libfastrpc_dep = declare_dependency(
  link_with: lib,
  dependencies: dependecies,
  include_directories: [includes]
)
meson.override_dependency('libfastrpc', libfastrpc_dep)

test(
  'test_base64',
  executable(
    'test_base64',
    'test/base64.cc',
    include_directories: [includes],
    link_with: lib,
    dependencies: dependecies
  )
)

test(
  'test_protocol',
  executable(
    'test_protocol',
    'test/protocol.cc',
    include_directories: [includes],
    link_with: lib,
    dependencies: dependecies
  )
)

test(
  'test_marshallers',
  executable(
    'test_marshallers',
    'test/marshallers.cc',
    include_directories: [includes],
    link_with: lib,
    dependencies: dependecies
  ),
  args: ['testfile', meson.current_source_dir() + '/test/frpc.tests']
)

clang_tidy = find_program('clang-tidy', required: false)
if clang_tidy.found()
  input = files(sources + headers)
  command = [clang_tidy, '-quiet', '-p', meson.project_build_root(), '-header-filter', '^$']
  run_target('clang-tidy', command: command + input)
  run_target('clang-tidy-fix', command: command + ['-fix'] + input)
endif

if (get_option('docs'))
  doxygen = find_program('doxygen', required: true)
  dot = find_program('dot', required: true)

  input = []
  foreach file: sources + headers
    input += meson.project_source_root() / file
  endforeach

  conf_data = configuration_data()
  conf_data.set('PROJECT', meson.project_name())
  conf_data.set('VERSION', meson.project_version())
  conf_data.set_quoted('REVISION', git_revision)
  conf_data.set('SRCDIR', meson.project_source_root())
  conf_data.set('DOT_PATH', dot.full_path())
  conf_data.set('OUTPUT', meson.project_build_root())
  conf_data.set('INPUT', ' '.join(input))

  doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: conf_data
  )
  run_target('docs', command: [doxygen, doxyfile])
endif
