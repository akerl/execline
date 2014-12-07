PACKAGE = execline
BUILD_DIR = /tmp/$(PACKAGE)-build
RELEASE_DIR = /tmp/$(PACKAGE)-release
RELEASE_FILE = $(PACKAGE).tar.gz

.PHONY : default manual container build push local

default: container

manual:
	./meta/launch /bin/bash || true

container:
	./meta/launch

build:
	rm -rf $(BUILD_DIR)
	cp -R upstream $(BUILD_DIR)
	patch -d $(BUILD_DIR) -p1 < patch
	make -C $(BUILD_DIR) install
	tar -czv -C $(RELEASE_DIR) -f $(RELEASE_FILE) .

push:
	git commit -am "$$(cat upstream/package/version)" || true
	ssh -oStrictHostKeyChecking=no git@github.com &>/dev/null || true
	git tag -f "$$(cat upstream/package/version)"
	git push origin :"$$(cat upstream/package/version)" || true
	git push --tags origin master
	targit -a .github -c -f akerl/execline $$(cat upstream/package/version) $(RELEASE_FILE)

local: build push

