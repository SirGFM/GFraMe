from ubuntu:20.04

SHELL ["/bin/bash", "-c"]

ARG SDL_VERSION=2.28.3
RUN dpkg --add-architecture i386 && \
	apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get install -y \
		git \
		build-essential \
		gcc-multilib \
		g++-multilib \
		mingw-w64 \
		python3 \
		libsdl2-dev \
		libsdl2-dev:i386 \
		curl \
		unzip && \
	curl -o /tmp/sdl2-win64.zip --location \
		https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/SDL2-${SDL_VERSION}-win32-x64.zip && \
	mkdir -p /gframe-dev/win/x86_64/include && \
	mkdir -p /gframe-dev/win/x86_64/lib/sdl2 && \
	unzip -d /gframe-dev/win/x86_64/lib/sdl2 /tmp/sdl2-win64.zip && \
	curl -o /tmp/sdl2-win32.zip --location \
		https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/SDL2-${SDL_VERSION}-win32-x86.zip && \
	mkdir -p /gframe-dev/win/i686/include && \
	mkdir -p /gframe-dev/win/i686/lib/sdl2 && \
	unzip -d /gframe-dev/win/i686/lib/sdl2 /tmp/sdl2-win32.zip && \
	curl -o /tmp/sdl2-mingw.zip --location \
		https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/SDL2-devel-${SDL_VERSION}-mingw.zip && \
	unzip -d /tmp/ /tmp/sdl2-mingw.zip && \
	mv /tmp/SDL2-${SDL_VERSION}/x86_64-w64-mingw32/lib/* /gframe-dev/win/x86_64/lib/sdl2/ && \
	mv /tmp/SDL2-${SDL_VERSION}/x86_64-w64-mingw32/include/SDL2 /gframe-dev/win/x86_64/include/ && \
	mv /tmp/SDL2-${SDL_VERSION}/i686-w64-mingw32/lib/* /gframe-dev/win/i686/lib/sdl2/ && \
	mv /tmp/SDL2-${SDL_VERSION}/i686-w64-mingw32/include/SDL2 /gframe-dev/win/i686/include/ && \
	rm -f /tmp/sdl2-win64.zip/tmp/sdl2-win32.zip/tmp/sdl2-mingw.zip

ARG EMSDK_VERSION=3.1.46
RUN git clone --depth 1 --branch ${EMSDK_VERSION} https://github.com/emscripten-core/emsdk.git && \
	/emsdk/emsdk install ${EMSDK_VERSION} && \
	/emsdk/emsdk activate ${EMSDK_VERSION} && \
	echo 'source "/emsdk/emsdk_env.sh"' >> /etc/bash.bashrc

ARG CSYNTH_VERSION=devel-v2.0.0-rc
RUN source "/emsdk/emsdk_env.sh" && \
	git clone --depth 1 --branch ${CSYNTH_VERSION} https://github.com/SirGFM/c_synth.git && \
	cd c_synth && \
	make install -j8 \
		OS=Linux ARCH=i686 DEBUG=yes \
		PREFIX=/gframe-dev/linux/i686 && \
	make clean OS=Linux DEBUG=yes && \
	make install -j8 \
		OS=Linux ARCH=i686 RELEASE=yes \
		PREFIX=/gframe-dev/linux/i686 && \
	make clean OS=Linux RELEASE=yes && \
	make install -j8 \
		OS=Linux ARCH=x86_64 DEBUG=yes \
		PREFIX=/gframe-dev/linux/x86_64 && \
	make clean OS=Linux DEBUG=yes && \
	make install -j8 \
		OS=Linux ARCH=x86_64 RELEASE=yes \
		PREFIX=/gframe-dev/linux/x86_64 && \
	make clean OS=Linux RELEASE=yes && \
	make install -j8 \
		OS=Win ARCH=i686 DEBUG=yes \
		AR=i686-w64-mingw32-ar \
		STRIP=i686-w64-mingw32-strip \
		CC=i686-w64-mingw32-gcc \
		PREFIX=/gframe-dev/win/i686 && \
	make clean OS=Win DEBUG=yes && \
	make install -j8 \
		OS=Win ARCH=i686 RELEASE=yes \
		AR=i686-w64-mingw32-ar \
		STRIP=i686-w64-mingw32-strip \
		CC=i686-w64-mingw32-gcc \
		PREFIX=/gframe-dev/win/i686 && \
	make clean OS=Win RELEASE=yes && \
	make install -j8 \
		OS=Win ARCH=x86_64 DEBUG=yes \
		AR=x86_64-w64-mingw32-ar \
		STRIP=x86_64-w64-mingw32-strip \
		CC=x86_64-w64-mingw32-gcc \
		PREFIX=/gframe-dev/win/x86_64 && \
	make clean OS=Win DEBUG=yes && \
	make install -j8 \
		OS=Win ARCH=x86_64 RELEASE=yes \
		AR=x86_64-w64-mingw32-ar \
		STRIP=x86_64-w64-mingw32-strip \
		CC=x86_64-w64-mingw32-gcc \
		PREFIX=/gframe-dev/win/x86_64 && \
	make clean OS=Win RELEASE=yes && \
	make emscript \
		OS=emscript RELEASE=yes \
		CC=emcc && \
	mkdir -p /gframe-dev/emscript/ && \
	cp bin/release/emscript/* /gframe-dev/emscript/

ARG GFRAME_VERSION=devel-v3.1.0-rc
RUN source "/emsdk/emsdk_env.sh" && \
	git clone --depth 1 --branch ${GFRAME_VERSION} https://github.com/SirGFM/GFraMe.git && \
	cd GFraMe && \
	make install -j8 \
		OS=Linux ARCH=i686 DEBUG=yes \
		LDFLAGS=-L/gframe-dev/linux/i686/lib/c_synth \
		CFLAGS=-I/gframe-dev/linux/i686/include \
		PREFIX=/gframe-dev/linux/i686 && \
	make clean OS=Linux DEBUG=yes && \
	make install -j8 \
		OS=Linux ARCH=i686 RELEASE=yes \
		LDFLAGS=-L/gframe-dev/linux/i686/lib/c_synth \
		CFLAGS=-I/gframe-dev/linux/i686/include \
		PREFIX=/gframe-dev/linux/i686 && \
	make clean OS=Linux RELEASE=yes && \
	make install -j8 \
		OS=Linux ARCH=x86_64 DEBUG=yes \
		LDFLAGS=-L/gframe-dev/linux/x86_64/lib/c_synth \
		CFLAGS=-I/gframe-dev/linux/x86_64/include \
		PREFIX=/gframe-dev/linux/x86_64 && \
	make clean OS=Linux DEBUG=yes && \
	make install -j8 \
		OS=Linux ARCH=x86_64 RELEASE=yes \
		LDFLAGS=-L/gframe-dev/linux/x86_64/lib/c_synth \
		CFLAGS=-I/gframe-dev/linux/x86_64/include \
		PREFIX=/gframe-dev/linux/x86_64 && \
	make clean OS=Linux RELEASE=yes && \
	make install -j8 \
		OS=Win ARCH=i686 DEBUG=yes \
		AR=i686-w64-mingw32-ar \
		STRIP=i686-w64-mingw32-strip \
		CC=i686-w64-mingw32-gcc \
		LDFLAGS=-L/gframe-dev/win/i686/lib/c_synth\ -L/gframe-dev/win/i686/lib/sdl2 \
		CFLAGS=-I/gframe-dev/win/i686/include \
		PREFIX=/gframe-dev/win/i686 && \
	make clean OS=Win DEBUG=yes && \
	make install -j8 \
		OS=Win ARCH=i686 RELEASE=yes \
		AR=i686-w64-mingw32-ar \
		STRIP=i686-w64-mingw32-strip \
		CC=i686-w64-mingw32-gcc \
		LDFLAGS=-L/gframe-dev/win/i686/lib/c_synth\ -L/gframe-dev/win/i686/lib/sdl2 \
		CFLAGS=-I/gframe-dev/win/i686/include \
		PREFIX=/gframe-dev/win/i686 && \
	make clean OS=Win RELEASE=yes && \
	make install -j8 \
		OS=Win ARCH=x86_64 DEBUG=yes \
		AR=x86_64-w64-mingw32-ar \
		STRIP=x86_64-w64-mingw32-strip \
		CC=x86_64-w64-mingw32-gcc \
		LDFLAGS=-L/gframe-dev/win/x86_64/lib/c_synth\ -L/gframe-dev/win/x86_64/lib/sdl2 \
		CFLAGS=-I/gframe-dev/win/x86_64/include \
		PREFIX=/gframe-dev/win/x86_64 && \
	make clean OS=Win DEBUG=yes && \
	make install -j8 \
		OS=Win ARCH=x86_64 RELEASE=yes \
		AR=x86_64-w64-mingw32-ar \
		STRIP=x86_64-w64-mingw32-strip \
		CC=x86_64-w64-mingw32-gcc \
		LDFLAGS=-L/gframe-dev/win/x86_64/lib/c_synth\ -L/gframe-dev/win/x86_64/lib/sdl2 \
		CFLAGS=-I/gframe-dev/win/x86_64/include \
		PREFIX=/gframe-dev/win/x86_64 && \
	make clean OS=Win RELEASE=yes && \
	make emscript \
		OS=emscript RELEASE=yes \
		CC=emcc \
		CFLAGS=-I./include\ -I/gframe-dev/linux/i686/include && \
	mkdir -p /gframe-dev/emscript/ && \
	cp bin/release/emscript/* /gframe-dev/emscript/

ADD env.sh /gframe-dev/
ADD Makefile.env /gframe-dev/
