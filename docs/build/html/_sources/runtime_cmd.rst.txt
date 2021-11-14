Runtime and Command Line
========================

Cocoa contains a JavaScript engine based on Google's V8. So it can be treated as a JavaScript
runtime environment which provides 2D rendering functions.

Cocoa is a standalone executable file, and it is linked to a series of shared
libraries. As these libraries are built and released with Cocoa project you should **NOT**
replace or change them.

To start Cocoa, just open a terminal, enter the directory of Cocoa and type:
``./Cocoa <working directory>``

Options
-------

Here're some possible command line arguments, which will change Cocoa's behaviour.

.. tip::
    See also :ref:`following section <Language Binding>` to learn about language bindings.

--help
    Print available options and then exit with 0.
    Using ``-h`` for short is acceptable.
--version
    Print version and copyright information, then exit.
    Using ``-v`` for short is acceptable.
--log-file
    Specify a file to write logs. Requires a string as value.
    Using ``-o`` for short is acceptable.
--log-stderr
    Write logs to standard error instead of default stdout.

    .. note::
        If ``--log-file`` and ``--log-stderr`` are provided at the same time, the last one
        in the arguments list will work.

--log-level
    Specify log level. Using ``-L`` for short is acceptable. Requires a string as value.

    +-----------+------------------------------+
    | Level     |  Allowed message types       |
    +===========+==============================+
    | debug     | debug, info, warning, error, |
    |           | fatal                        |
    +-----------+------------------------------+
    | normal    | info, warning, error, fatal  |
    +-----------+------------------------------+
    | quiet     | warning, error, fatal        |
    +-----------+------------------------------+
    | silent    | error, fatal                 |
    +-----------+------------------------------+
    | disabled  | none                         |
    +-----------+------------------------------+

--disable-color-log
    Don't generate colored logs through ANSI escape code.

--vm-thread-pool-size
    Specify the number of worker threads for V8. Requires an integer as value.
    If it's zero or not set, a suitable default based on the current number of processors
    will be choosen.

--lbp-blacklist
    Specify a comma seperated list of language bindings. Those language bindings will not
    be loaded and available in JavaScript.

--lbp-preload
    Load an external shared library as a language binding to extend JavaScript API.
    Requires a string as path.

--lbp-allow-override
    Cocoa will crash if language bindings have the same name. By specifying this option,
    it will be allowed to load language bindings with the same name. The last namesake
    language binding will work.

    .. warning::
        Internal language bindings (like ``core``, ``nvg``, etc.) may be overridden by
        this option. That will cause serious security problems and make Cocoa crash.
        This option is just for debugging or testing, never use it in released products.

For those long options (begin with double ``-``) that require a value, use ``=value``;
for those short options (begin with a single ``-``) that require a value, place ``value``
as next option directly.
For example, both ``--log-file=/path/to/log`` and ``-o /path/to/log`` are acceptable.

.. tip::
    You can put short options together like ``-hvo /path/to/log``. In that case, only the
    last short option can associate with the value ``/path/to/log``.

Language Binding
----------------

Language binding (aka LB) is a bridge between JavaScript and native code.
Here's an example that shows how it works.

Supposing following TypeScript will be executed:

.. code-block:: javascript
    :linenos:

    Cocoa.core.print("Hello, World!\n");

V8 will look up the method ``print()`` and invoke our native code
in ``src/Koi/lang/CoreBinding.cc``:

.. code-block:: cpp
    :linenos:

    // type check and convert from v8::Local<v8::Value> to std::string
    // is binder's responsibility.
    #include <cstdlib>
    #include <string>
    void jni_core_print(const std::string& str)
    {
        if (str.empty())
            return;
        std::fwrite(str.c_str(), str.size(), 1, stdout);
    }


Write Your Own Language Binding
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::
    Skip this part if you're not familiar with C++ and V8.

.. code-block:: cpp
    :linenos:

    #include <iostream>
    #include "Koi/binder/Module.h"
    #include "Koi/lang/Base.h"

    using namespace cocoa::koi;

    class DynTestBinding : public lang::BaseBindingModule
    {
    public:
        // Provide a name and description of this language binding
        // when constructing superclass lang::BaseBindingModule.
        DynTestBinding()
            : lang::BaseBindingModule("dyn_test",
                                      "Dynamic language binding [preloader test]") {}
        ~DynTestBinding() override = default;

        // getModule() will be called automatically.
        // You should register constants, classes and methods here.
        void getModule(binder::Module &mod) override {
            mod.set("targetMethod", &DynTestBinding::targetMethod);
        }

        static void targetMethod() {
            std::cout << "DynTestBinding::targetMethod() is called" << std::endl;
        }
    };

    // The use of keyword extern is necessary to avoid C++ compiler's
    // name mangling.
    // Macro LANG_BINDING_HOOK_SYM defines a certain function signature
    // so that Cocoa's language binding preloader can find it.
    extern "C" void LANG_BINDING_HOOK_SYM(lang::LbpBlock *block)
    {
        // Never delete DynTestBinding object as Cocoa does that automatically.
        // We provide a interface (struct `LbpBlock`) that allows you accessing
        // other registered language bindings.
        lang::RegisterBinding(new DynTestBinding());
    }

    // Each language binding has a globally unique identifier.
    // Cocoa identifies language bindings internally by GUID.
    LANG_BINDING_GUID("dyn_test", "1.0.0");

Compile above code by:

.. code-block::
    :linenos:

    cd <Cocoa project directory>
    ./script/build-language-binding test.cc -o libtest.so

Test it in JavaScript (index.js):

.. code-block:: javascript
    :linenos:

    Cocoa.dyn_test.targetMethod();
    // Expected ouput: DynTestBinding::targetMethod() is called

Run:

.. code-block::
    :linenos:

    # Replace ./build/Cocoa with the path of Cocoa executable file
    ./build/Cocoa --lbp-preload=/path/to/libtest.so
