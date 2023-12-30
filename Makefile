all: build install

build:
	python HookySetup.py --build_ext --inplace

install:
	python HookySetup.py --install
