.PHONY: usage help build-info debug release windebug winrelease android webdebug webrelease webdist clean distclean

# Configuration
WEBDIST_DIR := dist_web
ANDROID_PROJ_DIR := android
EMSCRIPTEN_TOOLCHAIN := /usr/lib/emscripten/cmake/Modules/Platform/Emscripten.cmake
DEMO_DIRS := FishTankDemo TickTacToe TestBed LuaTest

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
	@echo "OhmFlux Build System - Usage"
	@echo "-----------------------------------------------------------------"
	@echo "make debug      : Build native Desktop (Debug)"
	@echo "make release    : Build native Desktop (Release)"
	@echo ""
	@echo "make windebug   : Cross-compile Windows (Debug via MinGW)"
	@echo "make winrelease : Cross-compile Windows (Release via MinGW)"
	@echo ""
	@echo "make android    : Cross-compile Android (Release ARM64)"
	@echo ""
	@echo "make webdebug   : Build WebGL (Debug via Emscripten)"
	@echo "make webrelease : Build WebGL (Release via Emscripten)"
	@echo "make webdist    : Build WebGL Release and deploy to $(WEBDIST_DIR)"
	@echo ""
	@echo "make build-info  : Show packages to install on Arch and FreeBSD "
	@echo ""
	@echo "make clean      : Remove build/ directory"
	@echo "make distclean  : Remove all build artifacts, binaries, and $(WEBDIST_DIR)"
	@echo "-----------------------------------------------------------------"
	@echo ""

build-info:
	@echo "--- [ Arch Linux Setup ] ---"
	@echo "Nativ:      sudo pacman -S sdl3 glew opengl-headers"
	@echo "Windows:    yay -S mingw-w64-sdl3 mingw-w64-glew"
	@echo "Android:    yay -S android-ndk"
	@echo "            add environment: export ANDROID_NDK_HOME=/opt/android-ndk"
	@echo "            yay -S android-sdk android-sdk-build-tools android-sdk-platform-tools"
	@echo "            sudo pacman -S gradle"
	@echo "Emscripten: pkg install sdl3 glew"
	@echo "            testing @bash:"
	@echo "            source /etc/profile.d/emscripten.sh"
	@echo "            emrun index.html"
	@echo ""
	@echo "--- [ FreeBSD Setup ] ---"
	@echo "Nativ:   sudo pkg install cmake gcc sdl3 glew"
	@echo "Windows: pkg install mingw-w64-gcc mingw-w64-binutils"

# -----------------  D E S K T O P  --------------------
debug:
	cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug $(JOBS)

release:
	cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release $(JOBS)

# ---------- C R O S S C O M P I L E for W I N ----------------
windebug:
	x86_64-w64-mingw32-cmake -S . -B build/win_debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/win_debug $(JOBS)

winrelease:
	x86_64-w64-mingw32-cmake -S . -B build/win_release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/win_release $(JOBS)

# --------- C R O S S C O M P I L E ANDROID -------------
# Ensure ANDROID_NDK_HOME and ANDROID_HOME ate set in your environment
# arch: export ANDROID_NDK_HOME=/opt/android-ndk
# arch: export ANDROID_HOME=/opt/android-sdk
# Define where your Android project template is located
# --- Settings ---
android:
	# 1. Build the .so libraries (Engine + Demos)
	cmake -S . -B build/android \
		-DCMAKE_TOOLCHAIN_FILE=$(ANDROID_NDK_HOME)/build/cmake/android.toolchain.cmake \
		-DANDROID_ABI=arm64-v8a \
		-DANDROID_PLATFORM=android-24 \
		-DCMAKE_BUILD_TYPE=Release
	cmake --build build/android $(JOBS)

	# 2. Initialize the SDL3 Android Project if missing
	@if [ ! -f $(ANDROID_PROJ_DIR)/gradlew ]; then \
		echo "Initializing SDL3 Android Project from deps..."; \
		cp -r build/android/_deps/sdl3-src/android-project/* $(ANDROID_PROJ_DIR)/; \
		chmod +x $(ANDROID_PROJ_DIR)/gradlew; \
	fi

	# 3. Build an APK for each demo
	@for target in $(DEMO_DIRS); do \
		echo "--- Packaging APK for: $$target ---"; \
		# Ensure jniLibs dir exists and copy .so files \
		mkdir -p $(ANDROID_PROJ_DIR)/app/src/main/jniLibs/arm64-v8a/; \
		cp build/android/lib$$target.so $(ANDROID_PROJ_DIR)/app/src/main/jniLibs/arm64-v8a/; \
		find build/android -name "libSDL3.so" -exec cp {} $(ANDROID_PROJ_DIR)/app/src/main/jniLibs/arm64-v8a/ \; ; \
		\
		# 2025 WAY: Tell SDL3 which .so to load by editing its strings.xml \
		sed -i "s/<string name=\"SDL_DEFAULT_LIBRARY\">.*<\/string>/<string name=\"SDL_DEFAULT_LIBRARY\">$$target<\/string>/g" \
			$(ANDROID_PROJ_DIR)/app/src/main/res/values/strings.xml; \
		\
		# Build the APK via Gradle \
		cd $(ANDROID_PROJ_DIR) && ./gradlew assembleDebug; \
		cd ..; \
		\
		# Move and rename the result \
		mkdir -p build/apks/; \
		cp $(ANDROID_PROJ_DIR)/app/build/outputs/apk/debug/app-debug.apk build/apks/$$target.apk; \
	done

# -----------------  W E B  --------------------
webdebug:
	cmake -S . -B build/web_debug \
		-DCMAKE_TOOLCHAIN_FILE=$(EMSCRIPTEN_TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Debug
	cmake --build build/web_debug $(JOBS)

webrelease:
	cmake -S . -B build/web_release \
		-DCMAKE_TOOLCHAIN_FILE=$(EMSCRIPTEN_TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Release
	cmake --build build/web_release $(JOBS)

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
	rm -rf build/
	echo "done"

distclean:
	@echo "Performing deep clean..."
	# Remove temporary build folders
	rm -rf build/
	# Remove distribution folder
	rm -rf $(WEBDIST_DIR)
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
