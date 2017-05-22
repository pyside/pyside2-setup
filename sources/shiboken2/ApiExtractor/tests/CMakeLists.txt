find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Test)
find_package(Qt5Xml)
find_package(Qt5XmlPatterns)

macro(declare_test testname)
    # gone: qt4_automoc("${testname}.cpp")
    if (EXISTS "${testname}.h")
        add_executable(${testname} "${testname}.h ${testname}.cpp")
    else ()
        add_executable(${testname} "${testname}.cpp")
    endif ()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${apiextractor_SOURCE_DIR}
                        ${Qt5Test_INCLUDE_DIRS}
                        )
    target_link_libraries(${testname}
                          ${Qt5XmlPatterns_LIBRARIES}
                          ${Qt5Test_LIBRARIES}
                          ${Qt5Core_LIBRARIES}
                          ${Qt5Gui_LIBRARIES}
                          apiextractor)
    add_test(${testname} ${testname})
    if (INSTALL_TESTS)
        install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${testname}
                DESTINATION share/apiextractor${apiextractor_SUFFIX}/tests)
    endif()
endmacro(declare_test testname)

declare_test(testabstractmetaclass)
declare_test(testabstractmetatype)
declare_test(testaddfunction)
declare_test(testarrayargument)
declare_test(testcodeinjection)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/utf8code.txt"
                "${CMAKE_CURRENT_BINARY_DIR}/utf8code.txt" COPYONLY)
declare_test(testcontainer)
declare_test(testconversionoperator)
declare_test(testconversionruletag)
declare_test(testctorinformation)
declare_test(testdroptypeentries)
declare_test(testdtorinformation)
declare_test(testenum)
declare_test(testextrainclude)
declare_test(testfunctiontag)
declare_test(testimplicitconversions)
declare_test(testinserttemplate)
declare_test(testmodifyfunction)
declare_test(testmultipleinheritance)
declare_test(testnamespace)
declare_test(testnestedtypes)
declare_test(testnumericaltypedef)
declare_test(testprimitivetypetag)
declare_test(testrefcounttag)
declare_test(testreferencetopointer)
declare_test(testremovefield)
declare_test(testremoveimplconv)
declare_test(testremoveoperatormethod)
declare_test(testresolvetype)
declare_test(testreverseoperators)
declare_test(testtemplates)
declare_test(testtoposort)
declare_test(testvaluetypedefaultctortag)
declare_test(testvoidarg)
declare_test(testtyperevision)
if (NOT DISABLE_DOCSTRINGS)
    declare_test(testmodifydocumentation)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/a.xml"
                   "${CMAKE_CURRENT_BINARY_DIR}/a.xml" COPYONLY)
endif()

