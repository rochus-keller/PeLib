This is a modified version of https://github.com/LADSoft/DotNetPELib, commit [b1fd257 on 17 Nov 2020](https://github.com/LADSoft/DotNetPELib/commit/b1fd257702aff9d7c76e41095ac708a621120686).

See ORIG_README.md and ChangeLog.txt for more information.

This is a fully refactored version of the original library for use with the Oberon+ toolchain (see https://github.com/rochus-keller/oberon). 

The Allocator class is no longer present. Instances are still automatically deleted when PELib is deleted or goes out of scope. 

The IL and DLL reader code is removed, as well as all ObjIn and ObjOut methods. PELib::MSCorLibAssembly now creates a minimal assembly which is then incrementaly completed using the member refs pointing to mscorlib.

Added headers to decouple concerns. Added Stream class to remove PELib circular dependencies.

Many fixes.

The code is known to work on Linux and Windows. The generated assemblies work with both .NET Core and Mono.

This is work in progress.

CIL generics are not supported.
