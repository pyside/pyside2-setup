all:
	(cd libfoo; $(MAKE))
	(cd foobinding; $(MAKE))


test:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(PWD)/libfoo PYTHONPATH=$(PYTHONPATH):$(PWD)/foobinding/foo python foo_test.py

clean:
	(cd libfoo; $(MAKE) clean)
	(cd foobinding; $(MAKE) clean)

