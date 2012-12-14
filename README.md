#LICENSE
Copyright (C) 2012 Tyler Pham, Raymond Padillo and Many Ayromlou of Ryerson University

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

This program also makes use of the Parrot AR.Drone library and the LICENSE can be found in those folders
	
#KARD: Kinect-A.R. Drone
The KARD Project is to allow the A.R. Drone to be controlled via the Kinect gestures.

#Getting the Environment Setup for Kinect & A.R. Drone

##Using Kinect and OpenNI/PrimeSense in Xcode
Use the script *_install_mac_kinect.sh*
* Command line options:
	* "--devel": use the Unstable libraries
	* "uninstall": remove all OpenNI/PrimeSense/Kinect/libusb packages
	
##Using A.R. Drone with Xcode
To use the A.R. Drone SDK 2.0 with Xcode do the following:

###Building the Libraries
_*Note:* We are using the Examples build as it compiles a complete set of static libraries we will be using_

_More detailed instructions can be found in: ARDrone/README.textile_

###Install OpenCV
OpenCV will be used for video and other image processing work. Although, there are private OpenCV.frameworks available we will be usin this instead. To install it, we are using brew:
	$> brew install opencv

###Clean and rebuild the libraries
	$> cd into ARDrone/
	$> cd Examples/OSX
	$> make USE_OSX=yes clean
	$> make USE_OSX=yes
	$> cd -

###Copy the libraries into _*'Libraries'*_ folder
	$> find . -name "lib*.a" -exec cp "{}" ../Libraries/ \;
	$> find . -type f \( -iname "libvlib.a" -or -iname "libpc_ardrone.a" -or -iname "libsdk.a" -or -iname "libpc_ardrone_notool.a" \) -exec cp "{}" ../Libraries/ \;

###Add the Search Paths to your Xcode Build Settings
Search for "Search Paths" and enter the following in each row
- Header Search Paths: $(SRCROOT)/ARDrone/ARDroneLib/**
- Library Search Paths: $(SRCROOT)/Libraries

###Add the Libraries/Frameworks to your Xcode Build Phases
Under *_Link Binary With Libraries_*:
- Click the *"+"*
- Select *"Add Other"*
- Select the following Frameworks:
	* GLKit.framework
- Select the following libraries under *"Libraries"*:
	* libpc_ardrone.a
	* libsdk.a
	* libvlib.a
- Select the following libraries under *"/usr/local/Cellar/lib"*:
	* libavcodec.a
	* libswscale.a
	* libavutil.a