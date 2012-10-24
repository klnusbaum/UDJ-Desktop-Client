#UDJ

UDJ is a social music player. It allows many people to control
a single music player democratically. Checkout the
[UDJ homepage][home] for more information. This is the official
UDJ Desktop Player. For more details on actually interacting with
UDJ (so you can do something like creating your own client), see the [UDJ Server Repository][server].


## Building The Desktop Client

### Requirements

The UDJ Desktop Client requires a couple external libraries and an external build tool called
CMake. That said, UDJ is cross-platform and can be built on Windows, Mac OSX, and most 
distributions of Linux.

1. CMake is the build system used by Desktop Client. Precompiled binaries for both OSX and
Windows can be found on the [CMake Website][cmake]. Most linux distributions have CMake in their
package repositories. It can also be built from source which is also located on the CMake website.

2. Qt is the cross-platform GUI framework used by the UDJ Desktop Client. The SDK and libraries
for all platforms can be downloaded from [Qt's website][qt]. Most linux distributions also
have Qt in their package repository. Note that the Qt phonon library is also required.

3. Taglib is used by the UDJ Desktop Client for identifying song information. The source
can be downloaded from the [taglib website][taglib]. On OSX, taglib can easily be installed
via [homebrew][brew]. On Linux, most distributions have the taglib library in their 
repository. On Windows, shit is tough. I'll try to add some instructions on that later.

4. (Windows Only) OpenSSL is not bundled with Windows for severally reasons (all of which piss off the lead developer).
A binary distribution for windows can be found [here][win-openssl].

### Configuring
If you've installed all of your libraries and cmake in default locations, configuring should
be very straight forward. Simply use cmake to configure the project (we recommend an out of 
source build). You can turn on debug messages by setting the `UDJ_DEBUG_BUILD` variable to `ON`.

#### Note for CMake 2.8.8
There is a regression in CMake 2.8.8 that gives the DeployQt4.cmake some issues. Applying this [patch][deploypatch]
to it should fix the issue. Alternatively you can simply change the line in DeployQt4.cmake that says

    function(resolve_qt4_paths paths_var)
      set(executable_path ${ARGV1})

to

    function(resolve_qt4_paths paths_var)
      if(ARGC GREATER 1)
         set(executable_path ${ARGV1})
      endif()

#### Note for building on Windows with CMake 2.8.8 and below
There is a deficiency in the FindQt4.cmake module for CMake 2.8.8 and below
that does not allow it to find the phonon_ds9 backend on windows. This can
be fixed by applying [this patch][findphononpatch] to the FindQt4.cmake file.
Alternatively, you can simply change your FindQt4.cmake file yourself like so. Find
the line that says:

    SET( QT_PHONON_BACKEND_PLUGINS phonon_qt7 )

and change it to:


    IF(APPLE)
      SET( QT_PHONON_BACKEND_PLUGINS phonon_qt7 )
    ELSEIF(WIN32)
      SET( QT_PHONON_BACKEND_PLUGINS phonon_ds9 )
    ENDIF()

### Building on Ubuntu 12.04 LTS

1. Enter the command "sudo apt-get update" into Terminal to update the repository on your machine.
2. Enter the command "sudo apt-get install" with the following dependencies to build UDJ:
	- cmake
	- libqt4-dev
	- libtag1-dev
	- liphonon-dev
	- phonon-backend-vlc
	- build essentials
3. In the /build folder, enter "make" into the Terminal to create a MakeFile
4. Change into the /src folder and enter "./udj" to run the UDJ Desktop Application

### Building
CMake will generate different projects base on your host system. On OSX and Linux the default is 
a makefile based project. Hence a simple issue of the `make` command will build the project 
(unless you've configured CMake to generate some other type of project). 
On Windows, CMake generates a Visual Studio solution file that can then be used to build UDJ.

## Who Are You?

UDJ is a team effort lead by [Kurtis Nusbaum][kln].
I really like computers and programming.

## License
UDJ is licensed under the [GPLv2][gpl].

## Questions/Comments?

If you have any questions or comments, feel free to post them to
the [UDJ mailing list][mailing].

[home]:https://www.udjplayer.com
[server]:https://github.com/klnusbaum/UDJ-Server
[kln]:https://github.com/klnusbaum/
[gpl]:https://github.com/klnusbaum/UDJ-Desktop-Client/blob/master/LICENSE
[cmake]:http://www.cmake.org/cmake/resources/software.html
[qt]:http://qt.nokia.com/downloads
[taglib]:http://developer.kde.org/~wheeler/taglib.html
[brew]:http://mxcl.github.com/homebrew/
[mailing]:mailto:udjdev@bazaarsolutions.com
[deploypatch]:https://github.com/downloads/klnusbaum/UDJ-Desktop-Client/0001-DeployQt4-Set-executable_path-if-actually-passed.patch
[findphononpatch]:https://github.com/downloads/klnusbaum/UDJ-Desktop-Client/0001-phonon-backend-tweak.patch
[win-openssl]:http://www.openssl.org/related/binaries.html
