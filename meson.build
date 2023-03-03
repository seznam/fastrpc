project('fastrpc', 'cpp',
       version: '8.1.13',
       default_options: [
         'cpp_std=c++14',
         'warning_level=3',
         'buildtype=debugoptimized'
       ])

pkg = import('pkgconfig')

lib_deps = [
  dependency('libxml-2.0')
]

FASTRPC_MAJOR = 8
FASTRPC_MINOR = 0

conf_data = configuration_data()
conf_data.set('FASTRPC_MAJOR', FASTRPC_MAJOR)
conf_data.set('FASTRPC_MINOR', FASTRPC_MINOR)

version_header = configure_file(input: 'src/frpcversion.h.in',
                                output: 'frpcversion.h',
                                configuration: conf_data)

headers = files('''
    src/frpc.h
    src/frpcvalue.h
    src/frpcarray.h
    src/frpcstruct.h
    src/frpcbinary.h
    src/frpcdatetime.h
    src/frpcstring.h
    src/frpcbool.h
    src/frpcint.h
    src/frpcdouble.h
    src/frpcpool.h
    src/frpcerror.h
    src/frpckeyerror.h
    src/frpctypeerror.h
    src/frpcindexerror.h
    src/frpcstreamerror.h
    src/frpclenerror.h
    src/frpcserverproxy.h
    src/frpcdatabuilder.h
    src/frpcmarshaller.h
    src/frpcunmarshaller.h
    src/frpcwriter.h
    src/frpcprotocolerror.h
    src/frpchttperror.h
    src/frpcencodingerror.h
    src/frpchttpio.h
    src/frpchttp.h
    src/frpchttpclient.h
    src/frpctreebuilder.h
    src/frpctreefeeder.h
    src/frpcfault.h
    src/frpcmethod.h
    src/frpcdefaultmethod.h
    src/frpcmethodregistry.h
    src/frpcserver.h
    src/frpcresponseerror.h
    src/frpcplatform.h
    src/frpcheadmethod.h
    src/frpcsocket.h
    src/frpcsocketunix.h
    src/frpcsocketwin.h
    src/frpcplatform.h
    src/frpcconnector.h
    src/frpcconverters.h
    src/frpcnull.h
    src/frpcbinmarshaller.h
    src/frpcxmlmarshaller.h
    src/frpcinternals.h
    src/frpccompare.h
    src/frpcb64marshaller.h
    src/frpcjsonmarshaller.h
    src/frpcb64writer.h
    src/frpcconfig.h
    src/frpcbinunmarshaller-extras.h
    src/frpcbinaryref.h
    src/frpclogging.h
    src/frpcstring_view.h
'''.split())

sources = files('''
    src/frpcvalue.cc
    src/frpcarray.cc
    src/frpcstruct.cc
    src/frpcbinary.cc
    src/frpcdatetime.cc
    src/frpcstring.cc
    src/frpcbool.cc
    src/frpcint.cc
    src/frpcdouble.cc
    src/frpcpool.cc
    src/frpcerror.cc
    src/frpckeyerror.cc
    src/frpctypeerror.cc
    src/frpcindexerror.cc
    src/frpcstreamerror.cc
    src/frpclenerror.cc
    src/frpcserverproxy.cc
    src/frpcdatabuilder.cc
    src/frpcmarshaller.cc
    src/frpcunmarshaller.cc
    src/frpcwriter.cc
    src/frpcbinmarshaller.cc
    src/frpcxmlmarshaller.cc
    src/frpcbinunmarshaller.cc
    src/frpcxmlunmarshaller.cc
    src/frpcprotocolerror.cc
    src/frpchttperror.cc
    src/frpcencodingerror.cc
    src/frpchttpio.cc
    src/frpchttp.cc
    src/frpchttpclient.cc
    src/frpctreebuilder.cc
    src/frpctreefeeder.cc
    src/frpcfault.cc
    src/frpc.cc
    src/frpcmethodregistry.cc
    src/frpcserver.cc
    src/frpcresponseerror.cc
    src/frpcconnector.cc
    src/frpcnull.cc
    src/frpcurlunmarshaller.cc
    src/frpcjsonmarshaller.cc
    src/frpcb64unmarshaller.cc
    src/frpcbase64.cc
    src/frpcb64writer.cc
    src/frpcconfig.cc
    src/frpccompare.cc
    src/frpcstring_view.cc
'''.split())

inc = include_directories('src')

lib = static_library('fastrpc', [sources, version_header],
                     dependencies: lib_deps,
                     include_directories: inc,
                     install: true)

# NOT IN SUBDIR! FRPC headers are prefixed 'frpc'
install_headers([headers, version_header])

pkg.generate(lib, name: 'libfastrpc')

libfastrpc_dep = declare_dependency(link_with: lib,
                                    dependencies: lib_deps,
                                    include_directories: inc)
meson.override_dependency('libfastrpc', libfastrpc_dep)

test_base64 = executable('test_base64', 'test/base64.cc',
                         include_directories: inc,
                         link_with: lib, dependencies: lib_deps)

test_protocol = executable('test_protocol', 'test/protocol.cc',
                           include_directories: inc,
                           link_with: lib, dependencies: lib_deps)

test_marshallers = executable('test_marshallers', 'test/marshallers.cc',
                              include_directories: inc,
                              link_with: lib, dependencies: lib_deps)

test('test_base64', test_base64)
test('test_protocol', test_protocol)
test('test_marshallers', test_marshallers, args: ['testfile', meson.current_source_dir() + '/test/frpc.tests'])
