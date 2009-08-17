all: generate
	(cd foo; $(MAKE))

generate:
	boostpythongenerator --disable-named-arg global.h \
						--include-paths=`pwd`/../libfoo \
						--typesystem-paths=. --output-directory=. \
						typesystem_foo.xml

clean:
	rm *.log .preprocessed.tmp foo/*.hpp foo/*.cpp -rf
	(cd foo; $(MAKE) clean)

