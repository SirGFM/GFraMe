if [ "${OS}" != "emscript" ] && [ "${OS}" != "win" ] && [ "${OS}" != "linux" ]; then
	echo 'defaulting OS to "linux"...'
	export OS=linux
fi

if [ -z "${ARCH}" ]; then
	ARCH=$(uname -m)
	echo "defaulting ARCH to \"${ARCH}\"..."
fi

if [ "${ARCH}" != "i686" ] && [ "${ARCH}" != "x86_64" ] ; then
	echo 'envvar ARCH must be set to either i686 or x86_64!'
	return
fi

if [ "${OS}" == "emscript" ]; then
	export CC=emcc
	export CFLAGS=-I/gframe-dev/linux/i686/include
	export LDFLAGS=-L/gframe-dev/emscript
else
	if [ "${OS}" == "win" ]; then
		export TOOL_PREFIX=${ARCH}-w64-mingw32-
	fi
	export LDFLAGS=-L/gframe-dev/${OS}/${ARCH}/lib/c_synth\ -L/gframe-dev/${OS}/${ARCH}/lib/GFraMe
	export CFLAGS=-I/gframe-dev/${OS}/${ARCH}/include/
	export AR=${TOOL_PREFIX}ar
	export STRIP=${TOOL_PREFIX}strip
	export CC=${TOOL_PREFIX}gcc
	export WINDRES=${TOOL_PREFIX}windres
fi
