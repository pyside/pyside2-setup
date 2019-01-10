Chapter 1: Reading data from a CSV
===================================

There are several ways to read data from a CSV file. The following are the most
common ways:

- Native reading
- the `CSV module <https://docs.python.org/3/library/csv.html>`_
- the `numpy module <https://www.numpy.org>`_
- the `pandas module <https://pandas.pydata.org/>`_

In this chapter, you will learn to use pandas to read and filter CSV data.
In addition, you could pass the data file through a command-line option to your
script.

The following python script, :code:`main.py`, demonstrates how to do it:

.. literalinclude:: datavisualize1/main.py
   :language: python
   :linenos:
   :lines: 40-

The Python script uses the :code:`argparse` module to accept and parse input
from the command line. It then uses the input, which in this case is the filename,
to read and print data to the prompt.

Try running the script in the following way to check if you get desired output:

::

    $python datavisualize1/main.py -f all_hour.csv
                              time   latitude   longitude  depth    ...      magNst     status  locationSource  magSource
    0  2019-01-10T12:11:24.810Z  34.128166 -117.775497   4.46    ...         6.0  automatic              ci         ci
    1  2019-01-10T12:04:26.320Z  19.443333 -155.615997   0.72    ...         6.0  automatic              hv         hv
    2  2019-01-10T11:57:48.980Z  33.322500 -116.393167   4.84    ...        11.0  automatic              ci         ci
    3  2019-01-10T11:52:09.490Z  38.835667 -122.836670   1.28    ...         7.0  automatic              nc         nc
    4  2019-01-10T11:25:44.854Z  65.108200 -149.370100  20.60    ...         NaN  automatic              ak         ak
    5  2019-01-10T11:25:23.786Z  69.151800 -144.497700  10.40    ...         NaN   reviewed              ak         ak
    6  2019-01-10T11:16:11.761Z  61.331800 -150.070800  20.10    ...         NaN  automatic              ak         ak

    [7 rows x 22 columns]
