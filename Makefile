# vita-turbo-storage - build and deploy helpers.
#
#   make                      build both modules (uses your VitaSDK if $VITASDK
#                             is set, otherwise the pinned VitaSDK Docker image)
#   make install VITA_IP=...  upload the modules to ur0:tai/ over FTP
#   make install-qm VITA_IP=... upload the quick-menu row to ur0:QuickMenuReborn/
#   make deploy  VITA_IP=...  install, then reboot the Vita so they load
#   make log     VITA_IP=...  fetch the plugin logs into ./logs/
#   make config               print the lines to add to ur0:tai/config.txt
#   make clean
#
# install/deploy/log talk to vitacompanion (FTP on :1337, commands on :1338)
# running on the Vita. Output: build/storageturbo_kernel.skprx, build/storageturbo.suprx,
# build/storageturbo_settings.suprx, build/storageturbo_qm.suprx (optional, needs
# QuickMenuReborn)

BUILD   ?= build
VITA_IP ?=
# Pinned so every build uses the same toolchain (VitaSDK 2026.08, GCC 15.2).
VITASDK_IMAGE ?= vitasdk/vitasdk@sha256:8227da1b596d981f4513cd9b36bda1e43857f6dae5bcf4312c1cd40167f7d9bb
# Build id for the logs: the git commit (with -dirty for uncommitted changes),
# taken here because the build container may have no git.
BUILD_ID ?= $(shell git describe --always --dirty --abbrev=7 --exclude '*' 2>/dev/null || echo unknown)

MODULES := $(BUILD)/storageturbo_kernel.skprx $(BUILD)/storageturbo.suprx \
           $(BUILD)/storageturbo_settings.suprx
FTP      = ftp://$(VITA_IP):1337

.PHONY: all build install install-qm deploy log config clean check-ip

all: build

build:
ifdef VITASDK
	cmake -S . -B $(BUILD) -DCMAKE_TOOLCHAIN_FILE=$(VITASDK)/share/vita.toolchain.cmake \
	  -DTS_BUILD_ID=$(BUILD_ID)
	cmake --build $(BUILD) -j
else
	docker run --rm -v "$(CURDIR)":/src -w /src $(VITASDK_IMAGE) sh -c '\
	  cmake -S . -B $(BUILD) -DCMAKE_TOOLCHAIN_FILE=$$VITASDK/share/vita.toolchain.cmake \
	    -DTS_BUILD_ID=$(BUILD_ID) && \
	  cmake --build $(BUILD) -j$$(nproc) && \
	  chown -R $(shell id -u):$(shell id -g) $(BUILD)'
endif

check-ip:
	@test -n "$(VITA_IP)" || { echo "Set VITA_IP, e.g. make $(MAKECMDGOALS) VITA_IP=192.168.1.50"; exit 1; }

install: build check-ip
	@for f in $(MODULES); do \
	  echo "upload $$f"; curl -sS -T $$f "$(FTP)/ur0:/tai/$$(basename $$f)" || exit 1; \
	done
	@echo "Installed. If this is the first install, add the plugin to config.txt (make config)."

install-qm: build check-ip
	curl -sS --ftp-create-dirs -T $(BUILD)/storageturbo_qm.suprx "$(FTP)/ur0:/QuickMenuReborn/storageturbo_qm.suprx"
	@echo "Installed. Restart the Vita (or reload QuickMenuReborn) to see the row."

deploy: install
	@printf 'reboot\n' | nc -w 3 $(VITA_IP) 1338 >/dev/null || true
	@echo "Rebooting the Vita - the plugin loads at boot."

log: check-ip
	@mkdir -p logs
	curl -sS "$(FTP)/ur0:/data/turbostorage/kernel.log" -o logs/kernel.log
	curl -sS "$(FTP)/ur0:/data/turbostorage/user.log" -o logs/user.log
	-curl -sS "$(FTP)/ur0:/data/turbostorage/kernel.old.log" -o logs/kernel.old.log
	-curl -sS "$(FTP)/ur0:/data/turbostorage/boot_guard" -o logs/boot_guard
	@echo "Saved logs/kernel.log, kernel.old.log, user.log and boot_guard"

config:
	@printf '%s\n' '*KERNEL' 'ur0:tai/storageturbo_kernel.skprx' '' '*main' 'ur0:tai/storageturbo.suprx' '' \
	  '*NPXS10015' 'ur0:tai/storageturbo_settings.suprx'

clean:
	rm -rf $(BUILD) logs
