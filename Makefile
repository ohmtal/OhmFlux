.PHONY: usage help build-info debug release windebug winrelease android webdebug webrelease webdist clean distclean

# Configuration
# --------------
DEMO_DIRS := FishTankDemo TickTacToe TestBed LuaTest Amana ImguiTest FluxEditor

BASE_BUILD_DIR := _build
WEBDIST_DIR := dist_web
EMSCRIPTEN_TOOLCHAIN := /usr/lib/emscripten/cmake/Modules/Platform/Emscripten.cmake

# Android config:
# Your assets must be in subdirectory assets!
#TODO: Change the path /opt/android to where you installed android studio !
NDK_DIRS := $(wildcard /opt/android/sdk/ndk/*/)
ANDROID_PROJ_DIR := android
ANDROID_NDK_HOME := $(lastword $(sort $(NDK_DIRS)))
# ANDROID_NDK_HOME := $(shell @ls -d /opt/android/sdk/ndk/*/ | sort -V | tail -n 1)
ANDROID_PLATFORM := android-24

# Parallel Build Detection
# Uses all available cores on Linux/FreeBSD, defaults to 4 if detection fails
JOBS := -j $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Tool Detection
BROTLI := $(shell command -v brotli 2> /dev/null)
GZIP   := $(shell command -v gzip 2> /dev/null)

# -----------------  H E L P  &  S E T U P  --------------------
help: usage

usage:
	@echo "-----------------------------------------------------------------"
	@echo -e "\e[34mOhmFlux Build System - Usage\e[0m"
	@echo "-----------------------------------------------------------------"
	@echo "make debug      : Build native Desktop (Debug)"
	@echo "make release    : Build native Desktop (Release)"
	@echo ""
	@echo "make windebug   : Cross-compile Windows (Debug via MinGW)"
	@echo "make winrelease : Cross-compile Windows (Release via MinGW)"
	@echo ""
	@echo "make android    : experimental Android (Release ARM64)"
	@echo ""
	@echo "make webdebug   : Build WebGL (Debug via Emscripten)"
	@echo "make webrelease : Build WebGL (Release via Emscripten)"
	@echo "make webdist    : Build WebGL Release and deploy to $(WEBDIST_DIR)"
	@echo ""
	@echo "make build-info : Show packages to install on Arch and FreeBSD "
	@echo ""
	@echo "make clean      : Remove $(BASE_BUILD_DIR)/ directory"
	@echo "make distclean  : Remove all build artifacts, binaries, and $(WEBDIST_DIR)"
	@echo "-----------------------------------------------------------------"
	@echo ""

build-info:
	@echo "--- [ Arch Linux Setup ] ---"
	@echo "Nativ:      sudo pacman -S sdl3 glew opengl-headers"
	@echo "Windows:    yay -S mingw-w64-sdl3 mingw-w64-glew"
	@echo "Android:    download and install Android Studio and set the pathes like:"
	@echo "            export ANDROID_HOME=/opt/android/sdk"
	@echo "            export ANDROID_SDK_ROOT=/opt/android/sdk"
	@echo "            export ANDROID_NDK_HOME=/opt/android/sdk/ndk/28.2.13676358  # Update version if different"
	@echo "            export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"
	@echo "            sudo pacman -S gradle"
	@echo "Emscripten: sudo pacman -S emscripten"
	@echo "            testing @bash:"
	@echo "            source /etc/profile.d/emscripten.sh"
	@echo "            emrun index.html"
	@echo ""
	@echo "--- [ FreeBSD Setup ] ---"
	@echo "Nativ:   sudo pkg install cmake gcc sdl3 glew"
	@echo ""
	@echo ""

# -----------------  D E S K T O P  --------------------
debug:
	cmake -S . -B $(BASE_BUILD_DIR)/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BASE_BUILD_DIR)/debug $(JOBS)

release:
	cmake -S . -B $(BASE_BUILD_DIR)/release -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BASE_BUILD_DIR)/release $(JOBS)

