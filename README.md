# [SmartTiler] (https://github.com/Gaia3D/SmartTiler)

## Summary
This application, SmartTiler, is for tiling massive F4D contents which is devised for Mago3D - 3D web geo-platform. (www.mago3d.com). This project is of Multiplatform(UNIX, MAC OSX, Windows) C++ project.

## Getting Started
Start by cloning this repository, making sure to pass the `--recursive` flag to grab all the dependencies.
If you forgot, then you can `git submodule update --init` instead.

```bash
git clone --recursive https://github.com/Gaia3D/SmartTiler.git
cd SmartTiler
cd build
```

Now generate a project file or makefile for your platform.

```bash
# UNIX Makefile
cmake ..

# Mac OSX
cmake -G "Xcode" ..

# Microsoft Windows
cmake -G "Visual Studio 16 2019" ..
cmake -G "Visual Studio 16 2019 Win64" ..
...
```

## Dependencies

Functionality           | Version | Library
----------------------- | ------- | ------------------------------------------
Json Loading            | 0.10.7 | [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

## License
Copyright (c) 2012-2021, http://www.gaia3d.com
All rights reserved.

mago3D SmartTiler Commercial License for ISVs and VARs:
Gaia3D provides its mago3D SmartTiler under a dual license model designed
to meet the development and distribution needs of both commercial distributors
(such as ISVs and VARs) and open source projects.

For ISVs, VARs and Other Distributors of Commercial Applications:
ISVs (Independent Software Vendors), VARs (Value Added Resellers) and
other distributors that combine and distribute commercially licensed software with
mago3D SmartTiler software and do not wish to distribute the source code
or the commercially licensed software under version 3 of the GNU AFFERO GENERAL PUBLIC LICENSE
(the "AGPL" https://www.gnu.org/licenses/agpl-3.0.en.html ) must enter into
a commercial license agreement with Gaia3D.

For Open Source Projects and Other Developers of Open Source Applications:
For developers of Free Open Source Software ("FOSS") applications under the GPL or AGPL
that want to combine and distribute those FOSS applications with mago3D SmartTiler software,
Gaia3D's open source software licensed under the AGPL is the best option.