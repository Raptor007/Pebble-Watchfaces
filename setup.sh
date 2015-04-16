#!/bin/sh

echo
echo "Pebble SDK Setup Script by Raptor007"

if [ ! -f "path.sh" ]
then
	curl -o path.sh https://raw.githubusercontent.com/Raptor007/Pebble-Watchfaces/master/path.sh
fi

if [ ! -x "path.sh" ]
then
	chmod +x path.sh
fi

source "./path.sh"

if [ -z "${PEBBLE_SDK}" ]
then
	echo "ERROR: Pebble SDK not found."
	exit
fi

echo "Detected version: ${PEBBLE_SDK}"

echo
echo "You may need to enter your sudo password, possibly several times."

if [ ! -z "$(which port)" ]
then
	sudo port selfupdate
	sudo port install arm-none-eabi-gcc +universal
	sudo port install glib1 +universal
	sudo port install glib2 +universal
	sudo port install freetype +universal
	sudo port install libmpc +universal
	sudo port install libpixman +universal
	sudo port install python27 +universal
	sudo port install python_select +universal
	sudo port select --set python python27
	sudo port install py27-pip +universal
	sudo port select --set pip pip27
	sudo port install py27-pil +universal
	sudo port install cctools +universal

	# Prefer system libc++ if present, otherwise use MacPorts.
	if [ ! -f "/usr/lib/libc++.dylib" ]
	then
		if [ ! -x "/opt/local/bin/clang++" ]
		then
			sudo port install clang-3.4 +universal
			sudo port install clang_select +universal
			sudo port select --set clang mp-clang-3.4
		fi

		sudo port install libcxx +universal configure.compiler=macports-clang
	fi

	# Prefer system libc++abi if present, otherwise use MacPorts.
	if [ ! -f "/usr/lib/libc++abi.dylib" ]
	then
		if [ ! -x "/opt/local/bin/clang++" ]
		then
			sudo port install clang-3.4 +universal
			sudo port install clang_select +universal
			sudo port select --set clang mp-clang-3.4
		fi

		sudo port install libcxxabi +universal configure.compiler=macports-clang
	fi

	# Prefer Homebrew boost if present, otherwise use MacPorts.
	if [ ! -f "/usr/local/lib/libboost_python-mt.dylib" ]
	then
		if [ ! -x "/opt/local/bin/clang++" ]
		then
			sudo port install clang-3.4 +universal
			sudo port install clang_select +universal
			sudo port select --set clang mp-clang-3.4
		fi

		CFLAGS="-fPIC" CXXFLAGS="-fPIC" LDFLAGS="-fPIC" sudo port install boost +universal +python27 +clang34 -no_static configure.compiler=macports-clang
	fi
else
	echo
	echo "MacPorts not installed; skipping dependency installation."
	echo "If setup fails now, please install MacPorts and try again:"
	echo "  https://macports.org/"
	echo
fi

sudo pip install --upgrade pip
CFLAGS="" pip install --user --upgrade -r "${PEBBLE_SDK}/requirements.txt"
sudo pip install --upgrade virtualenv

