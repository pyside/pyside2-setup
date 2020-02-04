SampleBinding Example
***********************

This example showcases how you can use Shiboken to generate CPython-based
binding code for a C++ library. The C++ library is called :code:`Universe`,
with two classes: :code:`Icecream` and :code:`Truck`. Ice-creams are
characterized by their flavor, and :code:`Truck` serves as a vehicle of
:code:`Icecream` distribution for kids in a neighborhood.

First, let's look at the definition of the two classes:

.. code-block:: cpp
   :caption: icecream.h

    class Icecream
    {
    public:
        Icecream(const std::string &flavor);
        virtual Icecream *clone();
        virtual ~Icecream();
        virtual const std::string getFlavor();

    private:
        std::string m_flavor;
    };

.. code-block:: cpp
   :caption: truck.h

    class Truck {
    public:
        Truck(bool leaveOnDestruction = false);
        Truck(const Truck &other);
        Truck& operator=(const Truck &other);
        ~Truck();

        void addIcecreamFlavor(Icecream *icecream);
        void printAvailableFlavors() const;

        bool deliver() const;
        void arrive() const;
        void leave() const;

        void setLeaveOnDestruction(bool value);
        void setArrivalMessage(const std::string &message);

    private:
        void clearFlavors();

        bool m_leaveOnDestruction = false;
        std::string m_arrivalMessage = "A new icecream truck has arrived!\n";
        std::vector m_flavors;
    };

Here's a summary of what's included in the :code:`Universe` library:

* The :code:`Icecream` polymorphic type, which is intended to be overridden.
* The :code:`Icecream::getFlavor()` method returns the flavor depending on the
  actual derived type.
* The :code:`Truck` value type that contains pointers, hence the copy
  constructor.
* :code:`Truck` stores the :code:`Icecream` objects in a vector, which can be
  modified via :code:`Truck::addIcecreamFlavor()`.
* The :code:`Truck’s` arrival message can be customized using its
  :code:`setArrivalMessage()` method.
* The :code:`Truck::deliver()` method tells us if the ice-cream delivery was
  successful.

Shiboken typesystem
====================

Now that the library definitions are in place, the Shiboken generator needs a
header file that includes the types we are interested in:

.. code-block:: cpp
   :caption: bindings.h

    #ifndef BINDINGS_H
    #define BINDINGS_H
    #include "icecream.h"
    #include "truck.h"
    #endif // BINDINGS_H

In addition, Shiboken also requires an XML-based typesystem file that defines the
relationship between C++ and Python types:

.. code-block:: xml
   :caption: bindings.xml

    <?xml version="1.0"?>
    <typesystem package="Universe">
        <primitive-type name="bool"/>
        <primitive-type name="std::string"/>
        <object-type name="Icecream">
            <modify-function signature="clone()">
                <modify-argument index="0">
                    <define-ownership owner="c++"/>
                </modify-argument>
            </modify-function>
        </object-type>
        <value-type name="Truck">
            <modify-function signature="addIcecreamFlavor(Icecream*)">
                <modify-argument index="1">
                    <define-ownership owner="c++"/>
                </modify-argument>
            </modify-function>
        </value-type>
    </typesystem>

One important thing to notice here is that we declare :code:`"bool"` and
:code:`"std::string"` as primitive types. These types are used by some of the
C++ methods as parameters or return types, so Shiboken must know about them.
Then, Shiboken can generate relevant conversion code between C++ and Python, although
most C++ primitive types are handled by Shiboken without additional code.

Next, we declare the two aforementioned classes. One of them as an
“object-type” and the other as a “value-type”. The main difference is that
object-types are passed around in generated code as pointers, whereas
value-types are copied (value semantics).

By specifying the names of these classes in the typesystem file, Shiboken
automatically tries to generate bindings for all methods of those
classes. You need not mention all the methods manually in the XML file, unless
you want to modify them.

Object ownership rules
=======================

Shiboken doesn't know if Python or C++ are responsible for freeing the C++ objects that were
allocated in the Python code, and assuming this might lead to errors.
There can be cases where Python should release the C++ memory when the reference count of the
Python object becomes zero, but it should never delete the underlying C++ object just from
assuming that it will not be deleted by underlying C++ library, or if it's maybe parented to
another object (like QWidgets).

