Chapter 6 - Plot the data in the ChartView
===========================================

The last step of this tutorial is to plot the CSV data inside our QChart. For
this, you need to go over our data and include the data on a QLineSeries.

After adding the data to the series, you can modify the axis to properly
display the QDateTime on the X-axis, and the magnitude values on the Y-axis.

Here is the updated main_widget.py from with an additional function that adds
the QLineSeries.

.. literalinclude:: datavisualize6/main_widget.py
   :language: python
   :linenos:
   :pyobject: Widget.add_series

Now, run the application to visualize the earthquake magnitudes
data at different times.

.. image:: images/datavisualization_app.png

Try modifying the sources to get different output. For example, you could try
to plot more data from the CSV.
