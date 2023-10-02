#This is free and unencumbered software released into the public domain.
#Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.
#In jurisdictions that recognize copyright laws, the author or authors of this software dedicate any and all copyright interest in the software to the public domain. We make this dedication for the benefit of the public at large and to the detriment of our heirs and successors. We intend this dedication to be an overt act of relinquishment in perpetuity of all present and future rights to this software under copyright law.
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#For more information, please refer to <https://unlicense.org/>

include(CheckCXXCompilerFlag)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.18")
	include(CheckLinkerFlag)
endif()

function(determineSupportedHardeningCompilerFlags property)
	set(FLAGS_HARDENING "")
	foreach(flag ${ARGN})
		unset(var_name)
		string(REPLACE "=" "_eq_" var_name ${flag})
		string(REPLACE "," "_comma_" var_name ${var_name})
		set(var_name "SUPPORTS_HARDENING_${property}_${var_name}")
		check_cxx_compiler_flag(${flag} ${var_name}) #since linker flags and other flags are in the form of compiler flags. THIS DOESN'T REALLY WORK
		if(${${var_name}})
			list(APPEND FLAGS_HARDENING "${flag}")
		endif()
	endforeach(flag)
	string(REPLACE ";" " " FLAGS_HARDENING "${FLAGS_HARDENING}")
	#message(STATUS "FLAGS_HARDENING ${FLAGS_HARDENING}")
	set(HARDENING_${property} "${FLAGS_HARDENING}" PARENT_SCOPE)
endfunction(determineSupportedHardeningCompilerFlags)

function(determineSupportedHardeningLinkerFlags property)
	set(FLAGS_HARDENING "")
	foreach(flag ${ARGN})
		unset(var_name)
		string(REPLACE "=" "_eq_" var_name ${flag})
		string(REPLACE "," "_comma_" var_name ${var_name})
		set(var_name "SUPPORTS_HARDENING_${property}_${var_name}")
		check_linker_flag(CXX ${flag} ${var_name})
		if(${${var_name}})
			list(APPEND FLAGS_HARDENING "${flag}")
		endif()
	endforeach(flag)
	string(REPLACE ";" " " FLAGS_HARDENING "${FLAGS_HARDENING}")
	#message(STATUS "FLAGS_HARDENING ${FLAGS_HARDENING}")
	set(HARDENING_${property} "${FLAGS_HARDENING}" PARENT_SCOPE)
endfunction(determineSupportedHardeningLinkerFlags)

function(processCompilerFlagsList target property cache)
	get_target_property(FLAGS_UNHARDENED ${target} ${property})
	if(FLAGS_UNHARDENED MATCHES "FLAGS_UNHARDENED-NOTFOUND")
		set(FLAGS_UNHARDENED "")
	endif()
	#message(STATUS "processCompilerFlagsList ${target} ${property} ${FLAGS_UNHARDENED}")
	#message(STATUS "HARDENING_${property} ${HARDENING_${property}}")
	
	if(cache)
		if(HARDENING_${property})
		else()
			determineSupportedHardeningCompilerFlags(${property} ${ARGN})
			set(HARDENING_${property} "${HARDENING_${property}}" CACHE STRING "Hardening flags")
		endif()
	else()
		determineSupportedHardeningCompilerFlags(${property} ${ARGN})
	endif()
	
	set(FLAGS_HARDENED ${FLAGS_UNHARDENED})
	list(APPEND FLAGS_HARDENED ${HARDENING_${property}})
	string(REPLACE ";" " " FLAGS_HARDENED "${FLAGS_HARDENED}")
	#message(STATUS "${target} PROPERTIES ${property} ${FLAGS_HARDENED}")
	set_target_properties(${target} PROPERTIES ${property} "${FLAGS_HARDENED}")
endfunction(processCompilerFlagsList)

function(processLinkerFlagsList target property cache)
	get_target_property(FLAGS_UNHARDENED ${target} ${property})
	if(FLAGS_UNHARDENED MATCHES "FLAGS_UNHARDENED-NOTFOUND")
		set(FLAGS_UNHARDENED "")
	endif()
	#message(STATUS "processCompilerFlagsList ${target} ${property} ${FLAGS_UNHARDENED}")
	#message(STATUS "HARDENING_${property} ${HARDENING_${property}}")
	
	if(cache)
		if(HARDENING_${property})
		else()
			determineSupportedHardeningLinkerFlags(${property} ${ARGN})
			set(HARDENING_${property} "${HARDENING_${property}}" CACHE STRING "Hardening flags")
		endif()
	else()
		determineSupportedHardeningLinkerFlags(${property} ${ARGN})
	endif()
	
	set(FLAGS_HARDENED ${FLAGS_UNHARDENED})
	list(APPEND FLAGS_HARDENED ${HARDENING_${property}})
	string(REPLACE ";" " " FLAGS_HARDENED "${FLAGS_HARDENED}")
	#message(STATUS "${target} PROPERTIES ${property} ${FLAGS_HARDENED}")
	set_target_properties(${target} PROPERTIES ${property} "${FLAGS_HARDENED}")