In our case, the :code:`clone()` method is only called inside the C++ library,
and we assume that the C++ code takes care of releasing the cloned object.

As for :code:`addIcecreamFlavor()`, we know that a :code:`Truck` owns the
:code:`Icecream` object, and will remove it once the :code:`Truck` is
destroyed. That's why the ownership is set to “c++” in the typesystem file,
so that the C++ objects are not deleted when the corresponding Python names
go out of scope.

Build
=====

To build the :code:`Universe` custom library and then generate bindings for it,
use the :file:`CMakeLists.txt` file provided with the example. Later, you can reuse
the file for your own libraries with minor changes.

Now, run the :command:`"cmake ."` command from the prompt to configure the
project and build with the toolchain of your choice; we recommend the
‘(N)Makefiles’ generator.

As a result, you end up with two shared libraries:
:file:`libuniverse.(so/dylib/dll)` and :file:`Universe.(so/pyd)`. The former is
the custom C++ library, and the latter is the Python module to import in your
Python script.

For more details about these platforms, see the :file:`README.md` file.

Use the Python module
=====================

The following script uses the :code:`Universe` module, derives a few types from
:code:`Icecream`, implements virtual methods, instantiates objects, and much more:

.. code-block:: python
   :caption: main.py

    from Universe import Icecream, Truck

    class VanillaChocolateIcecream(Icecream):
        def __init__(self, flavor=""):
            super(VanillaChocolateIcecream, self).__init__(flavor)

        def clone(self):
            return VanillaChocolateIcecream(self.getFlavor())

        def getFlavor(self):
            return "vanilla sprinked with chocolate"

    class VanillaChocolateCherryIcecream(VanillaChocolateIcecream):
        def __init__(self, flavor=""):
            super(VanillaChocolateIcecream, self).__init__(flavor)

        def clone(self):
            return VanillaChocolateCherryIcecream(self.getFlavor())

        def getFlavor(self):
            base_flavor = super(VanillaChocolateCherryIcecream, self).getFlavor()
            return base_flavor + " and a cherry"

    if __name__ == '__main__':
        leave_on_destruction = True
        truck = Truck(leave_on_destruction)

        flavors = ["vanilla", "chocolate", "strawberry"]
        for f in flavors:
            icecream = Icecream(f)
            truck.addIcecreamFlavor(icecream)

        truck.addIcecreamFlavor(VanillaChocolateIcecream())
        truck.addIcecreamFlavor(VanillaChocolateCherryIcecream())

        truck.arrive()
        truck.printAvailableFlavors()
        result = truck.deliver()

        if result:
            print("All the kids got some icecream!")
        else:
            print("Aww, someone didn't get the flavor they wanted...")

        if not result:
            special_truck = Truck(truck)
            del truck

            print("")
            special_truck.setArrivalMessage("A new SPECIAL icecream truck has arrived!\n")
            special_truck.arrive()
            special_truck.addIcecreamFlavor(Icecream("SPECIAL *magical* icecream"))
            special_truck.printAvailableFlavors()
            special_truck.deliver()
            print("Now everyone got the flavor they wanted!")
            special_truck.leave()

After importing the classes from the :code:`Universe` module, it derives two
types from :code:`Icecream` for different “flavors”. It then creates a
:code:`truck` to deliver some regular flavored Icecreams and two special ones.

If the delivery fails, a new :code:`truck` is created with the old flavors
copied over, and a new *magical* flavor that will surely satisfy all customers.

Try running it to see if the ice creams are delivered.

.. note::
    You can find the sources for this example under
    :file:`<PYTHON_ENV_ROOT>/site-packages/lib/PySide2/examples/samplebinding`.

Refer to the following topics for detailed information about using Shiboken:
 * :doc:`Shiboken module <shibokenmodule>`
 * :doc:`Type System Variables <typesystemvariables>`
 * :doc:`User Defined Type Conversion <typeconverters>`
 * :doc:`Object ownership <ownership>`
 * :doc:`Frequently Asked Questions <faq>`
