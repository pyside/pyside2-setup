Manipulating Documentation
--------------------------

inject-documentation
^^^^^^^^^^^^^^^^^^^^

    The inject-documentation node inserts the documentation into the generated
    documentation. This node is a child of the object-type, value-type and
    modify-function nodes.

    .. code-block:: xml

         <value-type>
             <inject-documentation mode="append | prepend | replace" format="native | target" >
                 // the documentation
             </inject-code>
         </value-type>

    The **mode** attribute default value is *replace*.

    The **format** attribute specifies when the documentation injection will
    occur and it accepts the following values:

        * native: Before XML<->Backend transformation occur, so the injected code *must* be a valid XML.
        * target: After XML<->Backend transformation occur, so the injected code *must* be a valid backend format.

    At the moment the only supported backend is Sphinx.

modify-documentation
^^^^^^^^^^^^^^^^^^^^

    The modify-documentation node allows you to change the auto-generated
    documentation. API Extractor transforms XML's from qdoc (the Qt documentation
    tool) into .rst files to be processed later using Sphinx. You can modify
    the XML before the transformation takes place.

    .. code-block:: xml

        <modify-documentation xpath="...">
            <!-- new documentation -->
        </modify-documentation>

    The **xpath** attribute is the XPath to the node that you want to modify.
