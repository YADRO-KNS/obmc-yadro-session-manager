project('session-manager', ['cpp'],
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++2a',
    ],
    license: 'Apache-2.0',
    version: '1.0',
)

cxx = meson.get_compiler('cpp')
add_project_arguments(
    cxx.get_supported_arguments([
        '-DBOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT',
        '-DBOOST_ASIO_DISABLE_THREADS',
        '-DBOOST_ERROR_CODE_HEADER_ONLY',
        '-DBOOST_ALL_NO_LIB',
    ]),
    language : 'cpp')

boost = dependency('boost', required: true)
sdbusplus_dep = dependency('sdbusplus', required: true)
pdi_dep = dependency('phosphor-dbus-interfaces', required: true)
pl_dep = dependency('phosphor-logging', required: true)

systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_pkgconfig_variable('systemdsystemunitdir')
bindir = get_option('prefix') + '/' +get_option('bindir')

conf_data = configuration_data()
conf_data.set('MESON_INSTALL_PREFIX',get_option('prefix'))

executable('session-manager',
    'src/main.cpp',
    'src/manager.cpp',
    'src/session.cpp',
    dependencies: [
        boost,
        sdbusplus_dep,
        pdi_dep,
        pl_dep,
    ],
    include_directories: [
        'src',
    ],
    install: true,
)

configure_file(input : 'xyz.openbmc_project.SessionManager.service.in',
    output : 'xyz.openbmc_project.SessionManager.service',
    install_dir: systemd_system_unit_dir,
    configuration: conf_data,
    install : true
)
