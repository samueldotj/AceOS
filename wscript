#!/usr/bin/env python

import os, Task, Options, Configure, Utils, misc

Configure.autoconfig = 1

VERSION				= '3.0.' + os.popen("svn info | grep Revision | cut -f2 -d:").read().strip()
APPNAME				= 'Ace'
srcdir 				= '.'
blddir				= 'build'

arch				= 'i386'
defines				= '-D _KERNEL_'

debug_flags			= '-gdwarf-2 -g3'

nasm_flags 			= defines + ' -w+orphan-labels -f elf'

kernel_c_flags		= defines + ' -Wall -Wno-multichar -ffreestanding -funsigned-char -fno-leading-underscore -c -fno-stack-protector'
acpi_c_flags		= kernel_c_flags + ' -Wno-format '
driver_c_flags		= kernel_c_flags
app_c_flags			= '-Wall ' 

kernel_ld_flags 	= '--gc-sections -Wl,-Map,kernel.map -T ../src/kernel/kernel.ld -nostdlib -nostartfiles '
driver_ld_flags 	= '-r'
app_ld_flags		= '-Wall ' 


def configure(conf):
	conf.check_tool('gcc nasm misc')
	conf.check_cc(msg="checking for flag='-fno-stack-protector'", ccflags='-fno-stack-protector', mandatory=1)
	
	if Options.options.arch:
		arch = Options.options.arch
	
	conf.env.append_unique('CCFLAGS_KERNEL', kernel_c_flags )
	conf.env.append_unique('CCFLAGS_ACPI', acpi_c_flags )
	conf.env.append_unique('CCFLAGS_DRIVER', driver_c_flags )
	conf.env.append_unique('CCFLAGS_APPLICATION', app_c_flags) ;
	
	conf.env.append_unique('NASM_FLAGS', nasm_flags)
	
	conf.env.append_unique('LINKFLAGS_KERNEL', kernel_ld_flags )
	conf.env.append_unique('LINKFLAGS_DRIVER', driver_ld_flags )
	conf.env.append_unique('LINKFLAGS_APPLICATION', app_ld_flags )
	
	if Options.options.build_type == 'debug':
		conf.env.append_unique('CCFLAGS', debug_flags)  
			
def build(bld):
	bld.add_subdirs('src/lib')
	bld.add_subdirs('src/kernel')
	bld.add_subdirs('src/drivers')
	bld.add_subdirs('src/app')
	bld.add_group()
	bld.new_task_gen( name='bootcd', source='src/kernel/kernel.sys', target='../bootcd.iso', rule='../scripts/create_bootcd.sh ${TGT[0].abspath(env)}' )	

def set_options(opt):
	opt.add_option("--arch", action="store", default="i386", help="Hardware architecture to build for.(currently only i386)", dest="arch")
	opt.add_option("--build", action="store", default="debug", help="Build type - release, debug", dest="build_type")
	opt.add_option("--usr-include", action="store", default="/usr/include", help="User include path", dest="usr_include")
	opt.add_option("--usr-lib", action="store", default="/usr/lib", help="User library search path", dest="usr_lib")
