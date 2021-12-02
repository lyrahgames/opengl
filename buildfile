./: {*/ -build/} doc{README.md AUTHORS.md} legal{COPYING.md} manifest

# Don't install tests.
#
tests/: install = false
