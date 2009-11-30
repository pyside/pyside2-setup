Command line options
********************

At the moment, there is just one flag to change the |project| behaviour, ``--enable-parent-ctor-heuristic``. This flag enable an usefull heuristic which can save a lot of work when writing the typesystem.

This heuristic will be triggered when generating code for a method and:

* The function is a constructor.
* The argument name is "parent".
* The argument type is a pointer to an object.

When triggered, the heuristic will set the argument named "parent" as the parent of the current object.
Being a child of an object means that when the object's parent dies, the C++ instance also dies, so the Python references will be invalidated.

The main focus of this tag was to remove a lot of hand written code from typesystem when binding Qt libraries, for Qt, this heuristic is never wrong, but be aware that it might be when binding your own libraries.
