Chapter 2 - Filtering data
===========================

In the previous chapter, you learned how to read and print data that is a
bit raw. Now, try to select a few columns and handle them properly.

Start with these two columns: Time (time) and Magnitude (mag). After getting
the information from these columns, filter and adapt the data. Try formatting
the date to Qt types.

There is not much to do for the Magnitude column, as it's just a floating point
number. You could take special care to check if the data is correct. This could
be done by filtering the data that follows the condition, "magnitude > 0", to
avoid faulty data or unexpected behavior.

The Date column provides data in UTC format (for example,
2018-12-11T21:14:44.682Z), so you could easily map it to a QDateTime object
defining the structure of the string. Additionally, you can adapt the time
based on the timezone you are in, using QTimeZone.

The following script filters and formatts the CSV data as described earlier:

.. literalinclude:: datavisualize2/main.py
   :language: python
   :linenos:
   :emphasize-lines: 44,47-69
   :line: 40-

Now that you have a tuple of QDateTime and float data, try improving the
output further. That's what you'll learn in the following chapters.
