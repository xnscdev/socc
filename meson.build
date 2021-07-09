project('socc', 'cpp', version: '0.0.1', license: 'GPL-3.0-or-later',
	default_options: ['cpp_std=gnu++17'])

socc_config = configuration_data()
socc_config.set_quoted('VERSION', meson.project_version())

if target_machine.cpu_family() in ['aarch64', 'alpha', 'ia64', 'mips64',
				   'ppc64', 'riscv64', 'sparc64', 'x86_64']
  socc_config.set('LP_WIDTH', '8')
elif target_machine.cpu_family() in ['arc', 'arm', 'mips', 'ppc', 'riscv32',
				     'sparc', 'x86']
  socc_config.set('LP_WIDTH', '4')
else
  error('Target CPU family ' + target_machine.cpu_family() + ' not supported')
endif

configure_file(input: 'config.h.in', output: 'config.h',
	       configuration: socc_config)

socc_inc = include_directories('.')

socc_src = [
  'diagnostics.cc',
  'lex.cc',
  'main.cc',
  'parse-expr.cc',
  'type.cc'
]

executable('socc', socc_src, include_directories: socc_inc)