endfunction(processLinkerFlagsList)

function(setupPIC target)
	set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON) # bad, doesn't work
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		get_target_property(type ${target} TYPE)
		if(type STREQUAL "EXECUTABLE")
			list(APPEND HARDENING_PIC_COMPILE_FLAGS
				"-fPIE"
			)
		else()
			list(APPEND HARDENING_PIC_COMPILE_FLAGS
				"-fPIC"
			)
		endif()
		if(type STREQUAL "EXECUTABLE")
			# https://mropert.github.io/2018/02/02/pic_pie_sanitizers/
			list(APPEND HARDENING_PIC_LINKER_FLAGS
				"-pie;-Wl,-pie"  # Thanks to Jannik2099 for pointing out how to do it correctly
			)
		endif()
	elseif(MSVC)
		list(APPEND HARDENING_PIC_COMPILE_FLAGS
			"/dynamicbase" "/HIGHENTROPYVA"
		)
	else()
		message(ERROR "The compiler is not supported")
	endif()
	processCompilerFlagsList(${target} COMPILE_FLAGS OFF ${HARDENING_PIC_COMPILE_FLAGS})
	processCompilerFlagsList(${target} LINK_FLAGS OFF ${HARDENING_PIC_LINKER_FLAGS})
endfunction(setupPIC)