# ---------- C R O S S C O M P I L E for W I N ----------------
windebug:
	x86_64-w64-mingw32-cmake -S . -B $(BASE_BUILD_DIR)/win_debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BASE_BUILD_DIR)/win_debug $(JOBS)

winrelease:
	x86_64-w64-mingw32-cmake -S . -B $(BASE_BUILD_DIR)/win_release -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BASE_BUILD_DIR)/win_release $(JOBS)

# --------- experimental ANDROID -------------
# Ensure ANDROID_NDK_HOME and ANDROID_HOME ate set in your environment
# arch: export ANDROID_NDK_HOME=/opt/android-ndk
# arch: export ANDROID_HOME=/opt/android-sdk
# Define where your Android project template is located
# Your assets must be in subdirectory assets!
# --- Settings ---
android:
	# 1. Build the .so libraries (Engine + Demos)
	cmake -S . -B $(BASE_BUILD_DIR)/android \
		-DCMAKE_TOOLCHAIN_FILE=$(ANDROID_NDK_HOME)/build/cmake/android.toolchain.cmake \
		-DANDROID_ABI=arm64-v8a \
		-DANDROID_PLATFORM=$(ANDROID_PLATFORM) \
		-DCMAKE_BUILD_TYPE=Release
	cmake --build $(BASE_BUILD_DIR)/android $(JOBS)

	# 2. Initialize and Patch the SDL3 Android Project
	mkdir -p $(ANDROID_PROJ_DIR)/app
	@if [ ! -f $(ANDROID_PROJ_DIR)/gradlew ]; then \
		echo "Initializing SDL3 Project Template..."; \
		cp -r $(BASE_BUILD_DIR)/android/_deps/sdl3-src/android-project/* $(ANDROID_PROJ_DIR)/; \
		chmod +x $(ANDROID_PROJ_DIR)/gradlew; \
	fi

	# Overwrite the dynamic Gradle file with your static template
	cp build.gradle.template $(ANDROID_PROJ_DIR)/app/build.gradle
	rm -rf $(ANDROID_PROJ_DIR)/app/jni

	# 3. Build an APK for each demo
	@for target in $(DEMO_DIRS); do \
		echo "--- Packaging APK for: $$target ---"; \
		rm -rf $(ANDROID_PROJ_DIR)/app/libs/arm64-v8a/*; \
		mkdir -p $(ANDROID_PROJ_DIR)/app/libs/arm64-v8a/; \
		cp $(BASE_BUILD_DIR)/android/lib$$target.so $(ANDROID_PROJ_DIR)/app/libs/arm64-v8a/libmain.so; \
		echo "----- copy assets ----"; \
		ASSET_DIR=$(ANDROID_PROJ_DIR)/app/src/main/assets/; \
		rm -rf $$ASSET_DIR; \
		mkdir -p $$ASSET_DIR; \
		cp -r $$target/assets p $$ASSET_DIR; \
		echo "----- copy SDL ----"; \
		find $(BASE_BUILD_DIR)/android -name "libSDL3.so" -exec cp {} $(ANDROID_PROJ_DIR)/app/libs/arm64-v8a/ \; ; \
		sed -i "s/<string name=\"SDL_DEFAULT_LIBRARY\">.*<\/string>/<string name=\"SDL_DEFAULT_LIBRARY\">main<\/string>/g" \
			$(ANDROID_PROJ_DIR)/app/src/main/res/values/strings.xml; \
		cd $(ANDROID_PROJ_DIR) && ./gradlew assembleDebug -Pandroid.sdk.dir=$(ANDROID_HOME); \
		cd ..; \
		mkdir -p $(BASE_BUILD_DIR)/apks/; \
		cp $(ANDROID_PROJ_DIR)/app/build/outputs/apk/debug/app-debug.apk $(BASE_BUILD_DIR)/apks/$$target.apk; \
	done

	@echo "---------------------------------------------------------------------------------"
	@echo "Finished! hopefully"
	@echo "---------------------------------------------------------------------------------"
	@echo "# 1. Install it to your phone example:"
	@echo "    adb install ./_build/apks/FishTankDemo.apk"
	@echo "# 2. Launch it"
	@echo "# (Assuming your package name in the template was org.libsdl.app)"
	@echo "# adb shell am start -n org.libsdl.app/org.libsdl.app.SDLActivity"
	@echo "DEBUG WITH:"
	@echo "adb logcat '*:F' | grep -Ei "SDL|libmain|DEBUG"
	@echo "Check content example:"
	@echo "unzip -l ./_build/apks/FishTankDemo.apk"
	@echo "---------------------------------------------------------------------------------"

# -----------------  W E B  --------------------
webdebug:
	cmake -S . -B $(BASE_BUILD_DIR)/web_debug \
		-DCMAKE_TOOLCHAIN_FILE=$(EMSCRIPTEN_TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BASE_BUILD_DIR)/web_debug $(JOBS)

webrelease:
	cmake -S . -B $(BASE_BUILD_DIR)/web_release \
		-DCMAKE_TOOLCHAIN_FILE=$(EMSCRIPTEN_TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Release
	cmake --build $(BASE_BUILD_DIR)/web_release $(JOBS)

webdist: webrelease
	@echo "--- Preparing Distribution Directory: $(WEBDIST_DIR) ---"
	@mkdir -p $(WEBDIST_DIR)
	@for dir in $(DEMO_DIRS); do \
		echo "Deploying $$dir..."; \
		mkdir -p $(WEBDIST_DIR)/$$dir; \
		cp $$dir/index.html $(WEBDIST_DIR)/$$dir/ 2>/dev/null || true; \
		cp $$dir/index.js $(WEBDIST_DIR)/$$dir/; \
		cp $$dir/index.wasm $(WEBDIST_DIR)/$$dir/; \
		if [ -f $$dir/index.data ]; then \
			cp $$dir/index.data $(WEBDIST_DIR)/$$dir/; \
		fi; \
		if [ -n "$(BROTLI)" ]; then \
			echo "  -> Brotli compressing $$dir assets..."; \
			$(BROTLI) --best --force $(WEBDIST_DIR)/$$dir/index.wasm $(WEBDIST_DIR)/$$dir/index.js; \
			[ -f $(WEBDIST_DIR)/$$dir/index.data ] && $(BROTLI) --best --force $(WEBDIST_DIR)/$$dir/index.data; \
		elif [ -n "$(GZIP)" ]; then \
			echo "  -> Gzip compressing $$dir assets..."; \
			$(GZIP) -9 -k -f $(WEBDIST_DIR)/$$dir/index.wasm $(WEBDIST_DIR)/$$dir/index.js; \
			[ -f $(WEBDIST_DIR)/$$dir/index.data ] && $(GZIP) -9 -k -f $(WEBDIST_DIR)/$$dir/index.data; \
		fi; \
	done
	@echo "-----------------------------------------------------------------"
	@echo "WebGL Deployment complete! Files ready in: ./$(WEBDIST_DIR)"



# -----------------  C L E A N --------------------
clean:
	@echo "Removing build directory..."
	rm -rf build
	rm -rf $(BASE_BUILD_DIR)
	echo "done"

distclean:
	@echo "Performing deep clean..."
	# Remove temporary build folders
	rm -rf build
	rm -rf $(BASE_BUILD_DIR)
	# Remove distribution folder
	rm -rf $(WEBDIST_DIR)
	rm -rf $(ANDROID_PROJ_DIR)
	# Clean up Demo directories
	@for dir in $(DEMO_DIRS); do \
		echo "  Cleaning $$dir..."; \
		rm -f $$dir/*.log; \
		rm -f $$dir/$$dir_*.x86_64; \
		rm -f $$dir/$$dir_*.exe; \
		rm -f $$dir/index.html $$dir/index.js $$dir/index.wasm $$dir/index.data; \
		rm -f $$dir/index.wasm.br $$dir/index.js.br $$dir/index.data.br; \
		rm -f $$dir/index.wasm.gz $$dir/index.js.gz $$dir/index.data.gz; \
	done
	@echo "Deep clean complete."
