This is a modified version of https://github.com/LADSoft/DotNetPELib, commit [b1fd257 on 17 Nov 2020](https://github.com/LADSoft/DotNetPELib/commit/b1fd257702aff9d7c76e41095ac708a621120686).

See ORIG_README.md and ChangeLog.txt for more information.

The API has changed a bit compared to the original library. The Allocator class is no longer present. Instances are still automatically deleted when PELib is deleted or goes out of scope.

The code is known to work on Linux and Windows. The generated assemblies work with both .NET Core and Mono.

This is work in progress.