function(harden target)
	setupPIC("${target}")
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		list(APPEND HARDENING_COMPILER_FLAGS
			"-Wall" "-Wextra" "-Wconversion" "-Wformat" "-Wformat-security" "-Werror=format-security"
			"-fno-strict-aliasing" "-fno-common"
			#"-fstack-check"
			#"-fcf-protection=full" # conflicts to "-mindirect-branch"
			#"-fcf-protection=branch"
			"-fcf-runtime-abi=full"

			"-ffp-exception-behavior=strict"
			"-mcet"
			"-fsanitize=cfi"
			"-fsanitize=cfi-cast-strict"
			"-fsanitize=cfi-derived-cast"
			"-fsanitize=cfi-unrelated-cast"
			"-fsanitize=cfi-nvcall"
			"-fsanitize=cfi-vcall"
			"-fsanitize=cfi-icall"
			"-fsanitize=cfi-mfcall"
			#"-fsanitize=signed-integer-overflow" # ubsan
			#"-fsanitize=unsigned-integer-overflow" # ubsan
			
			"-mbranch-protection=standard"
			"-mbranch-protection=pac-ret+leaf"
			"-mbranch-protection=bti"
			
			"-fzero-call-used-regs=all"
			"-mharden-sls=all"
			"-mfunction-return=thunk-extern"
			"-ftrivial-auto-var-init=pattern"

			# CLang-ish flags
			"-mretpoline"
			"-lvi-load-hardening"
			"-lvi-cfi"
			"-ehcontguard"
			
			# TODO implement compiler flag dependence on libs linking
			
			#"-fvtable-verify=std;vtv"
			#"-fvtable-verify=[std|preinit|none]"
			
			# this conflicts with gcc which now has -fcf-protection=full hardcoded
			#"-fcf-protection=branch"
			"-fcf-protection=none -mindirect-branch"
			"-fcf-protection=none -mindirect-branch=thunk-extern"
			"-fcf-protection=none -mindirect-branch=thunk-inline"
			"-fcf-protection=none -mindirect-return"
			"-fcf-protection=none -mindirect-branch-register"
			"-fcf-protection=none -mindirect-branch-loop"
			
			"-x86-speculative-load-hardening"
			"-mno-indirect-branch-register"
		)

		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			# Implemented only for 64-bit
			list(APPEND HARDENING_COMPILER_FLAGS
				"-mspeculative-load-hardening"
			)
			set(HARDENING_SSE2_DEFAULT_VALUE ON)
		else()
			set(HARDENING_SSE2_DEFAULT_VALUE OFF)
		endif()

		option(HARDENING_SSE2 "Enable hardening flags requiring at least SSE2 support for target" "${HARDENING_SSE2_DEFAULT_VALUE}")
		if(HARDENING_SSE2)
			list(APPEND HARDENING_COMPILER_FLAGS
				"-mlfence-after-load=yes"
				"-mlfence-before-indirect-branch=all"
				"-mlfence-before-ret=not"
			)
		endif(HARDENING_SSE2)
		
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
			# https://clang.llvm.org/docs/ShadowCallStack.html
			list(APPEND HARDENING_COMPILER_FLAGS
				"-fsanitize=shadow-call-stack"
			)
		endif()

		# some flags are bugged in GCC
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		else()
			list(APPEND HARDENING_COMPILER_FLAGS
				"-ftrapv" # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=35412
			)
		endif()
		
		# GCC 9 has removed these flags
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10))
			message(STATUS "GCC 9 removes some hardening flags but doesn't fail if they are present, instead shows deprecation message. In order to not to put garbage into warnings we don't insert them. See the code of Hardening.cmake for the details.")
		else()
			list(APPEND HARDENING_COMPILER_FLAGS
				"-mmitigate-rop" 
				"-fcheck-pointer-bounds"
				"-fchkp-treat-zero-size-reloc-as-infinite"
				"-fchkp-first-field-has-own-bounds"
				"-fchkp-narrow-bounds"
				"-fchkp-narrow-to-innermost-array"
				"-fchkp-optimize"
				"-fchkp-use-fast-string-functions"
				"-fchkp-use-nochk-string-functions"
				"-fchkp-use-static-const-bounds"
			)
		endif()
		
		list(APPEND HARDENING_LINKER_FLAGS
			"-Wl,-O1"
			"-Wl,-flto"
			"-fstack-clash-protection"
		)
		if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
			list(APPEND HARDENING_LINKER_FLAGS
				"-Wl,--sort-common"
				"-Wl,--as-needed"
			)
		endif()

		if(CMAKE_SYSTEM_NAME MATCHES "Windows")
			list(APPEND HARDENING_LINKER_FLAGS
				"-Wl,--export-all-symbols"
				"-Wl,--nxcompat"
				"-Wl,--dynamicbase"
			)
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			#	list(APPEND HARDENING_LINKER_FLAGS "-Wl,--image-base,0x140000000") # doesn't work for this project
			endif()
		elseif(CMAKE_SYSTEM_NAME MATCHES "Linux") # other using ELF too?
			list(APPEND HARDENING_COMPILER_FLAGS
				# on MinGW hello world works, but more complex things just exit without any output or crash in the middle of execution
				"-fstack-protector"
				"-fstack-protector-strong"
			)
			list(APPEND HARDENING_LINKER_FLAGS
				# not present in MinGW
				"-Wl,-z,relro"
				"-Wl,-z,now"
				"-Wl,-z,ibtplt"
				"-Wl,-z,ibt"
				"-Wl,-z,shstk"
				"-Wl,-z,notext"  # may be required for PIC
				"-Wl,-z-noexecstack"
				"-Wl,-z,noexecheap"
			)
		endif()
		list(APPEND HARDENING_MACRODEFS
			"-D_FORTIFY_SOURCE=2"  # 3 requires recent Ubuntu, which is not available in GitHub Actions, which MinGW-w64 has __chk_fail __memcpy_chk __strcat_chk
			"-D_GLIBCXX_ASSERTIONS"
		)
	elseif(MSVC)
		set(HARDENING_COMPILER_FLAGS "/sdl" "/GS" "/SafeSEH" "/guard:cf" "/HIGHENTROPYVA")
		set(HARDENING_LINKER_FLAGS "/guard:cf")
	else()
		message(ERROR "The compiler is not supported")
	endif()

	processCompilerFlagsList(${target} COMPILE_FLAGS ON ${HARDENING_COMPILER_FLAGS})
	if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.18")
		processLinkerFlagsList(${target} LINK_FLAGS ON ${HARDENING_LINKER_FLAGS})
	else()
		processCompilerFlagsList(${target} LINK_FLAGS ON ${HARDENING_LINKER_FLAGS})
	endif()
	
	#list(JOIN HARDENING_MACRODEFS " " HARDENING_MACRODEFS) # unneeded, list is needed, not string
	set(HARDENING_MACRODEFS "${HARDENING_MACRODEFS}" CACHE STRING "Hardening flags CMake list (not string!)")
	target_compile_definitions(${target} PRIVATE ${HARDENING_MACRODEFS})
endfunction(harden)
