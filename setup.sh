#!/bin/sh

echo
echo "Pebble SDK Setup Script by Raptor007"
echo
echo "You may need to enter your sudo password, possibly several times."

if [ ! -f "path.sh" ]
then
	curl -o path.sh https://github.com/Raptor007/Pebble-Watchfaces/blob/master/path.sh
fi

if [ ! -x "path.sh" ]
then
	sudo chmod +x path.sh
fi

source "./path.sh"

if [ -z "${PEBBLE_SDK}" ]
then
	echo "Pebble SDK not found."
	return
fi

echo "Detected version: ${PEBBLE_SDK}"

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
	sudo port install boost +python27 +universal
else
	echo
	echo "MacPorts not installed; skipping dependency installation."
	echo "If setup fails now, please install MacPorts and try again:"
	echo "  https://macports.org/"
	echo
fi

CFLAGS="" pip install --user -r "${PEBBLE_SDK}/requirements.txt"

if [ -z "$(which virtualenv)" ]
then
	sudo pip install virtualenv
fi

if [ ! -z "$(echo ${PEBBLE_SDK} | grep PebbleSDK-3)" ]
then
	if [ ! -d "${PEBBLE_SDK}/.env" ]
	then
		virtualenv --no-site-packages "${PEBBLE_SDK}/.env"
	fi

	source "${PEBBLE_SDK}/.env/bin/activate"
	CFLAGS="" pip install -r "${PEBBLE_SDK}/requirements.txt"
	deactivate
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