if [ ! -z "$(echo ${PEBBLE_SDK} | grep PebbleSDK-3)" ]
then
	if [ ! -d "${PEBBLE_SDK}/.env" ]
	then
		virtualenv --no-site-packages "${PEBBLE_SDK}/.env"
	fi

	source "${PEBBLE_SDK}/.env/bin/activate"
	CFLAGS="" pip install --upgrade -r "${PEBBLE_SDK}/requirements.txt"
	deactivate

	echo "Fixing Pebble simulator..."

	if [ ! -f "${PEBBLE_SDK}/Pebble/common/qemu/qemu-system-arm_Darwin_i386" ]
	then
		cd "${PEBBLE_SDK}/Pebble/common/qemu" && sudo ln -s qemu-system-arm_Darwin_x86_64 qemu-system-arm_Darwin_i386
		cd "${HERE}"
	fi

	# If no Homebrew boost, we need PyV8 to look in /opt/local/lib instead.
	if [ ! -f "/usr/local/lib/libboost_python-mt.dylib" ]
	then
		# First, keep a backup if we haven't already done so.
		if [ ! -f "${HERE}/${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/_PyV8.so.bak" ]
		then
			cp "${HERE}/${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/_PyV8.so" "${HERE}/${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/_PyV8.so.bak"
		fi

		# Use the PYV8_FIX environment variable to determine if we'll relink the existing .so or rebuild it from source.
		if [ "${PYV8_FIX}" != "rebuild" ]
		then
			# Use cctools to relink PyV8 from /usr/local/bin to /opt/local/bin.

			NAMETOOL="install_name_tool"
			if [ -x "/opt/local/bin/install_name_tool" ]
			then
				NAMETOOL="/opt/local/bin/install_name_tool"
			fi

			BOOSTLIBS="$(ls -1 /opt/local/lib/libboost*)"
			for BOOSTLIB in ${BOOSTLIBS}
			do
				BREW_BOOSTLIB="/usr/local/lib/$(basename ${BOOSTLIB})"
				if [ -f "${BREW_BOOSTLIB}" ]
				then
					"${NAMETOOL}" -change "${BOOSTLIB}" "${BREW_BOOSTLIB}" "${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/_PyV8.so"
				else
					"${NAMETOOL}" -change "${BREW_BOOSTLIB}" "${BOOSTLIB}" "${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/_PyV8.so"
				fi
			done
		else
			# Rebuild PyV8 from source.

			svn co 'https://github.com/pebble/pyv8/trunk' /tmp/pyv8
			cd /tmp/pyv8
			sed "s#extra_compile_args += \[\"-Wdeprecated-writable-strings\", \"-stdlib=libc++\"\]##" /tmp/pyv8/setup.py | sed "s#.replace\('-Werror', ''\)#.replace('-Werror', '').replace('-Wlinefeed-eof', '')#" | sed "s#'GCC_WARN_ABOUT_MISSING_NEWLINE': 'YES'#'GCC_WARN_ABOUT_MISSING_NEWLINE': 'NO'#" > /tmp/pyv8/setup_gcc.py
			chmod +x /tmp/pyv8/*.py

			PYV8_CC="clang++"
			if [ -x "/opt/local/bin/clang++" ]
			then
				PYV8_CC="/opt/local/bin/clang++"
			fi

			if [ -z "${PYTHON_HOME}" -a -d "/opt/local/Library/Frameworks/Python.framework/Versions/2.7" ]
			then
				PYTHON_HOME="/opt/local/Library/Frameworks/Python.framework/Versions/2.7"
			fi

			PYV8_FLAGS="-I/opt/local/include -std=c++11 -stdlib=libc++ -lc++ -lc++abi -lboost_python-mt -mmacosx-version-min=10.6 -fPIC"
			PYTHON_HOME="${PYTHON_HOME}" BOOST_HOME=/opt/local CC="${PYV8_CC}" CXX="${PYV8_CC}" LINK="${PYV8_CC}" CFLAGS="${PYV8_FLAGS}" CCFLAGS="${PYV8_FLAGS}" CXXFLAGS="${PYV8_FLAGS}" LDFLAGS="${PYV8_FLAGS}" ./setup.py build

			# If the build was successful, install our build of _PyV8.so into the Pebble SDK.
			PYV8_SO="$(find /tmp/pyv8/build -name _PyV8.so)"
			if [ ! -z "${PYV8_SO}" ]
			then
				cp "${PYV8_SO}" "${HERE}/${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/"
			fi

			cd "${HERE}"
		fi
	else
		# We do have Homebrew boost installed, so restore our .so.bak if we've messed with the .so before.
		if [ -f "${HERE}/${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/_PyV8.so.bak" ]
		then
			mv "${HERE}/${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/_PyV8.so.bak" "${HERE}/${PEBBLE_SDK}/Pebble/common/phonesim/PyV8/darwin64/_PyV8.so"
		fi
	fi
fi

if [ ! -d "${PEBBLE_SDK}/Examples/watchfaces/drop_zone" ]
then
	echo "Downloading example watchface drop_zone..."
	mkdir -p "${PEBBLE_SDK}/Examples/watchfaces"
	svn co 'https://github.com/pebble-examples/drop-zone/trunk' "${PEBBLE_SDK}/Examples/watchfaces/drop_zone"
fi

GCC_FILES="$(find ${PEBBLE_SDK} -name pebble_sdk_gcc.py)"
if [ -z "${GCC_FILES}" ]
then
	echo "Generating waf build scripts..."
	cd "${PEBBLE_SDK}/Examples/watchfaces/drop_zone" && pebble build 2>/dev/null
	cd "${HERE}"

	GCC_FILES="$(find ${PEBBLE_SDK} -name pebble_sdk_gcc.py)"
fi

if [ ! -z "${GCC_FILES}" ]
then
	for GCC_FILE in ${GCC_FILES}
	do
		if [ -z "$(grep nostartfiles ${GCC_FILE})" ]
		then
			echo "Patching: ${GCC_FILE}"

			sed "s/optimize_flag]/optimize_flag,'-nostartfiles']/" "${GCC_FILE}" > "${GCC_FILE}.new"
			if [ -s "${GCC_FILE}.new" ]
			then
				mv "${GCC_FILE}" "${GCC_FILE}.bak"
				mv "${GCC_FILE}.new" "${GCC_FILE}"
			else
				echo "WARNING: Failed to patch: ${GCC_FILE}"
			fi
		fi
	done
else
	echo "WARNING: Couldn't find pebble_sdk_gcc.py to patch."
fi

echo "Attempting to build Drop Zone example watchface..."
cd "${PEBBLE_SDK}/Examples/watchfaces/drop_zone" && pebble clean && pebble build
cd "${HERE}"

echo
echo "Setup complete for: ${PEBBLE_SDK}"
echo
echo "To configure paths for this session, type:"
echo "  source path.sh"
echo
