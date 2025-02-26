# KlvDecoder
Converts KLV binary encoded metadata in a STANAG 4609 Motion Imagery stream to a human readable text format.

## How to Build
This project uses CMake to generate a build system, build and install the application, __KLVDecoder__. 

### 1.  Prerequisites
This project requires the following tools:

1. Install CMake, https://cmake.org/
2. Install vcpkg, https://github.com/microsoft/vcpkg, which is a packet management system.
3. Configure `VCPKG_ROOT` environment variable to the path where you installed __vcpkg__.

### 2.  External Dependencies
The __vcpkg__ package manager will install most of the external dependencies. However, the following are ones that 
you are required to build and install manually:

 - [mp2t library](https://github.com/jimcavoy/mp2tp) build and install the library.
 - [klvp library](https://github.com/jimcavoy/klvp) build and install the library.

 ### 3. Generate Build System
 On Windows, the following command will generate a Visual Studio 17 (VS 2022) solution and project files.
 ```
 cmake --preset=windows-base
 ```
 On Linux, the following command will generate UNIX Makefiles.
 ```
 cmake --preset=linux-base
 ```

 ### 4. Build KlvDecoder
```
cmake --build ./build --config <Debug|Release>
```

The `--config` option is either `Debug` for a debug build or `Release` for a release build. `Debug` is the default build configuration if the `--config` option is absent.

### 5. Install KlvDecoder

On Windows, you need Administrator role privileges before running the below command.
```
cmake --install ./build --config <Debug|Release>
```

On Linux

```
sudo cmake --install ./build --config <Debug|Release>
```

The `--config` option tells CMake either to install a debug or release build.

## To Test
To test __KlvDecoder__ build, enter the following:

```
ctest --test-dir ./build -C <Debug|Release>
```

The `-C` option specifies the build configuration to test, either `Debug` or `Release` build.

## Usage

```
KlvDecoder v1.0.0
Copyright (c) 2025 ThetaStream Consulting, jimcavoy@thetastream.com
Allowed options.:
  -? [ --help ]         Produce help message.
  --source arg          Source Motion Imagery stream or file. (default: - )
  -r [ --reads ] arg    Number of KLV reads. Zero means continuous reads.
                        (default: 0
  -f [ --freqs ] arg    Frequency (Hz) to output the text representation.
                        (default: 1)
  -F [ --format ] arg   Output text format [info|json|xml]. (default: json).
```

## Examples

### Read a File
In this example, output a JSON representation of the KLV encoded metadata from a
STANAG 4609 Motion Imagery file.

```
KlvDeoder C:\somefolder\sample_file.ts -F json
```

### Read a Stream
In this example, output an XML representation of the KLV encoded metadata from a 
STANAG 4609 stream being piped in from __IReflxApp__ application.

```
IReflxApp.exe -s 239.255.0.1:1841 | KlvDecoder.exe -F xml
```
