.PHONY: usage debug release webdebug webrelease webdist clean distclean

# Configuration
WEBDIST_DIR = dist_web
EMSCRIPTEN_TOOLCHAIN = /usr/lib/emscripten/cmake/Modules/Platform/Emscripten.cmake
DEMO_DIRS = FishTankDemo TickTacToe TestBed LuaTest

# Parallel Build Detection
# Uses all available cores on Linux/FreeBSD, defaults to 4 if detection fails
JOBS := -j $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Tool Detection
BROTLI := $(shell command -v brotli 2> /dev/null)
GZIP   := $(shell command -v gzip 2> /dev/null)

usage:
	@echo "OhmFlux Build System - Usage"
	@echo "-----------------------------------------------------------------"
	@echo "make debug      : Build native Desktop (Debug)"
	@echo "make release    : Build native Desktop (Release)"
	@echo "make webdebug   : Build WebGL (Debug)"
	@echo "make webrelease : Build WebGL (Release)"
	@echo "make webdist    : Build WebGL Release and deploy to $(WEBDIST_DIR)"
	@echo "make clean      : Remove build artifacts"
	@echo "make distclean  : Remove all build artifacts, logs, binaries, and $(WEBDIST_DIR)"
	@echo "-----------------------------------------------------------------"

debug:
	cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug $(JOBS)

release:
	cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release $(JOBS)

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
		rm -f $$dir/index.html $$dir/index.js $$dir/index.wasm $$dir/index.data; \
		rm -f $$dir/index.wasm.br $$dir/index.js.br $$dir/index.data.br; \
		rm -f $$dir/index.wasm.gz $$dir/index.js.gz $$dir/index.data.gz; \
	done
	@echo "Deep clean complete."
